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

#include "qSlicerPathExplorerMarkupsTableWidget.h"
#include "qSlicerPathExplorerReslicingWidget.h"
#include "qSlicerPathExplorerTrajectoryTableWidget.h"

#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLSliceNode.h"

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

public slots:
  void onEntryNodeActivated(vtkMRMLNode* node);
  void onTargetNodeActivated(vtkMRMLNode* node);
  void onTrajectoryNodeActivated(vtkMRMLNode* node);
  void onMRMLSceneChanged(vtkMRMLScene* scene);
  void onEntryAddButtonToggled(bool state);
  void onTargetAddButtonToggled(bool state);

protected:
  QScopedPointer<qSlicerPathExplorerModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerPathExplorerModuleWidget);
  Q_DISABLE_COPY(qSlicerPathExplorerModuleWidget);
};

#endif
