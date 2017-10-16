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

// Qt includes
#include <QDebug>
#include <QMenu>
#include <QtGui>

// SlicerQt includes
#include "qSlicerWatchdogModuleWidget.h"
#include "ui_qSlicerWatchdogModuleWidget.h"

#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLWatchdogNode.h"
#include "vtkMRMLWatchdogDisplayNode.h"
#include "vtkSlicerWatchdogLogic.h"

int TOOL_NAME_COLUMN = 0;
int TOOL_MESSAGE_COLUMN = 1;
int TOOL_SOUND_COLUMN = 2;
int TOOL_STATUS_COLUMN = 3;
int TOOL_COLUMNS = 4;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ToolWatchdog
class qSlicerWatchdogModuleWidgetPrivate: public Ui_qSlicerWatchdogModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerWatchdogModuleWidget ); 

protected:
  qSlicerWatchdogModuleWidget* const q_ptr;
public:
  qSlicerWatchdogModuleWidgetPrivate( qSlicerWatchdogModuleWidget& object );
  vtkSlicerWatchdogLogic* logic() const;
  vtkMRMLWatchdogDisplayNode* displayNode() const;

  vtkWeakPointer<vtkMRMLWatchdogNode> WatchdogNode;
};

//-----------------------------------------------------------------------------
// qSlicerWatchdogModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidgetPrivate::qSlicerWatchdogModuleWidgetPrivate( qSlicerWatchdogModuleWidget& object )
: q_ptr( &object )
{
}

//-----------------------------------------------------------------------------
vtkSlicerWatchdogLogic* qSlicerWatchdogModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerWatchdogModuleWidget );
  return vtkSlicerWatchdogLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
vtkMRMLWatchdogDisplayNode* qSlicerWatchdogModuleWidgetPrivate::displayNode() const
{
  Q_Q( const qSlicerWatchdogModuleWidget );
  if ( this->WatchdogNode == NULL )
  {
    return NULL;
  }
  vtkMRMLWatchdogDisplayNode* displayNode = vtkMRMLWatchdogDisplayNode::SafeDownCast(this->WatchdogNode->GetDisplayNode());
  return displayNode;
}

 

//-----------------------------------------------------------------------------
// qSlicerWatchdogModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidget::qSlicerWatchdogModuleWidget(QWidget* _parent)
: Superclass( _parent )
, d_ptr( new qSlicerWatchdogModuleWidgetPrivate ( *this ) )
{
  this->CurrentCellPosition[0]=0;
  this->CurrentCellPosition[1]=0;
}

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidget::~qSlicerWatchdogModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setup()
{
  Q_D(qSlicerWatchdogModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  // Parameter node

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWatchdogNodeSelectionChanged() ) );

  // Watched nodes table

  d->DeleteToolButton->setIcon( QIcon( ":/Icons/MarkupsDelete.png" ) );

  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Name" << "Message" << "Sound" << "Status";
  d->ToolsTableWidget->setColumnCount( TOOL_COLUMNS );
  d->ToolsTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  d->ToolsTableWidget->horizontalHeader()->setResizeMode( QHeaderView::ResizeToContents );
  d->ToolsTableWidget->horizontalHeader()->setResizeMode(TOOL_MESSAGE_COLUMN, QHeaderView::Stretch);
#else
  d->ToolsTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
  d->ToolsTableWidget->horizontalHeader()->setSectionResizeMode(TOOL_MESSAGE_COLUMN, QHeaderView::Stretch);
#endif

  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  connect( d->AddToolButton, SIGNAL( clicked() ), this, SLOT( onAddToolNode() ) );
  connect( d->DeleteToolButton, SIGNAL( clicked() ), this, SLOT( onRemoveToolNode()) );

  connect(d->ToolsTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT( onTableItemDoubleClicked() ));
  d->ToolsTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->ToolsTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onToolsTableContextMenu(const QPoint&) ) );

  // Display

  QObject::connect(d->VisibilityCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVisibility(bool)));
  QObject::connect(d->FontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setFontSize(int)));
  QObject::connect(d->OpacitySliderWidget, SIGNAL(valueChanged(double)), this, SLOT(setOpacity(double)));
  QObject::connect(d->BackgroundColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(setBackgroundColor(QColor)));
  QObject::connect(d->TextColorPickerButton, SIGNAL(colorChanged(QColor)), this, SLOT(setTextColor(QColor)));

  QObject::connect(d->DisplayNodeViewComboBox, SIGNAL(checkedNodesChanged()), this, SLOT(updateMRMLDisplayNodeViewsFromWidget()));
  QObject::connect(d->DisplayNodeViewComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)), this, SLOT(updateMRMLDisplayNodeViewsFromWidget()));
  QObject::connect(d->DisplayNodeViewComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)), this, SLOT(updateMRMLDisplayNodeViewsFromWidget()));

  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::enter()
{
  Q_D(qSlicerWatchdogModuleWidget);
  this->Superclass::enter();

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default module node.
  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    // Create a module node if there is none in the scene.
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLWatchdogNode");
    if ( node == NULL )
    {
      node = this->mrmlScene()->AddNewNodeByClass("vtkMRMLWatchdogNode");
    }
    d->ModuleNodeComboBox->setCurrentNode(node);
  }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerWatchdogModuleWidget );
  this->Superclass::setMRMLScene( scene );
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onSceneImportedEvent()
{
  this->enter();
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onCurrentCellChanged(int currentRow, int currentColumn)
{
  if(this->CurrentCellPosition[0]!=currentRow && this->CurrentCellPosition[1]!=currentColumn)
  {
    return;
  }
  Q_D( qSlicerWatchdogModuleWidget );
  if ( d->WatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  std::string message = d->ToolsTableWidget->item(currentRow,currentColumn)->text().toStdString();
  d->WatchdogNode->SetWatchedNodeWarningMessage(currentRow, message.c_str());
  disconnect( d->ToolsTableWidget, SIGNAL( cellChanged( int , int ) ), this, SLOT( onCurrentCellChanged( int, int ) ) );
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onTableItemDoubleClicked()
{
  Q_D( qSlicerWatchdogModuleWidget );
  if(d->ToolsTableWidget->currentColumn()!=TOOL_MESSAGE_COLUMN)
  {
    return;
  }
  this->CurrentCellPosition[0]=d->ToolsTableWidget->currentRow();
  this->CurrentCellPosition[1]=d->ToolsTableWidget->currentColumn();
  connect( d->ToolsTableWidget, SIGNAL( cellChanged( int , int ) ), this, SLOT( onCurrentCellChanged( int , int ) ) );
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onWatchdogNodeSelectionChanged()
{
  Q_D( qSlicerWatchdogModuleWidget );
  vtkMRMLWatchdogNode* selectedWatchdogNode = vtkMRMLWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  qvtkReconnect(d->WatchdogNode, selectedWatchdogNode, vtkCommand::ModifiedEvent, this, SLOT(onWatchdogNodeModified()));
  d->WatchdogNode = selectedWatchdogNode;
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::updateFromMRMLNode()
{
  Q_D( qSlicerWatchdogModuleWidget );
  if ( d->WatchdogNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  d->ToolComboBox->setEnabled( true );
  updateWidget();
}

//-----------------------------------------------------------------------------
void  qSlicerWatchdogModuleWidget::onRemoveToolNode()
{
  Q_D( qSlicerWatchdogModuleWidget);

  QItemSelectionModel* selectionModel = d->ToolsTableWidget->selectionModel();
  std::vector< int > deleteNodes;
  // Need to find selected before removing because removing automatically refreshes the table
  for ( int i = 0; i < d->ToolsTableWidget->rowCount(); i++ )
  {
    if ( selectionModel->rowIntersectsSelection( i, d->ToolsTableWidget->rootIndex() ) )
    {
      deleteNodes.push_back( i );
    }
  }
  if ( d->WatchdogNode == NULL )
  {
    return;
  }
  for ( int i = deleteNodes.size() - 1; i >= 0; i-- )
  {
    d->WatchdogNode->RemoveWatchedNode(deleteNodes.at( i ));
  }
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onAddToolNode( )
{
  Q_D(qSlicerWatchdogModuleWidget);
  if ( d->WatchdogNode == NULL )
  {
    return;
  }
  vtkMRMLNode* currentToolNode = d->ToolComboBox->currentNode();
  if ( currentToolNode  == NULL )
  {
    return;
  }
  d->WatchdogNode->AddWatchedNode(currentToolNode);
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onToolsTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerWatchdogModuleWidget);

  QPoint globalPosition = d->ToolsTableWidget->viewport()->mapToGlobal( position );

  QMenu* transformsMenu = new QMenu( d->ToolsTableWidget );
  QAction* activateAction = new QAction( "Make list active", transformsMenu );
  QAction* deleteAction = new QAction( "Delete highlighted row", transformsMenu );

  transformsMenu->addAction( activateAction );
  transformsMenu->addAction( deleteAction );

  QAction* selectedAction = transformsMenu->exec( globalPosition );

  if ( d->WatchdogNode == NULL )
  {
    return;
  }

  if ( selectedAction == deleteAction )
  {
    QItemSelectionModel* selectionModel = d->ToolsTableWidget->selectionModel();
    std::vector< int > deleteNodes;
    // Need to find selected before removing because removing automatically refreshes the table
    for ( int i = 0; i < d->ToolsTableWidget->rowCount(); i++ )
    {
      if ( selectionModel->rowIntersectsSelection( i, d->ToolsTableWidget->rootIndex() ) )
      {
        deleteNodes.push_back( i );
      }
    }
    //Traversing this way should be more efficient and correct
    QString toolKey;
    for ( int i = deleteNodes.size() - 1; i >= 0; i-- )
    {
      d->WatchdogNode->RemoveWatchedNode(deleteNodes.at( i ));
    }
    this->updateWidget();
  }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::updateWidget()
{
  Q_D(qSlicerWatchdogModuleWidget);
  if ( d->WatchdogNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    d->ToolsTableWidget->clearContents();
    d->ToolsTableWidget->setRowCount( 0 );
    return;
  }

  // Display section

  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (displayNode)
  {
    d->VisibilityCheckBox->setChecked(displayNode->GetVisibility());
    d->DisplayNodeViewComboBox->setMRMLDisplayNode(displayNode);
    d->OpacitySliderWidget->setValue(displayNode->GetOpacity());
    double color[3]={0};
    displayNode->GetColor(color);
    d->TextColorPickerButton->setColor(QColor(color[0]*255, color[1]*255, color[2]*255));
    displayNode->GetEdgeColor(color);
    d->BackgroundColorPickerButton->setColor(QColor(color[0]*255, color[1]*255, color[2]*255));
    d->FontSizeSpinBox->setValue(displayNode->GetFontSize());
  }

  vtkMRMLDisplayableNode* currentToolNode = vtkMRMLDisplayableNode::SafeDownCast( d->ToolComboBox->currentNode() );
  d->AddToolButton->blockSignals( true );
  if ( currentToolNode == NULL )
  {
    d->AddToolButton->setChecked( Qt::Unchecked );
    // This will ensure that we refresh the widget next time we move to a non-null widget (since there is guaranteed to be a modified status of larger than zero)
    //return;
    d->AddToolButton->setEnabled(false);
  }
  else
  {
    // Only allow adding the selected node if not watched already
    if (d->WatchdogNode->GetWatchedNodeIndex(currentToolNode)<0)
    {
      d->AddToolButton->setEnabled(true);
    }
    else
    {
      d->AddToolButton->setEnabled(false);
    }
  }
  d->AddToolButton->blockSignals( false );

  // Update the fiducials table
  d->ToolsTableWidget->blockSignals( true );

  int numberOfWatchedNodes = d->WatchdogNode->GetNumberOfWatchedNodes();
  if (numberOfWatchedNodes > d->ToolsTableWidget->rowCount())
  {
    // Add lines to the table
    int rowStartIndex = d->ToolsTableWidget->rowCount();
    d->ToolsTableWidget->setRowCount( numberOfWatchedNodes );
    for (int rowIndex = rowStartIndex; rowIndex < numberOfWatchedNodes; rowIndex++)
    {
      QTableWidgetItem* nameItem = new QTableWidgetItem();
      nameItem->setFlags(nameItem->flags() & (~Qt::ItemIsEditable));
      d->ToolsTableWidget->setItem( rowIndex, TOOL_NAME_COLUMN, nameItem );

      QTableWidgetItem* messageItem = new QTableWidgetItem();
      d->ToolsTableWidget->setItem( rowIndex, TOOL_MESSAGE_COLUMN, messageItem );

      // Need to put the icon into a label to allow alignment in the center
      QWidget *pStatusIconWidget = new QWidget();
      QLabel *label = new QLabel;
      label->setObjectName("StatusIcon");
      QHBoxLayout *pStatusLayout = new QHBoxLayout(pStatusIconWidget);
      pStatusLayout->addWidget(label);
      pStatusLayout->setAlignment(Qt::AlignCenter);
      pStatusLayout->setContentsMargins(0,0,0,0);
      pStatusIconWidget->setLayout(pStatusLayout);
      d->ToolsTableWidget->setCellWidget( rowIndex, TOOL_STATUS_COLUMN, pStatusIconWidget);

      QWidget *pSoundWidget = new QWidget();
      QCheckBox *pCheckBox = new QCheckBox();
      pCheckBox->setObjectName("Sound");
      QHBoxLayout *pSoundLayout = new QHBoxLayout(pSoundWidget);
      pSoundLayout->addWidget(pCheckBox);
      pSoundLayout->setAlignment(Qt::AlignCenter);
      pCheckBox->setStyleSheet("margin-left:2px; margin-right:2px;margin-top:2px; margin-bottom:2px;");
      pSoundWidget->setLayout(pSoundLayout);
      d->ToolsTableWidget->setCellWidget( rowIndex, TOOL_SOUND_COLUMN, pSoundWidget);
      
      connect(pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSoundCheckBoxStateChanged(int)));
    }
  }
  else if (numberOfWatchedNodes < d->ToolsTableWidget->rowCount())
  {
    // Remove lines from the table
    d->ToolsTableWidget->setRowCount( numberOfWatchedNodes );
  }

  for (int watchedNodeIndex = 0; watchedNodeIndex < numberOfWatchedNodes; watchedNodeIndex++ )
  {
    vtkMRMLNode* watchedNode = d->WatchdogNode->GetWatchedNode(watchedNodeIndex);
    if (watchedNode==NULL)
    {
      qWarning("Invalid watched node found, cannot update watched node table row");
      continue;
    }
    
    d->ToolsTableWidget->item(watchedNodeIndex, TOOL_NAME_COLUMN)->setText(watchedNode->GetName());
    d->ToolsTableWidget->item(watchedNodeIndex, TOOL_MESSAGE_COLUMN)->setText(d->WatchdogNode->GetWatchedNodeWarningMessage(watchedNodeIndex));

    QCheckBox* pCheckBox = d->ToolsTableWidget->cellWidget( watchedNodeIndex, TOOL_SOUND_COLUMN)->findChild<QCheckBox*>("Sound");
    if (pCheckBox)
    {
      pCheckBox->setCheckState(d->WatchdogNode->GetWatchedNodePlaySound(watchedNodeIndex) ? Qt::Checked : Qt::Unchecked);
      pCheckBox->setAccessibleName(QString::number(watchedNodeIndex));
    }

    QLabel* statusIcon = d->ToolsTableWidget->cellWidget( watchedNodeIndex, TOOL_STATUS_COLUMN)->findChild<QLabel*>("StatusIcon");
    if (statusIcon)
    {
      if(d->WatchdogNode->GetWatchedNodeUpToDate(watchedNodeIndex))
      {
        statusIcon->setPixmap(QPixmap(":/Icons/NodeValid.png"));
        statusIcon->setToolTip("valid");
      }
      else
      {
        statusIcon->setPixmap(QPixmap(":/Icons/NodeInvalid.png"));
        statusIcon->setToolTip("invalid");
      }
    }
  }
  d->ToolsTableWidget->resizeRowsToContents();

  d->ToolsTableWidget->blockSignals( false );
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onSoundCheckBoxStateChanged(int state)
{
  Q_D(qSlicerWatchdogModuleWidget);
  if ( d->WatchdogNode == NULL )
  {
    return;
  }
  QCheckBox *pCheckBox = dynamic_cast <QCheckBox *>(QObject::sender());
  int cbRow = pCheckBox->accessibleName().toInt();
  if(cbRow>=0&&cbRow<d->WatchdogNode->GetNumberOfWatchedNodes())
  {
    d->WatchdogNode->SetWatchedNodePlaySound(cbRow, state != Qt::Unchecked);
  }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onWatchdogNodeModified()
{
  updateWidget();
}

//------------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setVisibility(bool visible)
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (!displayNode)
    {
    qWarning("qSlicerWatchdogModuleWidget::setVisibility failed: no display node is available");
    return;
    }
  displayNode->SetVisibility(visible);
}

//------------------------------------------------------------------------------
bool qSlicerWatchdogModuleWidget::visibility()const
{
  Q_D(const qSlicerWatchdogModuleWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setFontSize(int fontSize)
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (!displayNode)
    {
    qWarning("qSlicerWatchdogModuleWidget::setFontSize failed: no display node is available");
    return;
    }
  displayNode->SetFontSize(fontSize);
}

//------------------------------------------------------------------------------
int qSlicerWatchdogModuleWidget::fontSize()const
{
  Q_D(const qSlicerWatchdogModuleWidget);
  return d->FontSizeSpinBox->value();
}

//------------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setOpacity(double opacity)
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (!displayNode)
    {
    qWarning("qSlicerWatchdogModuleWidget::setOpacity failed: no display node is available");
    return;
    }
  displayNode->SetOpacity(opacity);
}

//------------------------------------------------------------------------------
double qSlicerWatchdogModuleWidget::opacity()const
{
  Q_D(const qSlicerWatchdogModuleWidget);
  return d->OpacitySliderWidget->value();
}

//------------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setBackgroundColor(QColor color)
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (!displayNode)
    {
    qWarning("qSlicerWatchdogModuleWidget::setBackgroundColor failed: no display node is available");
    return;
    }
  displayNode->SetEdgeColor(color.redF(), color.greenF(), color.blueF());
}

//------------------------------------------------------------------------------
QColor qSlicerWatchdogModuleWidget::backgroundColor()const
{
  Q_D(const qSlicerWatchdogModuleWidget);
  return d->BackgroundColorPickerButton->color();
}

//------------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setTextColor(QColor color)
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (!displayNode)
    {
    qWarning("qSlicerWatchdogModuleWidget::setTextColor failed: no display node is available");
    return;
    }
  displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
}

//------------------------------------------------------------------------------
QColor qSlicerWatchdogModuleWidget::textColor()const
{
  Q_D(const qSlicerWatchdogModuleWidget);
  return d->TextColorPickerButton->color();
}

// --------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::updateMRMLDisplayNodeViewsFromWidget()
{
  Q_D(qSlicerWatchdogModuleWidget);

  vtkMRMLWatchdogDisplayNode* displayNode = d->displayNode();
  if (!displayNode)
    {
    qWarning("qSlicerWatchdogModuleWidget::updateMRMLDisplayNodeViewsFromWidget failed: no display node is available");
    return;
    }
  int wasModifying = displayNode->StartModify();

  if (d->DisplayNodeViewComboBox->allChecked() || d->DisplayNodeViewComboBox->noneChecked())
    {
    displayNode->RemoveAllViewNodeIDs();
    }
  else
    {
    foreach (vtkMRMLAbstractViewNode* viewNode, d->DisplayNodeViewComboBox->checkedViewNodes())
      {
      displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
      }
    foreach (vtkMRMLAbstractViewNode* viewNode, d->DisplayNodeViewComboBox->uncheckedViewNodes())
      {
      displayNode->RemoveViewNodeID(viewNode ? viewNode->GetID() : 0);
      }
    }

  displayNode->EndModify(wasModifying);
}
