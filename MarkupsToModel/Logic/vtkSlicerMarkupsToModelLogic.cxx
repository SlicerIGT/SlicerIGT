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
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkCellArray.h> 
#include <vtkDelaunay3D.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkButterflySubdivisionFilter.h>

#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelNode.h"

// STD includes
#include <cassert>

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
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast(node);
  if ( markupsToModelNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( markupsToModelNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLMarkupsToModelNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( markupsToModelNode, events.GetPointer() );
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{

  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::SetMarkupsNode( vtkMRMLMarkupsFiducialNode* newMarkups, vtkMRMLMarkupsToModelNode* moduleNode )
{
  if ( moduleNode == NULL )
  {
    vtkWarningMacro( "SetWatchedModelNode: Module node is invalid" );
    return;
  }

  // Get the original color of the old model node
  vtkMRMLMarkupsFiducialNode* previousMarkups = moduleNode->GetMarkupsNode();

  if (previousMarkups==newMarkups)
  {
    // no change
    return;
  }

  //double previousOriginalColor[3]={0.5,0.5,0.5};
  //if(previousMarkups)
  //{
  //  moduleNode->GetOriginalColor(previousOriginalColor);
  //}

  //// Save the original color of the new model node
  //if(newMarkups!=NULL)
  //{
  //  double originalColor[3]={0.5,0.5,0.5};
  //  if ( newMarkups->GetDisplayNode() != NULL )
  //  {
  //    newMarkups->GetDisplayNode()->GetColor(originalColor);
  //  }
  //  moduleNode->SetOriginalColor(originalColor);
  //}

  // Switch to the new model node
  moduleNode->SetAndObserveMarkupsNodeID( (newMarkups!=NULL) ? newMarkups->GetID() : NULL );

  //// Restore the color of the old model node
  //if(previousMarkups!=NULL && previousMarkups->GetDisplayNode()!=NULL)
  //{
  //  previousMarkups->GetDisplayNode()->SetColor(previousOriginalColor[0],previousOriginalColor[1],previousOriginalColor[2]);
  //}
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{
  vtkMRMLMarkupsFiducialNode* markups=markupsToModelModuleNode->GetMarkupsNode(); 
  int numberOfMarkups = markups->GetNumberOfFiducials();
  
  if(numberOfMarkups<10)
  {
    vtkWarningMacro("Not enough fiducials");
    return;
  }

  if(markupsToModelModuleNode->GetModelNode() != NULL)
  {
    //this->GetMRMLScene()->RemoveNodeID(markupsToModelModuleNode->GetModelNodeID());
    this->GetMRMLScene()->RemoveNode(markupsToModelModuleNode->GetModelNode());
  }

  vtkSmartPointer< vtkPoints > modelPoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkCellArray > modelCellArray = vtkSmartPointer< vtkCellArray >::New();

  modelPoints->SetNumberOfPoints(numberOfMarkups);
  double markupPoint [3] = {0.0, 0.0, 0.0};

  for (int i=0; i<numberOfMarkups;i++)
  {
    markups->GetNthFiducialPosition(i,markupPoint);
    modelPoints->SetPoint(i, markupPoint);
  }

  modelCellArray->InsertNextCell(numberOfMarkups);
  for (int i=0; i < numberOfMarkups;i++)
  {
    modelCellArray->InsertCellPoint(i);
  }

  vtkSmartPointer< vtkPolyData > pointPolyData = vtkSmartPointer< vtkPolyData >::New();
  pointPolyData->SetLines(modelCellArray);
  pointPolyData->SetPoints(modelPoints);

  vtkSmartPointer< vtkDelaunay3D > delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetInputData(pointPolyData); //TODO SET VTK5

  vtkSmartPointer< vtkDataSetSurfaceFilter > surfaceFilter = vtkSmartPointer< vtkDataSetSurfaceFilter >::New();
  surfaceFilter->SetInputConnection(delaunay->GetOutputPort());

  vtkSmartPointer< vtkButterflySubdivisionFilter > subdivisionFilter = vtkSmartPointer< vtkButterflySubdivisionFilter >::New();
  subdivisionFilter->SetInputConnection(surfaceFilter->GetOutputPort());
  subdivisionFilter->SetNumberOfSubdivisions(3);
  subdivisionFilter->Update();

  vtkSmartPointer< vtkMRMLModelNode > modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( modelNode );

  vtkWarningMacro("PERRAS "<< markupsToModelModuleNode->GetModelNodeName() );
  modelNode->SetName( markupsToModelModuleNode->GetModelNodeName().c_str() );
  modelNode->SetAndObservePolyData( subdivisionFilter->GetOutput() );

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( displayNode );
  displayNode->SetName( markupsToModelModuleNode->GetDisplayNodeName().c_str());

  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  markupsToModelModuleNode->SetModelNode(modelNode);

}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkWarningMacro("PUTAS");
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if (callerNode == NULL)
  {
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( callerNode );
  if (markupsToModelNode == NULL)
  {
    return;
  }

  if (event==vtkMRMLMarkupsToModelNode::InputDataModifiedEvent)
  {
    // only recompute output if the input is changed
    // (for example we do not recompute the distance if the computed distance is changed)
    this->UpdateOutputModel(markupsToModelNode); 
  }
}


