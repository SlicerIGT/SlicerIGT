/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  ==============================================================================*/

// MarkupsToModel Logic includes
#include "vtkSlicerMarkupsToModelLogic.h"

// MRML includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSelectionNode.h"
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkDoubleArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyDataNormals.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <cassert>
#include <cmath>
#include <vector>
#include <set>

static const double CLEAN_POLYDATA_TOLERANCE_MM = 0.01;

#include "CreateClosedSurfaceUtil.h"
#include "CreateCurveUtil.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsToModelLogic);

//----------------------------------------------------------------------------
vtkSlicerMarkupsToModelLogic::vtkSlicerMarkupsToModelLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsToModelLogic::~vtkSlicerMarkupsToModelLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer< vtkMRMLMarkupsToModelNode >::New());
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneEndImport()
{
  vtkSmartPointer<vtkCollection> markupsToModelNodes = vtkSmartPointer<vtkCollection>::Take(
    this->GetMRMLScene()->GetNodesByClass("vtkMRMLMarkupsToModelNode"));
  vtkNew<vtkCollectionIterator> markupsToModelNodeIt;
  markupsToModelNodeIt->SetCollection(markupsToModelNodes);
  for (markupsToModelNodeIt->InitTraversal(); !markupsToModelNodeIt->IsDoneWithTraversal(); markupsToModelNodeIt->GoToNextItem())
  {
    vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast(markupsToModelNodeIt->GetCurrentObject());
    if (markupsToModelNode != NULL)
    {
      vtkWarningMacro("OnMRMLSceneEndImport: Module node added. Set the model pointer ");

      if (GetModelNodeName(markupsToModelNode).compare("") != 0 &&
          markupsToModelNode->GetModelNode() != NULL)
      {
        vtkMRMLNode* modelNodeFromScene = this->GetMRMLScene()->GetNodeByID(markupsToModelNode->GetModelNode()->GetID());
        if (modelNodeFromScene != NULL)
        {
          markupsToModelNode->SetAndObserveModelNodeID(modelNodeFromScene->GetID());
        }
        else
        {
          vtkWarningMacro("Did not find the saved Model node.");
        }
      }
    }
  }
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneStartImport()
{
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    vtkWarningMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or node");
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast(node);
  if (markupsToModelNode)
  {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Module node added.");
    vtkUnObserveMRMLNodeMacro(markupsToModelNode); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLMarkupsToModelNode::MarkupsPositionModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(markupsToModelNode, events.GetPointer());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    vtkWarningMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or node");
    return;
  }

  if (node->IsA("vtkSlicerMarkupsToModelNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeRemoved");
    vtkUnObserveMRMLNodeMacro(node);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::SetMarkupsNode(vtkMRMLMarkupsFiducialNode* newMarkups, vtkMRMLMarkupsToModelNode* moduleNode)
{
  if (moduleNode == NULL)
  {
    vtkWarningMacro("SetWatchedModelNode: Module node is invalid");
    return;
  }

  vtkMRMLMarkupsFiducialNode* previousMarkups = moduleNode->GetMarkupsNode();
  if (previousMarkups == newMarkups)
  {
    // no change
    return;
  }
  // Switch to the new model node
  moduleNode->SetAndObserveMarkupsNodeID((newMarkups != NULL) ? newMarkups->GetID() : NULL);
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateSelectionNode(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{
  if (!markupsToModelModuleNode)
  {
    // No markups selected
    return;
  }
  vtkMRMLMarkupsFiducialNode* markupsNode = markupsToModelModuleNode->GetMarkupsNode();
  if (!markupsNode)
  {
    // No markups selected
    return;
  }
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("UpdateSelectionNode: no scene defined!");
    return;
  }

  // try the application logic first
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (mrmlAppLogic)
  {
    selectionNode = mrmlAppLogic->GetSelectionNode();
  }
  else
  {
    // try a default string
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  }
  if (!selectionNode)
  {
    vtkErrorMacro("UpdateSelectionNode: selection node is not available");
    return;
  }

  const char *activeID = markupsNode ? markupsNode->GetID() : NULL;
  if (!activeID)
  {
    return;
  }

  const char *selectionNodeActivePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  if (selectionNodeActivePlaceNodeID != NULL && activeID != NULL && !strcmp(selectionNodeActivePlaceNodeID, activeID))
  {
    // no change
    return;
  }

  selectionNode->SetReferenceActivePlaceNodeID(activeID);
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{
  if (markupsToModelModuleNode->GetModelNode() == NULL || markupsToModelModuleNode->GetMarkupsNode() == NULL)
  {
    return;
  }

  // store the output poly data in this pointer
  vtkSmartPointer<vtkPolyData> outputPolyData = vtkSmartPointer<vtkPolyData>::New();

  switch (markupsToModelModuleNode->GetModelType())
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: CreateClosedSurfaceUtil::UpdateOutputCloseSurfaceModel(markupsToModelModuleNode, outputPolyData); break;
  case vtkMRMLMarkupsToModelNode::Curve: CreateCurveUtil::UpdateOutputCurveModel(markupsToModelModuleNode, outputPolyData); break;
  }

  // assign the poly data to the model node
  vtkMRMLModelNode* modelNode = markupsToModelModuleNode->GetModelNode();
  if (modelNode == NULL)
  {
    modelNode = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLModelNode"));
    modelNode->SetName(GetModelNodeName(markupsToModelModuleNode).c_str());
  }
  modelNode->SetAndObservePolyData(outputPolyData);

  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode == NULL)
  {
    modelNode->CreateDefaultDisplayNodes();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
    displayNode->SetName(GetModelDisplayNodeName(markupsToModelModuleNode).c_str());
  }

  markupsToModelModuleNode->SetAndObserveModelNodeID(modelNode->GetID());
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast(caller);
  if (callerNode == NULL)
  {
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast(callerNode);
  if (markupsToModelModuleNode == NULL || !markupsToModelModuleNode->GetAutoUpdateOutput())
  {
    return;
  }

  if (this->GetMRMLScene() &&
    (this->GetMRMLScene()->IsImporting() ||
    this->GetMRMLScene()->IsRestoring() ||
    this->GetMRMLScene()->IsClosing()))
  {
    return;
  }

  if (event == vtkMRMLMarkupsToModelNode::MarkupsPositionModifiedEvent
    || event == vtkCommand::ModifiedEvent)
  {
    this->UpdateOutputModel(markupsToModelModuleNode);
  }
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetMarkupsNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLMarkupsNode* markupsNode = mrmlNode->GetMarkupsNode();
  if (markupsNode == NULL)
  {
    return std::string(mrmlNode->GetID()).append("Markups");
  }

  return std::string(markupsNode->GetName());
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetModelNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLModelNode* modelNode = mrmlNode->GetModelNode();
  if (modelNode == NULL)
  {
    return std::string(mrmlNode->GetID()).append("Model");
  }

  return std::string(modelNode->GetName());
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetMarkupsDisplayNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLMarkupsNode* markupsNode = mrmlNode->GetMarkupsNode();
  if (markupsNode == NULL)
  {
    return std::string(mrmlNode->GetID()).append("MarkupsDisplay");
  }

  vtkMRMLModelDisplayNode* markupsDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
  if (markupsDisplayNode == NULL)
  {
    return std::string(mrmlNode->GetID()).append("MarkupsDisplay");
  }

  return std::string(markupsDisplayNode->GetName());
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetModelDisplayNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLModelNode* modelNode = mrmlNode->GetModelNode();
  if (modelNode == NULL)
  {
    return std::string(mrmlNode->GetID()).append("ModelDisplay");
  }

  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (modelDisplayNode == NULL)
  {
    return std::string(mrmlNode->GetID()).append("ModelDisplay");
  }

  return std::string(modelDisplayNode->GetName());
}
