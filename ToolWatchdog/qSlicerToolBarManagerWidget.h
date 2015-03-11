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

#ifndef __qSlicerToolBarManagerWidget_h
#define __qSlicerToolBarManagerWidget_h

//// Qt includes
////#include <QSignalMapper>
//#include <QToolBar>
//
//// CTK includes
//#include <ctkPimpl.h>
//// no ui begin
//#include <ctkVTKObject.h>
//// no ui end

// qMRMLWidget includes
#include "qSlicerWidget.h"
#include "qMRMLWidget.h"
#include "qSlicerWatchdogModuleExport.h"

class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLViewNode;
class qMRMLWatchdogToolBar;
class vtkMRMLWatchdogNode;
//class qSlicerToolBarManagerWidgetPrivate;

class Q_SLICER_QTMODULES_WATCHDOG_EXPORT qSlicerToolBarManagerWidget : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  //qSlicerToolBarManagerWidget();
  qSlicerToolBarManagerWidget(QWidget* parent = 0);
  virtual ~qSlicerToolBarManagerWidget();
  
  //void SetFirstlabel(char * watchDogNodeName);
  //void ToolNodeAdded(const char * toolName);
  //void SwapToolNodes(int toolA, int toolB );
  //void ToolNodeDeleted();
  //void DeleteToolNode(int row);
  //void SetNodeStatus(int row, bool status );
  //void SetNodeLabel(int row, const char * toolLabel);

public slots:
  virtual void setMRMLScene(vtkMRMLScene* newScene);


//signals:
//  //void screenshotButtonClicked();
//  //void sceneViewButtonClicked();
//  void mrmlSceneChanged(vtkMRMLScene*);
//
//protected:
//  QScopedPointer<qSlicerToolBarManagerWidgetPrivate> d_ptr;

private:
  void InitializeToolbar(vtkMRMLWatchdogNode* watchdogNodeAdded );
  //Q_DECLARE_PRIVATE(qSlicerToolBarManagerWidget);
  //Q_DISABLE_COPY(qSlicerToolBarManagerWidget);

  QHash<QString, qMRMLWatchdogToolBar *> * WatchdogToolbarHash;
};

#endif
