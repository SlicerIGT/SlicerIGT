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

// .NAME vtkSlicerWatchdogLogic - slicer watchdog logic class for displayable nodes (tools)
// .SECTION Description
// This class manages the logic associated with displayable nodes watchdog. The watched nodes last time stamp is 
// stored and compared with the current one every timer shot. If the last and current time stamps are different it 
// has an updated status.

#ifndef __vtkSlicerWatchdogLogic_h
#define __vtkSlicerWatchdogLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

// For referencing own MRML node
class vtkMRMLWatchdogNode;
class vtkMRMLDisplayableNode;

#include "vtkSlicerWatchdogModuleLogicExport.h"

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_WATCHDOG_MODULE_LOGIC_EXPORT vtkSlicerWatchdogLogic : 
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerWatchdogLogic *New();
  vtkTypeMacro(vtkSlicerWatchdogLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///Every time the timer is reached this method updates the tools status and the elapsed time
  void UpdateAllWatchdogNodes(bool &watchedNodeBecomeUpToDateSound, bool &watchedNodeBecomeOutdatedSound);

  /// Create a new watchdog node and associated display node, adding both to
  /// the scene.
  /// On success, return the id, on failure return an empty string.
  std::string AddNewWatchdogNode(const char *name = "Watchdog", vtkMRMLScene *scene = NULL); 

  /// Create a new display node and make observe it by the watchdog node.
  /// On success, return the id, on failure return an empty string.
  /// If a display node already exists for this node, return the id of that
  /// node.
  std::string AddNewDisplayNodeForWatchdogNode(vtkMRMLNode *mrmlNode);

protected:
  vtkSlicerWatchdogLogic();
  virtual ~vtkSlicerWatchdogLogic();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

  /// Initialize listening to MRML events
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);

private:
  vtkSlicerWatchdogLogic(const vtkSlicerWatchdogLogic&); // Not implemented
  void operator=(const vtkSlicerWatchdogLogic&); // Not implemented
};

#endif
