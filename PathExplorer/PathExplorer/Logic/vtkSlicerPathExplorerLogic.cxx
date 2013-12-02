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

// VTK includes
#include <vtkNew.h>

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

//---------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPathExplorerLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

