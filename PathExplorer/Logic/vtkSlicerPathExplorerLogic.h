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

// .NAME vtkSlicerPathExplorerLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerPathExplorerLogic_h
#define __vtkSlicerPathExplorerLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLScene.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerPathExplorerModuleLogicExport.h"

class vtkMRMLPathPlannerTrajectoryNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_PATHEXPLORER_MODULE_LOGIC_EXPORT vtkSlicerPathExplorerLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPathExplorerLogic *New();
  vtkTypeMacro(vtkSlicerPathExplorerLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void UpdateTrajectory(vtkMRMLPathPlannerTrajectoryNode* trajectoryNode);

  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

protected:
  vtkSlicerPathExplorerLogic();
  virtual ~vtkSlicerPathExplorerLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes() override;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerPathExplorerLogic(const vtkSlicerPathExplorerLogic&); // Not implemented
  void operator=(const vtkSlicerPathExplorerLogic&);               // Not implemented
};

#endif
