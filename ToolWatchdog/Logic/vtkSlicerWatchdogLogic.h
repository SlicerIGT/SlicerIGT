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

// .NAME vtkSlicerWatchdogLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerWatchdogLogic_h
#define __vtkSlicerWatchdogLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
//#include <vtkLandmarkTransform.h>
#include "vtkSmartPointer.h"

// For referencing own MRML node
class vtkMRMLWatchdogNode;


class vtkMRMLDisplayableNode;

// STD includes
#include <cstdlib>

#include "vtkSlicerWatchdogModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_WATCHDOG_MODULE_LOGIC_EXPORT vtkSlicerWatchdogLogic :
  public vtkSlicerModuleLogic
{
public:

  enum ToolState
  {
    OUT_OF_DATE,
    UP_TO_DATE
  };

  static vtkSlicerWatchdogLogic *New();
  vtkTypeMacro(vtkSlicerWatchdogLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  //void SetObservedToolNode( vtkMRMLDisplayableNode* newTransform, vtkMRMLWatchdogNode* moduleNode );
  //void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );

  /// Adds a tool to the list in the respective toolwatchdog node
  void AddToolNode( vtkMRMLWatchdogNode* toolWatchdogNode, vtkMRMLDisplayableNode *mrmlNode);
  /// Updates the state of the tool observed according to the timestamp. The elapsedTime is stored to keep track of time that tools have been disconnected.
  void UpdateToolStatus( vtkMRMLWatchdogNode* toolWatchdogNode, unsigned long ElapsedTimeSec );

protected:
  vtkSlicerWatchdogLogic();
  virtual ~vtkSlicerWatchdogLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:

  vtkSlicerWatchdogLogic(const vtkSlicerWatchdogLogic&); // Not implemented
  void operator=(const vtkSlicerWatchdogLogic&); // Not implemented
};

#endif
