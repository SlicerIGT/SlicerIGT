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
#include "qSlicerPathExplorerMarkupsTableWidget.h"
#include "ui_qSlicerPathExplorerMarkupsTableWidget.h"

// VTK includes
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathExplorer
class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerMarkupsTableWidgetPrivate
  : public Ui_qSlicerPathExplorerMarkupsTableWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathExplorerMarkupsTableWidget);
 protected:
  qSlicerPathExplorerMarkupsTableWidget * const q_ptr;
 public:
  qSlicerPathExplorerMarkupsTableWidgetPrivate(
    qSlicerPathExplorerMarkupsTableWidget& object);
  virtual void setupUi(qSlicerPathExplorerMarkupsTableWidget*);

 public:
  vtkMRMLMarkupsFiducialNode* FiducialNode;
  int CurrentMarkupSelected;
  double RGBA[3];
};

//-----------------------------------------------------------------------------
qSlicerPathExplorerMarkupsTableWidgetPrivate
::qSlicerPathExplorerMarkupsTableWidgetPrivate(
  qSlicerPathExplorerMarkupsTableWidget& object)
  : q_ptr(&object)
{
  this->FiducialNode = NULL;
  this->CurrentMarkupSelected = -1;;
  this->RGBA[0] = 1.0;
  this->RGBA[1] = 0.5;
  this->RGBA[2] = 0.5;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidgetPrivate
::setupUi(qSlicerPathExplorerMarkupsTableWidget* widget)
{
  this->Ui_qSlicerPathExplorerMarkupsTableWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerMarkupsTableWidget
::qSlicerPathExplorerMarkupsTableWidget(QWidget *parentWidget)
  : Superclass (parentWidget)
    , d_ptr( new qSlicerPathExplorerMarkupsTableWidgetPrivate(*this) )
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);
  d->setupUi(this);

  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
	  this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));

  connect(d->AddButton, SIGNAL(toggled(bool)),
          this, SLOT(onAddButtonToggled(bool)));

  connect(d->RemoveButton, SIGNAL(clicked()),
          this, SLOT(onRemoveButtonClicked()));

  connect(d->ClearButton, SIGNAL(clicked()),
          this, SLOT(onClearButtonClicked()));

  connect(d->TableWidget, SIGNAL(itemSelectionChanged()),
          this, SLOT(onSelectionChanged()));

  connect(d->TableWidget, SIGNAL(cellChanged(int,int)),
          this, SLOT(onCellChanged(int,int)));
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerMarkupsTableWidget
::~qSlicerPathExplorerMarkupsTableWidget()
{
}

//-----------------------------------------------------------------------------
bool qSlicerPathExplorerMarkupsTableWidget
::getAddButtonState()
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  return d->AddButton->isChecked();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::setAddButtonState(bool state)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  bool oldState = d->AddButton->blockSignals(true);
  d->AddButton->setChecked(state ? Qt::Checked : Qt::Unchecked);
  d->AddButton->blockSignals(oldState);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::setAndObserveMarkupFiducialNode(vtkMRMLMarkupsFiducialNode* markupList)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (d->FiducialNode)
    {
    // Disconnect signals
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
    this->qvtkDisconnect(d->FiducialNode, vtkMRMLMarkupsNode::PointAddedEvent,
                         this, SLOT(refreshMarkupTable(vtkObject*, void*)));
    this->qvtkDisconnect(d->FiducialNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                         this, SLOT(onMarkupModified(vtkObject*, void*)));
    this->qvtkDisconnect(d->FiducialNode, vtkMRMLMarkupsNode::PointRemovedEvent,
                         this, SLOT(refreshMarkupTable()));
#else
    this->qvtkDisconnect(d->FiducialNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent,
                         this, SLOT(onMarkupModified(vtkObject*, void*)));
    this->qvtkDisconnect(d->FiducialNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                         this, SLOT(onMarkupModified(vtkObject*, void*)));
    this->qvtkDisconnect(d->FiducialNode, vtkMRMLMarkupsNode::MarkupRemovedEvent,
                         this, SLOT(refreshMarkupTable()));
#endif
    }

  // Update FiducialNode with new markup list
  d->FiducialNode = markupList;

  // Populate table with existing markups
  this->populateTableWidget(markupList);

  // Update display
  vtkMRMLMarkupsDisplayNode* displayNode =
    markupList->GetMarkupsDisplayNode();
  if (displayNode)
    {
    displayNode->SetGlyphType(vtkMRMLMarkupsDisplayNode::Sphere3D);
    displayNode->SetGlyphScale(4.0);
    displayNode->SetTextScale(4.0);
    }

  // Connect signals to new node
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
  this->qvtkConnect(d->FiducialNode, vtkMRMLMarkupsNode::PointAddedEvent,
                    this, SLOT(refreshMarkupTable(vtkObject*, void*)));
  this->qvtkConnect(d->FiducialNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                    this, SLOT(onMarkupModified(vtkObject*, void*)));
  this->qvtkConnect(d->FiducialNode, vtkMRMLMarkupsNode::PointRemovedEvent,
                    this, SLOT(refreshMarkupTable()));
#else
  this->qvtkConnect(d->FiducialNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent,
                    this, SLOT(onMarkupModified(vtkObject*, void*)));
  this->qvtkConnect(d->FiducialNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                    this, SLOT(onMarkupModified(vtkObject*, void*)));
  this->qvtkConnect(d->FiducialNode, vtkMRMLMarkupsNode::MarkupRemovedEvent,
                    this, SLOT(refreshMarkupTable()));
#endif
}


//-----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* qSlicerPathExplorerMarkupsTableWidget
::getMarkupFiducialNode()
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  return d->FiducialNode;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  if (!newScene)
    {
    return;
    }

  this->qvtkReconnect(this->mrmlScene(), 
		      newScene, vtkMRMLScene::StartCloseEvent,
		      this, SLOT(onMRMLSceneClosed()));
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onMRMLSceneClosed()
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->TableWidget)
    {
    return;
    }

  d->TableWidget->setRowCount(0);
  d->TableWidget->clearContents();

  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onAddButtonToggled(bool pushed)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);
  
  if (!this->mrmlScene())
    {
    return;
    }

  if (!d->FiducialNode)
    {
    d->AddButton->setChecked(false);
    return;
    }

  vtkMRMLSelectionNode* selectionNode = 
    vtkMRMLSelectionNode::SafeDownCast(this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  vtkMRMLInteractionNode* interactionNode =
    vtkMRMLInteractionNode::SafeDownCast(this->mrmlScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
  if (!selectionNode || !interactionNode)
    {
    return;
    }

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode");
  selectionNode->SetActivePlaceNodeID(d->FiducialNode->GetID());

  if (pushed)
    {
    interactionNode->SwitchToPersistentPlaceMode();
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
    }
  else
    {
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    }

  emit addButtonToggled(pushed);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onRemoveButtonClicked()
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->TableWidget || !d->FiducialNode)
    {
    return;
    }

  int row = d->TableWidget->currentRow();
  if (row < 0 || row >= d->TableWidget->rowCount())
    {
    return;
    }

  // Remove Markup
  int markupIndex = d->TableWidget->item(row, Self::Name)->data(Self::MarkupIndex).toInt();
  
  if (d->FiducialNode->MarkupExists(markupIndex))
    {    
    // Row in the widget is automatically removed when a markup is removed
    // See onMarkupRemoved
    d->FiducialNode->RemoveMarkup(markupIndex);
    emit markupRemoved(d->FiducialNode, markupIndex);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onClearButtonClicked()
{
   Q_D(qSlicerPathExplorerMarkupsTableWidget);

   if (!d->TableWidget || !d->FiducialNode)
     {
     return;
     }
   
   int nOfMarkups = d->FiducialNode->GetNumberOfMarkups();
   for (int i = 0; i < nOfMarkups; ++i)
     {
     emit markupRemoved(d->FiducialNode, i);
     }
   d->FiducialNode->RemoveAllMarkups();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onSelectionChanged()
{
   Q_D(qSlicerPathExplorerMarkupsTableWidget);

   if (!d->TableWidget || !d->FiducialNode)
     {
     return;
     }


   // Performance: Block signals to avoid updating all information of the selected markups 
   // when selecting it. SetNthFiducialSelected fired the modified event, but neither
   // the name or the position is changed, so table doesn't require to be updated
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
   this->qvtkBlock(d->FiducialNode, vtkMRMLMarkupsNode::PointModifiedEvent, this);
#else
  this->qvtkBlock(d->FiducialNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent, this); 
#endif

   if (d->CurrentMarkupSelected >= 0 && d->CurrentMarkupSelected < d->FiducialNode->GetNumberOfMarkups())
     {
     d->FiducialNode->SetNthFiducialSelected(d->CurrentMarkupSelected, false);
     }

   int row = d->TableWidget->currentRow();
   int markupIndex = -1;
   if (row >= 0)
     {
     markupIndex = d->TableWidget->item(row, Self::Name)->data(Self::MarkupIndex).toInt();
     
     if (d->FiducialNode->MarkupExists(markupIndex))
       {
       d->FiducialNode->SetNthFiducialSelected(markupIndex, true);
       d->CurrentMarkupSelected = markupIndex;
       }
     }

   emit markupSelected(d->FiducialNode, markupIndex);

#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
   this->qvtkUnblock(d->FiducialNode, vtkMRMLMarkupsNode::PointModifiedEvent, this);
#else
  this->qvtkUnblock(d->FiducialNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent, this);
#endif
   d->FiducialNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onCellChanged(int row, int column)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->TableWidget || !d->FiducialNode)
    {
    return;
    }

  if (!d->TableWidget->item(row, Self::Name) ||
      !d->TableWidget->item(row, Self::R) ||
      !d->TableWidget->item(row, Self::A) ||
      !d->TableWidget->item(row, Self::S) ||
      !d->TableWidget->item(row, Self::Time))
    {
    // Not all items in the row have been set
    return;
    }

  // Get updated markup
  int markupIndex = d->TableWidget->item(row, Self::Name)->data(Self::MarkupIndex).toInt();

  if (!d->FiducialNode->MarkupExists(markupIndex))
    {
    return;
    }
  
  if (column == Self::Name)
    {
    // Name has been edited
    d->FiducialNode->SetNthMarkupLabel(markupIndex, d->TableWidget->item(row, column)->text().toStdString());
    }
  else
    {
    // Position has been edited
    double markupPosition[3] = {d->TableWidget->item(row, Self::R)->text().toDouble(),
				d->TableWidget->item(row, Self::A)->text().toDouble(),
				d->TableWidget->item(row, Self::S)->text().toDouble()};
    d->FiducialNode->SetNthFiducialPositionFromArray(markupIndex, markupPosition);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::onMarkupModified(vtkObject* /*caller*/, void* callData)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!callData || !d->FiducialNode)
    {
    return;
    }

  // Get index of modified markup
  int *nPtr = NULL;
  int markupIndex = -1;
  nPtr = reinterpret_cast<int*>(callData);
  if (nPtr)
    {
    markupIndex = *nPtr;
    }

  // Update or add markup if doesn't exists in the widget
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
  if (d->FiducialNode->ControlPointExists(markupIndex))
    {
    // Check if node already in table
    QAbstractItemModel* model = d->TableWidget->model();
    if (model)
	    {
	    QModelIndexList found = model->match(model->index(0,Self::Name),
					          Self::MarkupIndex, markupIndex, 
					          1, Qt::MatchExactly);
	    if (found.isEmpty())
	      {
	      this->addMarkupInTable(d->FiducialNode, markupIndex);
	      }
	    else
	      {
	      this->updateMarkupInTable(d->FiducialNode, markupIndex, found[0]);
	      }
      }
    }
#else
    if (d->FiducialNode->MarkupExists(markupIndex))
    {
    Markup* modifiedMarkup = d->FiducialNode->GetNthMarkup(markupIndex);
    if (modifiedMarkup)
      {
      // Check if node already in table
      QAbstractItemModel* model = d->TableWidget->model();
      if (model)
	{
	QModelIndexList found = model->match(model->index(0,Self::Name),
					     Self::MarkupIndex, markupIndex, 
					     1, Qt::MatchExactly);
	if (found.isEmpty())
	  {
	  this->addMarkupInTable(d->FiducialNode, markupIndex);
	  }
	else
	  {
	  this->updateMarkupInTable(d->FiducialNode, markupIndex, found[0]);
	  }
	}
      }
    }
#endif

}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::refreshMarkupTable()
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->FiducialNode)
    {
    return;
    }

  // ISSUE: This slot is executed after markup is externally removed
  // It's not possible to get markup id or index before it gets removed
  // Require to clean and repopulate the table widget
  this->populateTableWidget(d->FiducialNode);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::addMarkupInTable(vtkMRMLMarkupsFiducialNode* fNode, int markupIndex)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->TableWidget || !fNode || 
      d->FiducialNode != fNode)
    {
    return;
    }

  if (!d->FiducialNode->MarkupExists(markupIndex))
    {
    // Markup not found in the markup list
    return;
    }

  if (d->FiducialNode->GetNumberOfPointsInNthMarkup(markupIndex) != 1)
    {
    // Not a fiducial
    return;
    }

  std::string markupName = d->FiducialNode->GetNthMarkupLabel(markupIndex);
  double markupPos[3];
  d->FiducialNode->GetNthFiducialPosition(markupIndex, markupPos);

  int rowCount = d->TableWidget->rowCount();
  d->TableWidget->insertRow(rowCount);
  
  QTableWidgetItem* nameItem = new QTableWidgetItem(QString(markupName.c_str()));
  QTableWidgetItem* rItem    = new QTableWidgetItem(QString::number(markupPos[0],'f',2));
  QTableWidgetItem* aItem    = new QTableWidgetItem(QString::number(markupPos[1],'f',2));
  QTableWidgetItem* sItem    = new QTableWidgetItem(QString::number(markupPos[2],'f',2));
  QTableWidgetItem* timeItem = new QTableWidgetItem(QTime::currentTime().toString());
  nameItem->setData(Self::MarkupIndex, markupIndex);
  timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
  
  QColor color = QColor(d->RGBA[0]*255, d->RGBA[1]*255, d->RGBA[2]*255, 128);
  QBrush brush = QBrush(color);
  nameItem->setBackground(brush);
  rItem->setBackground(brush);
  aItem->setBackground(brush);
  sItem->setBackground(brush);
  timeItem->setBackground(brush);

  d->TableWidget->setItem(rowCount, Self::Name, nameItem);
  d->TableWidget->setItem(rowCount, Self::R, rItem);
  d->TableWidget->setItem(rowCount, Self::A, aItem);
  d->TableWidget->setItem(rowCount, Self::S, sItem);
  d->TableWidget->setItem(rowCount, Self::Time, timeItem);

  // Select it
  d->TableWidget->selectRow(rowCount);
  d->FiducialNode->SetNthFiducialSelected(markupIndex, true);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::updateMarkupInTable(vtkMRMLMarkupsFiducialNode* fNode, int markupIndex, QModelIndex index)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);
  
  if (!d->TableWidget || !fNode || !d->FiducialNode || !index.isValid() ||
      fNode != d->FiducialNode)
    {
    return;
    }
  
  if (!d->FiducialNode->MarkupExists(markupIndex))
    {
    return;
    }

  // Blocking table signal is required here to avoid updating markup position
  // when setting R value (A and S not set yet)
  bool state = d->TableWidget->blockSignals(true);

  std::string markupName = d->FiducialNode->GetNthMarkupLabel(markupIndex);
  double ras[3];
  d->FiducialNode->GetNthFiducialPosition(markupIndex, ras);


  int row = index.row();
  d->TableWidget->item(row, Self::Name)->setText(QString(markupName.c_str()));
  d->TableWidget->item(row, Self::R)->setText(QString::number(ras[0],'f',2));
  d->TableWidget->item(row, Self::A)->setText(QString::number(ras[1],'f',2));
  d->TableWidget->item(row, Self::S)->setText(QString::number(ras[2],'f',2));

  d->TableWidget->blockSignals(state);

  emit markupModified(d->FiducialNode, markupIndex);
}

//-----------------------------------------------------------------------------
QString qSlicerPathExplorerMarkupsTableWidget
::selectedMarkupID()
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->TableWidget || !d->FiducialNode)
    {
    return QString();
    }

  int row = d->TableWidget->currentRow();
  int markupIndex = d->TableWidget->item(row, Self::Name)->data(Self::MarkupIndex).toInt();
  
  if (!d->FiducialNode->ControlPointExists(markupIndex))
    {
    return QString();
    }

  return d->FiducialNode->GetNthMarkupID(markupIndex).c_str();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::setSelectedMarkup(vtkMRMLMarkupsFiducialNode* fNode, int markupIndex)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);
  
  if (!d->TableWidget || !d->FiducialNode || !fNode ||
      fNode != d->FiducialNode)
    {
    return;
    }

  if (!d->FiducialNode->MarkupExists(markupIndex))
    {
    return;
    }

  QAbstractItemModel* model = d->TableWidget->model();
  if (model)
    {
    QModelIndexList found = model->match(model->index(0,Self::Name),
					 Self::MarkupIndex, markupIndex, 
					 1, Qt::MatchExactly);
    if (!found.isEmpty())
      {
      d->TableWidget->selectRow(found[0].row());
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::setColor(double r, double g, double b)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);
  
  if (!d->TableWidget || !d->FiducialNode)
    {
    return;
    }

  d->RGBA[0] = r;
  d->RGBA[1] = g;
  d->RGBA[2] = b;

  // Table colors
  std::stringstream tableWidgetBackgroundColor;
  tableWidgetBackgroundColor << "QTableWidget { "
			     << "selection-background-color: rgba("
			     << r*255 << "," << g*255 << "," << b*255 << "," << 200 << "); }";
  d->TableWidget->setStyleSheet(QString::fromStdString(tableWidgetBackgroundColor.str()));

  // Markups color
  vtkMRMLMarkupsDisplayNode* displayNode =
    d->FiducialNode->GetMarkupsDisplayNode();
  if (displayNode)
    {
    displayNode->SetColor(r,g,b);
    displayNode->SetSelectedColor(1.0, 0.5, 0.5);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerMarkupsTableWidget
::populateTableWidget(vtkMRMLMarkupsFiducialNode* markupList)
{
  Q_D(qSlicerPathExplorerMarkupsTableWidget);

  if (!d->TableWidget || !markupList)
    {
    return;
    }

  bool test = d->TableWidget->blockSignals(true);
  d->TableWidget->setRowCount(0);
  d->TableWidget->clearContents();
  d->TableWidget->blockSignals(test);

  int numberOfMarkups = markupList->GetNumberOfMarkups();
  if (numberOfMarkups > 0)
    {
    for (int i = 0; i < numberOfMarkups; ++i)
      {
      this->addMarkupInTable(markupList, i);
      }
    }
}
