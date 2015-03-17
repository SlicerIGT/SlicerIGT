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

#ifndef __qSlicerPathExplorerMarkupsTableWidget_h
#define __qSlicerPathExplorerMarkupsTableWidget_h

#include <sstream>

// VTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerPathExplorerModuleWidgetsExport.h"
#include "qSlicerWidget.h"

#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

// Qt includes
#include <QTableWidget>
#include <QTime>

class qSlicerPathExplorerMarkupsTableWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLAnnotationHierarchyNode;
class vtkMRMLAnnotationFiducialNode;

class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerMarkupsTableWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;
  typedef qSlicerPathExplorerMarkupsTableWidget Self;
  qSlicerPathExplorerMarkupsTableWidget(QWidget *parent=0);
  virtual ~qSlicerPathExplorerMarkupsTableWidget();

  bool getAddButtonState();
  void setAddButtonState(bool state);
  void setAndObserveMarkupFiducialNode(vtkMRMLMarkupsFiducialNode* markupList);
  void setColor(double r, double g, double b);
  vtkMRMLMarkupsFiducialNode* getMarkupFiducialNode();
  const char* getSelectedMarkupID();
  
public slots:

  void setSelectedMarkup(vtkMRMLMarkupsFiducialNode* fNode, int markupIndex);

  // Scene events
  void onMRMLSceneChanged(vtkMRMLScene* newScene);
  void onMRMLSceneClosed();

  // GUI
  void onAddButtonToggled(bool pushed);
  void onRemoveButtonClicked();
  void onClearButtonClicked();
  void onSelectionChanged();
  void onCellChanged(int row, int column);
  
  // MarkupFiducialNode
  void onMarkupModified(vtkObject* /*caller*/, void* callData);
  void refreshMarkupTable();

protected:
  QScopedPointer<qSlicerPathExplorerMarkupsTableWidgetPrivate> d_ptr;

  void addMarkupInTable(vtkMRMLMarkupsFiducialNode* fNode, int markupIndex);
  void updateMarkupInTable(vtkMRMLMarkupsFiducialNode* fNode, int markupIndex, QModelIndex index);
  void populateTableWidget(vtkMRMLMarkupsFiducialNode* markupList);  

  enum ColumnType
  {
    Name = 0,
    R = 1,
    A = 2,
    S = 3,
    Time = 4
  };

  enum CustomRole
  {
    MarkupIndex = Qt::UserRole
  };

private:
  Q_DECLARE_PRIVATE(qSlicerPathExplorerMarkupsTableWidget);
  Q_DISABLE_COPY(qSlicerPathExplorerMarkupsTableWidget);

signals:
  void addButtonToggled(bool);
  void markupSelected(vtkMRMLMarkupsFiducialNode*, int);
  void markupModified(vtkMRMLMarkupsFiducialNode*, int);
  void markupRemoved(vtkMRMLMarkupsFiducialNode*, int);
};

#endif // __qSlicerPathExplorerMarkupsTableWidget_h
