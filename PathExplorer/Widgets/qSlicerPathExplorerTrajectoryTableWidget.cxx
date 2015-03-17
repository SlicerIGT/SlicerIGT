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

#include "vtkSlicerVersionConfigure.h"

// PathExplorer Widgets includes
#include "qSlicerPathExplorerTrajectoryTableWidget.h"
#include "ui_qSlicerPathExplorerTrajectoryTableWidget.h"

// VTK includes
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathExplorer
class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerTrajectoryTableWidgetPrivate
  : public Ui_qSlicerPathExplorerTrajectoryTableWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathExplorerTrajectoryTableWidget);
 protected:
  qSlicerPathExplorerTrajectoryTableWidget * const q_ptr;
 public:
  qSlicerPathExplorerTrajectoryTableWidgetPrivate(
    qSlicerPathExplorerTrajectoryTableWidget& object);
  virtual void setupUi(qSlicerPathExplorerTrajectoryTableWidget*);

 public:
  vtkMRMLMarkupsFiducialNode* EntryNode;
  vtkMRMLMarkupsFiducialNode* TargetNode;
  vtkMRMLAnnotationHierarchyNode* TrajectoryNode; 
  int SelectedEntryMarkupIndex;
  int SelectedTargetMarkupIndex;
  int SelectedTrajectoryIndex;
};

//-----------------------------------------------------------------------------
qSlicerPathExplorerTrajectoryTableWidgetPrivate
::qSlicerPathExplorerTrajectoryTableWidgetPrivate(
  qSlicerPathExplorerTrajectoryTableWidget& object)
  : q_ptr(&object)
{
  this->EntryNode = NULL;
  this->TargetNode = NULL;
  this->TrajectoryNode = NULL;
  this->SelectedEntryMarkupIndex = -1;
  this->SelectedTargetMarkupIndex = -1;
  this->SelectedTrajectoryIndex = -1;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidgetPrivate
::setupUi(qSlicerPathExplorerTrajectoryTableWidget* widget)
{
  this->Ui_qSlicerPathExplorerTrajectoryTableWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerTrajectoryTableWidget
::qSlicerPathExplorerTrajectoryTableWidget(QWidget *parentWidget)
  : Superclass (parentWidget)
    , d_ptr( new qSlicerPathExplorerTrajectoryTableWidgetPrivate(*this) )
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);
  d->setupUi(this);

  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
	  this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));

  connect(d->AddButton, SIGNAL(clicked()),
          this, SLOT(onAddButtonClicked()));

  connect(d->RemoveButton, SIGNAL(clicked()),
          this, SLOT(onRemoveButtonClicked()));

  connect(d->UpdateButton, SIGNAL(clicked()),
          this, SLOT(onUpdateButtonClicked()));

  connect(d->ClearButton, SIGNAL(clicked()),
          this, SLOT(onClearButtonClicked()));

  connect(d->TableWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(onSelectionChanged()));

  connect(d->TableWidget, SIGNAL(cellChanged(int,int)),
          this, SLOT(onCellChanged(int,int)));
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerTrajectoryTableWidget
::~qSlicerPathExplorerTrajectoryTableWidget()
{
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  if (!newScene)
    {
    return;
    }

  this->qvtkReconnect(this->mrmlScene(), 
		      newScene, vtkMRMLScene::EndCloseEvent,
		      this, SLOT(onMRMLSceneClosed()));
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onMRMLSceneClosed()
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->TableWidget)
    {
    return;
    }

  d->TableWidget->setRowCount(0);
  d->TableWidget->clearContents();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onAddButtonClicked()
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);
  
  if (!this->mrmlScene() || !d->TableWidget || 
      !d->EntryNode || !d->TargetNode ||
      !d->EntryNode->MarkupExists(d->SelectedEntryMarkupIndex) ||
      !d->TargetNode->MarkupExists(d->SelectedTargetMarkupIndex) ||
      !d->TrajectoryNode)
    {
    return;
    }

  // Check if such trajectory already exists
  QAbstractItemModel* model = d->TableWidget->model();
  if (model)
    {
    QModelIndexList found = model->match(model->index(0,Self::EntryName),
					 Self::EntryIndex, d->SelectedEntryMarkupIndex, 
					 -1, Qt::MatchExactly);

    for (int i = 0; i < found.count(); ++i)
      {
      int row = found[i].row();
      if (row >= 0)
	{
	int tmpTargetIndex = d->TableWidget->item(row, Self::TargetName)->data(Self::TargetIndex).toInt();
	if (tmpTargetIndex == d->SelectedTargetMarkupIndex)
	  {
	  // Found trajectory with same entry and target points
	  return;
	  }
	}
      }
    }

  // Add new trajectory
  double entryMarkupPosition[3];
  d->EntryNode->GetNthFiducialPosition(d->SelectedEntryMarkupIndex, entryMarkupPosition);
  double targetMarkupPosition[3];
  d->TargetNode->GetNthFiducialPosition(d->SelectedTargetMarkupIndex, targetMarkupPosition);

  vtkSmartPointer<vtkMRMLAnnotationRulerNode> ruler = 
    vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
  ruler->SetPosition1(entryMarkupPosition);
  ruler->SetPosition2(targetMarkupPosition);
  ruler->Initialize(this->mrmlScene());

  this->qvtkConnect(ruler.GetPointer(), vtkCommand::ModifiedEvent,
		    this, SLOT(onRulerModified(vtkObject*)));

  // Update TableWidget
  int rowCount = d->TableWidget->rowCount();
  d->TableWidget->insertRow(rowCount);
  
  QTableWidgetItem* rulerItem  = new QTableWidgetItem(QString(ruler->GetName()));
  QTableWidgetItem* entryItem  = new QTableWidgetItem(d->EntryNode->GetNthMarkupLabel(d->SelectedEntryMarkupIndex).c_str());
  QTableWidgetItem* targetItem = new QTableWidgetItem(d->TargetNode->GetNthMarkupLabel(d->SelectedTargetMarkupIndex).c_str());
  rulerItem->setData(Self::RulerID, ruler->GetID());
  entryItem->setData(Self::EntryIndex, d->SelectedEntryMarkupIndex);
  targetItem->setData(Self::TargetIndex, d->SelectedTargetMarkupIndex);

  entryItem->setFlags(entryItem->flags() & ~Qt::ItemIsEditable);
  targetItem->setFlags(targetItem->flags() & ~Qt::ItemIsEditable);

  d->TableWidget->setItem(rowCount, Self::RulerName, rulerItem);
  d->TableWidget->setItem(rowCount, Self::EntryName, entryItem);
  d->TableWidget->setItem(rowCount, Self::TargetName, targetItem);
  
  d->TableWidget->selectRow(rowCount);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onRemoveButtonClicked()
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->TableWidget || !this->mrmlScene())
    {
    return;
    }

  int row = d->TableWidget->currentRow();
  if (row < 0 || row >= d->TableWidget->rowCount())
    {
    return;
    }

  QString rulerID = QString(d->TableWidget->item(row, Self::RulerName)->data(Self::RulerID).toString());
  if (!rulerID.isNull() && !rulerID.isEmpty())
    {
    vtkMRMLNode* rulerNode = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
    this->mrmlScene()->RemoveNode(rulerNode);
    d->TableWidget->removeRow(row);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onUpdateButtonClicked()
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->TableWidget || !this->mrmlScene() ||
      !d->EntryNode || !d->TargetNode)
    {
    return;
    }

  if (d->SelectedTrajectoryIndex < 0)
    {
    return;
    }

  if (d->EntryNode->MarkupExists(d->SelectedEntryMarkupIndex) &&
      d->TargetNode->MarkupExists(d->SelectedTargetMarkupIndex))
    {
    d->TableWidget->item(d->SelectedTrajectoryIndex, Self::EntryName)->setText(d->EntryNode->GetNthMarkupLabel(d->SelectedEntryMarkupIndex).c_str());
    d->TableWidget->item(d->SelectedTrajectoryIndex, Self::EntryName)->setData(Self::EntryIndex, d->SelectedEntryMarkupIndex);
      
    d->TableWidget->item(d->SelectedTrajectoryIndex, Self::TargetName)->setText(d->TargetNode->GetNthMarkupLabel(d->SelectedTargetMarkupIndex).c_str());
    d->TableWidget->item(d->SelectedTrajectoryIndex, Self::TargetName)->setData(Self::TargetIndex, d->SelectedTargetMarkupIndex); 
      
    QString rulerID = QString(d->TableWidget->item(d->SelectedTrajectoryIndex, Self::RulerName)->data(Self::RulerID).toString());
      
    if (!rulerID.isNull() && !rulerID.isEmpty())
      {
      vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
      vtkMRMLAnnotationRulerNode* rulerNode =
	vtkMRMLAnnotationRulerNode::SafeDownCast(node);
      if (rulerNode)
	{
	// Necessary to block signals otherwise markups are updated 
	// when only first one is set
	double entryMarkupPosition[3];
	d->EntryNode->GetNthFiducialPosition(d->SelectedEntryMarkupIndex, entryMarkupPosition);
	double targetMarkupPosition[3];
	d->TargetNode->GetNthFiducialPosition(d->SelectedTargetMarkupIndex, targetMarkupPosition);
	  
	this->qvtkDisconnect(rulerNode, vtkCommand::ModifiedEvent, 
			     this, SLOT(onRulerModified(vtkObject*)));
	  
	rulerNode->SetPosition1(entryMarkupPosition);
	rulerNode->SetPosition2(targetMarkupPosition);
	  
	this->qvtkConnect(rulerNode, vtkCommand::ModifiedEvent,
			  this, SLOT(onRulerModified(vtkObject*)));
	}
      }
    }
  
  d->UpdateButton->setEnabled(0);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onClearButtonClicked()
{
   Q_D(qSlicerPathExplorerTrajectoryTableWidget);

   if (!d->TableWidget)
     {
     return;
     }

   int rowCount = d->TableWidget->rowCount();
   for (int i = 0; i < rowCount; ++i)
     {
     QString rulerID = QString(d->TableWidget->item(i, Self::RulerName)->data(Self::RulerID).toString());
     if (!rulerID.isNull() && !rulerID.isEmpty())
       {
       vtkMRMLNode* rulerNode = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
       this->mrmlScene()->RemoveNode(rulerNode);
       }
     }
   d->TableWidget->setRowCount(0);
   d->TableWidget->clearContents();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onSelectionChanged()
{
   Q_D(qSlicerPathExplorerTrajectoryTableWidget);

   if (!d->TableWidget ||
       !d->EntryNode || !d->TargetNode)
     {
     return;
     }

   d->SelectedTrajectoryIndex = d->TableWidget->currentRow();
   int entryMarkupIndex = -1;
   int targetMarkupIndex = -1;

   if (d->SelectedTrajectoryIndex < 0)
     {
     return;
     }
   
   entryMarkupIndex = d->TableWidget->item(d->SelectedTrajectoryIndex, Self::EntryName)->data(Self::EntryIndex).toInt();
   targetMarkupIndex = d->TableWidget->item(d->SelectedTrajectoryIndex, Self::TargetName)->data(Self::TargetIndex).toInt();

   if (!d->EntryNode->MarkupExists(entryMarkupIndex) || 
       !d->TargetNode->MarkupExists(targetMarkupIndex))
     {
     return;
     }

   d->UpdateButton->setEnabled(0);
   
   QString rulerID = QString(d->TableWidget->item(d->SelectedTrajectoryIndex, Self::RulerName)->data(Self::RulerID).toString());
   QString rulerName = QString(d->TableWidget->item(d->SelectedTrajectoryIndex, Self::RulerName)->text());
   if (!rulerID.isNull() && !rulerID.isEmpty())
     {
     vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
     vtkMRMLAnnotationRulerNode* rulerNode =
       vtkMRMLAnnotationRulerNode::SafeDownCast(node);
     if (rulerNode)
       {
       emit entryPointModified(d->EntryNode, entryMarkupIndex);
       emit targetPointModified(d->TargetNode, targetMarkupIndex);
       }
     emit selectedRulerChanged(rulerNode);
     }   
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onCellChanged(int row, int /*column*/)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->TableWidget)
    {
    return;
    }

  QString rulerID = QString(d->TableWidget->item(row, Self::RulerName)->data(Self::RulerID).toString());
  QString rulerName = QString(d->TableWidget->item(row, Self::RulerName)->text());
  if (!rulerID.isNull() && !rulerID.isEmpty())
    {
    vtkMRMLNode* rulerNode = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
    rulerNode->SetName(rulerName.toStdString().c_str());
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::setEntryMarkupsFiducialNode(vtkMRMLMarkupsFiducialNode* entryList)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!entryList)
    {
    return;
    }
  
  d->EntryNode = entryList;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::setTargetMarkupsFiducialNode(vtkMRMLMarkupsFiducialNode* targetList)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!targetList)
    {
    return;
    }
  
  d->TargetNode = targetList;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::setTrajectoryListNode(vtkMRMLAnnotationHierarchyNode* trajectoryList)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!trajectoryList)
    {
    return;
    }

  d->TrajectoryNode = trajectoryList;

  // TODO: For now clear all annotations in the hierarchy
  // Later should populate table with rulers if already inside
  // Require to be able to find correct markups used to create the ruler
  d->TrajectoryNode->RemoveAllChildrenNodes();

  // Set active hierachy node
  qSlicerAbstractCoreModule* annotationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Annotations");
  vtkSlicerAnnotationModuleLogic* annotationLogic;
  if (annotationModule)
    {
    annotationLogic =
      vtkSlicerAnnotationModuleLogic::SafeDownCast(annotationModule->logic());
    }

  if (annotationLogic->GetActiveHierarchyNode() != d->TrajectoryNode)
    {
    annotationLogic->SetActiveHierarchyNodeID(d->TrajectoryNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::setSelectedEntryMarkupID(vtkMRMLMarkupsFiducialNode* fNode, int entryMarkupIndex)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  d->SelectedEntryMarkupIndex = entryMarkupIndex;

  if (!d->TableWidget || !fNode || 
      !fNode->MarkupExists(entryMarkupIndex))
    {
    return;
    }

  if (d->SelectedTrajectoryIndex < 0 || d->SelectedTrajectoryIndex >= d->TableWidget->rowCount())
    {
    return;
    }

  int currentEntryMarkupIndex = d->TableWidget->item(d->SelectedTrajectoryIndex, Self::EntryName)->data(Self::EntryIndex).toInt();
  if (currentEntryMarkupIndex != d->SelectedEntryMarkupIndex)
    {
    d->UpdateButton->setEnabled(1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::setSelectedTargetMarkupID(vtkMRMLMarkupsFiducialNode* fNode, int targetMarkupIndex)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->TableWidget || !fNode ||
      !fNode->MarkupExists(targetMarkupIndex))
    {
    return;
    }

  d->SelectedTargetMarkupIndex = targetMarkupIndex;

  if (d->SelectedTrajectoryIndex < 0 || d->SelectedTrajectoryIndex >= d->TableWidget->rowCount())
    {
    return;
    }

  int currentTargetMarkupIndex = d->TableWidget->item(d->SelectedTrajectoryIndex, Self::TargetName)->data(Self::TargetIndex).toInt();
  if (currentTargetMarkupIndex != d->SelectedTargetMarkupIndex)
    {
    d->UpdateButton->setEnabled(1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onEntryMarkupRemoved(vtkMRMLMarkupsFiducialNode*, int removedMarkupIndex)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);
  
  if (!d->TableWidget)
    {
    return;
    }
  
  // Look for all ruler using this markup ID as entry point
  QAbstractItemModel* model = d->TableWidget->model();
  if (model)
    {
    QModelIndexList found = model->match(model->index(0,Self::EntryName),
					 Self::EntryIndex, removedMarkupIndex, 
					 -1, Qt::MatchExactly);
    
    // Remove rulers from the end to avoid updating row numbers of items on top
    bool test = d->TableWidget->blockSignals(true);
    int foundCount = found.count();
    for (int i = 0; i < foundCount ; ++i)
      {
      int currentRow = found[foundCount-1 - i].row();
      QString rulerID = QString(d->TableWidget->item(currentRow, Self::RulerName)->data(Self::RulerID).toString());
      if (!rulerID.isNull() && !rulerID.isEmpty())
	{
	vtkMRMLNode* rulerNode = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
	this->mrmlScene()->RemoveNode(rulerNode);
	d->TableWidget->removeRow(currentRow);
	d->SelectedTrajectoryIndex = d->TableWidget->currentRow();
	}
      }
    d->TableWidget->blockSignals(test);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onTargetMarkupRemoved(vtkMRMLMarkupsFiducialNode*, int removedMarkupIndex)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);
  
  if (!d->TableWidget)
    {
    return;
    }
  
  // Look for all ruler using this markup ID as entry point
  QAbstractItemModel* model = d->TableWidget->model();
  if (model)
    {
    QModelIndexList found = model->match(model->index(0,Self::TargetName),
					 Self::TargetIndex, removedMarkupIndex, 
					 -1, Qt::MatchExactly);
    
    bool test = d->TableWidget->blockSignals(true);
    int foundCount = found.count();
    for (int i = 0; i < foundCount; ++i)
      {
      int currentRow = found[foundCount-1 - i].row();
      QString rulerID = QString(d->TableWidget->item(currentRow, Self::RulerName)->data(Self::RulerID).toString());
      if (!rulerID.isNull() && !rulerID.isEmpty())
	{
	vtkMRMLNode* rulerNode = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
	this->mrmlScene()->RemoveNode(rulerNode);
	d->TableWidget->removeRow(currentRow);
	d->SelectedTrajectoryIndex = d->TableWidget->currentRow();
	}
      }
    d->TableWidget->blockSignals(test);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onEntryMarkupModified(vtkMRMLMarkupsFiducialNode* entryNode, int entryMarkupIndex)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->EntryNode || !entryNode ||
      d->EntryNode != entryNode ||
      !d->TableWidget || !this->mrmlScene() ||
      !entryNode->MarkupExists(entryMarkupIndex))
    {
    return;
    }

  std::string markupName = d->EntryNode->GetNthMarkupLabel(entryMarkupIndex);
  double markupPosition[3];
  d->EntryNode->GetNthFiducialPosition(entryMarkupIndex, markupPosition);

  // Check if this markup is used in a trajectory
  QAbstractItemModel* model = d->TableWidget->model();
  if (model)
    {
    QModelIndexList found = model->match(model->index(0,Self::EntryName),
					 Self::EntryIndex, entryMarkupIndex, 
					 -1, Qt::MatchExactly);

    for (int i = 0; i < found.count(); ++i)
      {
      int currentRow = found[i].row();
      if (currentRow >= 0)
	{
	// Update Name
	d->TableWidget->item(currentRow, Self::EntryName)->setText(markupName.c_str());
	
	// Update ruler position
	QString rulerID = QString(d->TableWidget->item(currentRow, Self::RulerName)->data(Self::RulerID).toString());
	if (!rulerID.isNull() || !rulerID.isEmpty())
	  {
	  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
	  vtkMRMLAnnotationRulerNode* ruler = 
	    vtkMRMLAnnotationRulerNode::SafeDownCast(node);
	  if (ruler)
	    {
	    ruler->SetPosition1(markupPosition);
	    }
	  }
	}
      }
    }

}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onTargetMarkupModified(vtkMRMLMarkupsFiducialNode* targetNode, int targetMarkupIndex)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->TargetNode || !targetNode ||
      d->TargetNode != targetNode ||
      !d->TableWidget || !this->mrmlScene() ||
      !targetNode->MarkupExists(targetMarkupIndex))
    {
    return;
    }

  std::string markupName = d->TargetNode->GetNthMarkupLabel(targetMarkupIndex);
  double markupPosition[3];
  d->TargetNode->GetNthFiducialPosition(targetMarkupIndex, markupPosition);

  // Check if this markup is used in a trajectory
  QAbstractItemModel* model = d->TableWidget->model();
  if (model)
    {
    QModelIndexList found = model->match(model->index(0,Self::TargetName),
					 Self::TargetIndex, targetMarkupIndex, 
					 -1, Qt::MatchExactly);

    for (int i = 0; i < found.count(); ++i)
      {
      int currentRow = found[i].row();
      if (currentRow >= 0)
	{
	// Update Name
	d->TableWidget->item(currentRow, Self::TargetName)->setText(markupName.c_str());
	
	// Update ruler position
	QString rulerID = QString(d->TableWidget->item(currentRow, Self::RulerName)->data(Self::RulerID).toString());
	if (!rulerID.isNull() || !rulerID.isEmpty())
	  {
	  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(rulerID.toStdString());
	  vtkMRMLAnnotationRulerNode* ruler = 
	    vtkMRMLAnnotationRulerNode::SafeDownCast(node);
	  if (ruler)
	    {
	    ruler->SetPosition2(markupPosition);
	    }
	  }
	}
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerTrajectoryTableWidget
::onRulerModified(vtkObject* caller)
{
  Q_D(qSlicerPathExplorerTrajectoryTableWidget);

  if (!d->EntryNode || !d->TargetNode ||
      !d->TableWidget)
    {
    return;
    }

  vtkMRMLAnnotationRulerNode* rulerNode =
    vtkMRMLAnnotationRulerNode::SafeDownCast(caller);
  if (rulerNode)
    {
    double p1[3] = {0.0, 0.0, 0.0};
    double p2[3] = {0.0, 0.0, 0.0};

    rulerNode->GetPosition1(p1);
    rulerNode->GetPosition2(p2);

    // Update markups
    QAbstractItemModel* model = d->TableWidget->model();
    if (model)
      {
      QModelIndexList found = model->match(model->index(0,Self::RulerName),
					   Self::RulerID, rulerNode->GetID(), 
					   1, Qt::MatchExactly);

      if (!found.isEmpty())
	{
	int row = found[0].row();
	int entryMarkupIndex = d->TableWidget->item(row, Self::EntryName)->data(Self::EntryIndex).toInt();
	if (entryMarkupIndex >= 0 && entryMarkupIndex < d->EntryNode->GetNumberOfMarkups())
	  {
	  d->EntryNode->SetNthFiducialPositionFromArray(entryMarkupIndex, p1);
	  }

	int targetMarkupIndex = d->TableWidget->item(row, Self::TargetName)->data(Self::TargetIndex).toInt();
	if (targetMarkupIndex >= 0 && targetMarkupIndex < d->TargetNode->GetNumberOfMarkups())
	  {
	  d->TargetNode->SetNthFiducialPositionFromArray(targetMarkupIndex, p2);
	  }
	}
      }
    }
}


