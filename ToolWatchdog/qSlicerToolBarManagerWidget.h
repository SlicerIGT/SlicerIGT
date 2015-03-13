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

class vtkMRMLScene;
class qMRMLWatchdogToolBar;
class vtkMRMLWatchdogNode;

class Q_SLICER_QTMODULES_WATCHDOG_EXPORT qSlicerToolBarManagerWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;

  /// Constructor
  qSlicerToolBarManagerWidget(QWidget* parent = 0);
  virtual ~qSlicerToolBarManagerWidget();

  QHash<QString, qMRMLWatchdogToolBar *> * GetToolbarHash();

public slots:
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  ///Every time the logic timer shoots will update the main window watchdog toolbar status
  void onUpdateToolbars();
  /// It removes the  watchdog toolbar widget from the QMainWindow.
  void RemoveToolbar(vtkObject* scene, vtkObject* node);
  /// If there is not a watchdog toolbar widget in the QMainWindow. It will add add one for every watchdog node 
  void AddToolbar(vtkObject*, vtkObject* nodeToBeRemoved);

signals:

private:

  void InitializeToolbar(vtkMRMLWatchdogNode* watchdogNodeAdded );

  Q_DISABLE_COPY(qSlicerToolBarManagerWidget);

  QHash<QString, qMRMLWatchdogToolBar *> * WatchdogToolbarHash;
};

#endif
