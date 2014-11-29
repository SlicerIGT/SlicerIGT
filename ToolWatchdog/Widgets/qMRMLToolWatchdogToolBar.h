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

#ifndef __qMRMLToolWatchdogToolBar_h
#define __qMRMLToolWatchdogToolBar_h

// Qt includes
//#include <QSignalMapper>
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
// no ui begin
#include <ctkVTKObject.h>
// no ui end

// qMRMLWidget includes
#include "qMRMLWidget.h"
#include "qMRMLToolWatchdogModuleWidgetsExport.h"

class qMRMLToolWatchdogToolBarPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLViewNode;

class QMRML_MODULE_TOOLWATCHDOG_WIDGETS_EXPORT qMRMLToolWatchdogToolBar : public QToolBar
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qMRMLToolWatchdogToolBar(const QString& title, QWidget* parent = 0);
  qMRMLToolWatchdogToolBar(QWidget* parent = 0);
  virtual ~qMRMLToolWatchdogToolBar();

  void ToolNodeAdded();
  void ToolNodeDeleted();
  void SetNodeStatus(int row, bool status );

public slots:
  virtual void setMRMLScene(vtkMRMLScene* newScene);
  void setActiveMRMLThreeDViewNode(vtkMRMLViewNode * newActiveMRMLThreeDViewNode);



signals:
  void screenshotButtonClicked();
  void sceneViewButtonClicked();
  void mrmlSceneChanged(vtkMRMLScene*);

protected:
  QScopedPointer<qMRMLToolWatchdogToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLToolWatchdogToolBar);
  Q_DISABLE_COPY(qMRMLToolWatchdogToolBar);
};

#endif
