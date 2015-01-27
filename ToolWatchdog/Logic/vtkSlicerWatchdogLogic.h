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

// .NAME vtkSlicerWatchdogLogic - slicer logic class for displayable nodes (tools) watchdog
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

//#include "vtkSmartPointer.h"
#include <QSound>
#include <QPointer>

// For referencing own MRML node
class vtkMRMLWatchdogNode;
class vtkMRMLDisplayableNode;
class QVTKSlicerWatchdogLogicInternal;

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

  /// Adds a tool to the list in the respective toolwatchdog node
  void AddToolNode( vtkMRMLWatchdogNode* toolWatchdogNode, vtkMRMLDisplayableNode *mrmlNode);
  /// Updates the state of the tool observed according to the timestamp. The elapsedTime is stored to keep track of time 
  ///that tools have been disconnected.
  void UpdateToolStatus( vtkMRMLWatchdogNode* toolWatchdogNode);
  ///Every time the timer is reached this method updates the tools status, the elapsed time and play the beep sound if 
  ///any watched tool (with playSound activated) is out-dated.
  void TimerEvent();
  vtkGetMacro(ElapsedTimeSec, double);
  void SetStatusRefreshTimeMiliSec( double statusRefeshRateMiliSec);
  QVTKSlicerWatchdogLogicInternal* GetQVTKLogicInternal();

protected:
  vtkSlicerWatchdogLogic();
  virtual ~vtkSlicerWatchdogLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  ///When a scene has been imported it will set the tools watched, the watchdog toolbar, and start up the timer.
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);


private:

  vtkSlicerWatchdogLogic(const vtkSlicerWatchdogLogic&); // Not implemented
  void operator=(const vtkSlicerWatchdogLogic&); // Not implemented
  double StatusRefreshTimeSec;
  double ElapsedTimeSec;
  double LastSoundElapsedTime;
  QVTKSlicerWatchdogLogicInternal* Internal;
  QPointer<QSound>  BreachSound;

};

#endif
