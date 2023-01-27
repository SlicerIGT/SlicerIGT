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

// PathExplorer Logic includes
#include "vtkSlicerPathExplorerLogic.h"

// MRML includes
#include "vtkMRMLPathPlannerTrajectoryNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPathExplorerLogic);

//----------------------------------------------------------------------------
vtkSlicerPathExplorerLogic::vtkSlicerPathExplorerLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPathExplorerLogic::~vtkSlicerPathExplorerLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  vtkSmartPointer<vtkMRMLPathPlannerTrajectoryNode> trajectoryNode
    = vtkSmartPointer<vtkMRMLPathPlannerTrajectoryNode>::New();
  this->GetMRMLScene()->RegisterNodeClass(trajectoryNode.GetPointer());
}

//------------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::UpdateTrajectory(vtkMRMLPathPlannerTrajectoryNode* trajectoryNode)
{
  if (!trajectoryNode || !this->GetMRMLScene())
  {
    return;
  }

  // Create missing paths
  int numberOfPaths = trajectoryNode->GetNumberOfPaths();
  for (int pathIndex = 0; pathIndex < numberOfPaths; pathIndex++)
  {
    vtkMRMLMarkupsLineNode* pathLineNode = trajectoryNode->GetNthPathLineNode(pathIndex);
    vtkMRMLMarkupsNode* entryPoints = trajectoryNode->GetEntryPointsNode();
    int entryPointIndex = (entryPoints ? trajectoryNode->GetNthEntryPointIndex(pathIndex) : -1);
    vtkMRMLMarkupsNode* targetPoints = trajectoryNode->GetTargetPointsNode();    
    int targetPointIndex = (targetPoints ? trajectoryNode->GetNthTargetPointIndex(pathIndex) : -1);
    if (entryPointIndex<0 || targetPointIndex<0)
    {
      if (pathLineNode)
        {
        pathLineNode->RemoveAllControlPoints();
        }
      continue;
    }
    // We have a valid path, make sure there is a line node for it
    // Make sure we have a valid path node
    if (!pathLineNode)
    {
      pathLineNode = vtkMRMLMarkupsLineNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsLineNode"));
      pathLineNode->SetLocked(true); // we want to interact with the target/entry points and not with the line endpoints
      trajectoryNode->SetNthPathLineNode(pathIndex, pathLineNode);
      vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
      if (shNode)
      {
        shNode->SetItemParent(shNode->GetItemByDataNode(pathLineNode), shNode->GetItemByDataNode(trajectoryNode));
      }
    }
    if (!pathLineNode)
    {
      // failed to get/create line node
      continue;
    }
    double entryPointPositionWorld[3] = { 0.0 };
    entryPoints->GetNthControlPointPositionWorld(entryPointIndex, entryPointPositionWorld);
    double targetPointPositionWorld[3] = { 0.0 };
    targetPoints->GetNthControlPointPositionWorld(targetPointIndex, targetPointPositionWorld);
    if (pathLineNode->GetNumberOfControlPoints() < 1)
    {
      pathLineNode->AddControlPointWorld(vtkVector3d(entryPointPositionWorld));
    }
    else
    {
      pathLineNode->SetNthControlPointPositionWorld(0, entryPointPositionWorld);
    }
    if (pathLineNode->GetNumberOfControlPoints() < 2)
    {
      pathLineNode->AddControlPointWorld(vtkVector3d(targetPointPositionWorld));
    }
    else
    {
      pathLineNode->SetNthControlPointPositionWorld(1, targetPointPositionWorld);
    }
    // Update name from point names
    std::string lineName = std::string(entryPoints->GetNthControlPointLabel(entryPointIndex))
      + " -> " + targetPoints->GetNthControlPointLabel(targetPointIndex);
    pathLineNode->SetName(lineName.c_str());
  }
}

//------------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    vtkWarningMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or node");
    return;
  }

  vtkMRMLPathPlannerTrajectoryNode* trajectoryNode = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(node);
  if (trajectoryNode)
  {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Module node added.");
    vtkUnObserveMRMLNodeMacro(trajectoryNode); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(trajectoryNode, events.GetPointer());
  }
}

//------------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (node == NULL || this->GetMRMLScene() == NULL)
  {
    vtkWarningMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or node");
    return;
  }

  if (node->IsA("vtkMRMLPathPlannerTrajectoryNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeRemoved");
    vtkUnObserveMRMLNodeMacro(node);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast(caller);
  if (callerNode == NULL)
  {
    return;
  }

  vtkMRMLPathPlannerTrajectoryNode* trajectoryNode = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(callerNode);
  if (trajectoryNode == NULL)
  {
    return;
  }

  if (event == vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent)
  {
    // only recompute output if the input is changed
    // (for example we do not recompute the distance if the computed distance is changed)
    this->UpdateTrajectory(trajectoryNode);
  }
}

