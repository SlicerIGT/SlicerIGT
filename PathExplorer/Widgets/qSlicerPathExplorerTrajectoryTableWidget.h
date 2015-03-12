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
 
  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898
 
==============================================================================*/

#ifndef __qSlicerPathExplorerTrajectoryTableWidget_h
#define __qSlicerPathExplorerTrajectoryTableWidget_h

#include <sstream>

// VTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerPathExplorerModuleWidgetsExport.h"
#include "qSlicerWidget.h"

#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

#include "vtkSlicerAnnotationModuleLogic.h"

// Qt includes
#include <QTableWidget>
#include <QTime>

class qSlicerPathExplorerTrajectoryTableWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLAnnotationHierarchyNode;
class vtkMRMLAnnotationFiducialNode;

class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerTrajectoryTableWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;
  typedef qSlicerPathExplorerTrajectoryTableWidget Self;
  qSlicerPathExplorerTrajectoryTableWidget(QWidget *parent=0);
  virtual ~qSlicerPathExplorerTrajectoryTableWidget();

  void setEntryMarkupsFiducialNode(vtkMRMLMarkupsFiducialNode* entryList);
  void setTargetMarkupsFiducialNode(vtkMRMLMarkupsFiducialNode* targetList);
  void setTrajectoryListNode(vtkMRMLAnnotationHierarchyNode* trajectoryList);

public slots:

  // Scene events
  void onMRMLSceneChanged(vtkMRMLScene* newScene);
  void onMRMLSceneClosed();
  void onRulerModified(vtkObject* caller);

  // GUI
  void onAddButtonClicked();
  void onRemoveButtonClicked();
  void onUpdateButtonClicked();
  void onClearButtonClicked();
  void onSelectionChanged();
  void onCellChanged(int row, int column);

  // Entry
  void setSelectedEntryMarkupID(vtkMRMLMarkupsFiducialNode* fNode, int entryMarkupIndex);
  void onEntryMarkupModified(vtkMRMLMarkupsFiducialNode* entryNode, int entryMarkupIndex);
  void onEntryMarkupRemoved(vtkMRMLMarkupsFiducialNode* entryNode, int removedMarkupIndex);

  // Target
  void setSelectedTargetMarkupID(vtkMRMLMarkupsFiducialNode* fNode, int targetMarkupIndex);
  void onTargetMarkupModified(vtkMRMLMarkupsFiducialNode* targetNode, int targetMarkupIndex);
  void onTargetMarkupRemoved(vtkMRMLMarkupsFiducialNode* targetNode, int targetMarkupIndex);

protected:
  QScopedPointer<qSlicerPathExplorerTrajectoryTableWidgetPrivate> d_ptr;

  enum ColumnType
  {
    RulerName = 0,
    EntryName = 1,
    TargetName = 2
  };

  enum CustomRole
  {
    RulerID = Qt::UserRole,
    EntryIndex,
    TargetIndex
  };

private:
  Q_DECLARE_PRIVATE(qSlicerPathExplorerTrajectoryTableWidget);
  Q_DISABLE_COPY(qSlicerPathExplorerTrajectoryTableWidget);

signals:
  void selectedRulerChanged(vtkMRMLAnnotationRulerNode*);
  void entryPointModified(vtkMRMLMarkupsFiducialNode*,int);
  void targetPointModified(vtkMRMLMarkupsFiducialNode*,int);
};

#endif // __qSlicerPathExplorerTrajectoryTableWidget_h
