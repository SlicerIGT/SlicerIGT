/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// .NAME vtkSlicerToolBarManagerWidget - slicer helper class for managing toolbars widgets of watchdog module
// .SECTION Description
// This class adds, updates, and removes watchdog toolbars into slicer main window.

#ifndef __qSlicerToolBarManagerWidget_h
#define __qSlicerToolBarManagerWidget_h

// SlicerQt includes
#include "qSlicerWidget.h"
#include "qSlicerWatchdogModuleExport.h"
#include <QSound>
#include <QPointer>

class QTimer;

class vtkMRMLScene;
class qMRMLWatchdogToolBar;
class vtkMRMLWatchdogNode;
class vtkSlicerWatchdogLogic;

class Q_SLICER_QTMODULES_WATCHDOG_EXPORT qSlicerToolBarManagerWidget 
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;

  /// Constructor
  qSlicerToolBarManagerWidget(QWidget* parent = 0);
  virtual ~qSlicerToolBarManagerWidget();

  QTimer* Timer;

  QHash<QString, qMRMLWatchdogToolBar *> * GetToolBarHash();
  void setLogic(vtkSlicerWatchdogLogic* watchdogLogic);

public slots:
  /// When the scene is set the node added and removed events are connected to respective slots, to add or remove a toolbar
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  /// After the watchdog logic is updated the toolbar status will be updated accordingly and 
  /// and play the beep sound if activated
  void onUpdateToolBars();
  /// Every time the timer shoots will update the watchdog logic.
  void  onTimerEvent();
  /// When a watchdog node is removed watchdog toolbar widget from the QMainWindow as well.
  void RemoveToolBar(vtkObject* scene, vtkObject* node);
  /// When a watchdog node is added a watchdog toolbar widget as well.
  void AddToolBar(vtkObject*, vtkObject* nodeToBeRemoved);
  /// StatusRefreshTimeSec is time tool bar updates
  void setStatusRefreshTimeSec( double StatusRefreshTimeSec);


private:
  /// If there is not a watchdog toolbar widget in the QMainWindow.
  void InitializeToolBar(vtkMRMLWatchdogNode* watchdogNodeAdded );
  /// Initializes sound
  void setSound(std::string watchdogModuleShareDirectory);

  Q_DISABLE_COPY(qSlicerToolBarManagerWidget);

  QHash<QString, qMRMLWatchdogToolBar *> * WatchdogToolBarHash;
  QPointer<QSound>  WatchdogSound;
  double LastSoundElapsedTime;
  double StatusRefreshTimeSec;
  vtkSlicerWatchdogLogic * WatchdogLogic;
};

#endif
