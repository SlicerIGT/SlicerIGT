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

#ifndef __qSlicerPathExplorerModuleWidget_h
#define __qSlicerPathExplorerModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerPathExplorerModuleExport.h"

#include "qSlicerPathExplorerReslicingWidget.h"

#include "vtkMRMLSliceNode.h"


class QShortcut;
class qSlicerPathExplorerModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PATHEXPLORER_EXPORT qSlicerPathExplorerModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPathExplorerModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPathExplorerModuleWidget();

  virtual void enter();
  virtual void exit();

public slots:
  void onTrajectoryNodeActivated(vtkMRMLNode* node);
  void onMRMLSceneChanged(vtkMRMLScene* scene);
  void onEKeyPressed();
  void onTKeyPressed();
  void onAddPath();
  void onEntryNodeSelected();
  void onTargetNodeSelected();
  void selectedPathLineItem(vtkIdType);

protected:
  QScopedPointer<qSlicerPathExplorerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

  void updateGUIFromMRML();

private:
  Q_DECLARE_PRIVATE(qSlicerPathExplorerModuleWidget);
  Q_DISABLE_COPY(qSlicerPathExplorerModuleWidget);

  QShortcut *eToAddShortcut;
  QShortcut *tToAddShortcut;
};

#endif
