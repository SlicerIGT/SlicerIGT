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
#include "qMRMLWatchdogToolBar.h"
#include "qSlicerToolBarManagerWidget.h"
#include "ui_qSlicerWatchdogModuleWidget.h"

#include "vtkMRMLDisplayableNode.h"
#include "vtkSlicerWatchdogLogic.h"
#include "vtkMRMLWatchdogNode.h"

int TOOL_LABEL_COLUMN = 0;
int TOOL_NAME_COLUMN = 1;
int TOOL_SOUND_COLUMN = 2;
int TOOL_TIMESTAMP_COLUMN = 3;
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
  qSlicerToolBarManagerWidget * ToolBarManager;
};

//-----------------------------------------------------------------------------
// qSlicerWatchdogModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidgetPrivate::qSlicerWatchdogModuleWidgetPrivate( qSlicerWatchdogModuleWidget& object )
: q_ptr( &object )
{
  this->ToolBarManager=NULL;
}

vtkSlicerWatchdogLogic* qSlicerWatchdogModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerWatchdogModuleWidget );
  return vtkSlicerWatchdogLogic::SafeDownCast( q->logic() );
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

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWatchdogNodeChanged() ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode* )), this, SLOT(onWatchdogNodeAddedByUser(vtkMRMLNode* ) ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode* )), this, SLOT(onModuleNodeAboutToBeRemoved(vtkMRMLNode* ) ) );

  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );

  connect( d->ToolBarVisibilityCheckBox, SIGNAL( clicked() ), this, SLOT( onToolBarVisibilityButtonClicked() ) );
  d->ToolBarVisibilityCheckBox->setChecked(true);

  connect( d->StatusRefreshRateSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( onStatusRefreshTimeSpinBoxChanged(int) ) );

  connect( d->AddToolButton, SIGNAL( clicked() ), this, SLOT( onToolNodeAdded() ) );
  connect( d->DeleteToolButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked()) );
  d->DeleteToolButton->setIcon( QIcon( ":/Icons/MarkupsDelete.png" ) );
  connect( d->UpToolButton, SIGNAL( clicked() ), this, SLOT( onUpButtonClicked()) );
  d->UpToolButton->setIcon( QIcon( ":/Icons/up.png" ) );
  connect( d->DownToolButton, SIGNAL( clicked() ), this, SLOT( onDownButtonClicked()) );
  d->DownToolButton->setIcon( QIcon( ":/Icons/down.png" ) );

  connect(d->ToolsTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT( onTableItemDoubleClicked() ));
  d->ToolsTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->ToolsTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onToolsTableContextMenu(const QPoint&) ) );

  connect( d->ToolBarManager->Timer, SIGNAL( timeout() ), this, SLOT( onTimeout() ) );

  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::SetToolBarManager(qSlicerToolBarManagerWidget * toolbarManager)
{
  Q_D(qSlicerWatchdogModuleWidget);
  if (toolbarManager!=NULL)
  {
    d->ToolBarManager=toolbarManager;
  }
  else
  {
    qCritical("Tool bar manager was not initialized");
  }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::enter()
{
  Q_D(qSlicerWatchdogModuleWidget);
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // Create a module MRML node if there is none in the scene.
  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLWatchdogNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLWatchdogNode > newNode = vtkSmartPointer< vtkMRMLWatchdogNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLWatchdogNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node" );
    return;
  }

  // For convenience, select a default module.
  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    d->ModuleNodeComboBox->setCurrentNodeID( node->GetID() );
  }

  foreach(qMRMLWatchdogToolBar * watchdogToolBar, *(d->ToolBarManager->GetToolBarHash()))
  {
    connect(watchdogToolBar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolBarVisibilityChanged(bool)) );
  }

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerWatchdogModuleWidget );
  this->Superclass::setMRMLScene( scene );
}

void qSlicerWatchdogModuleWidget::onSceneImportedEvent()
{
  this->enter();
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onStatusRefreshTimeSpinBoxChanged(int statusRefeshTimeMiliSec)
{
  Q_D( qSlicerWatchdogModuleWidget );

  d->ToolBarManager->Timer->stop();
  int statusRefeshTimeSec=((double)statusRefeshTimeMiliSec)/1000;
  d->ToolBarManager->Timer->start(statusRefeshTimeMiliSec);
  d->ToolBarManager->setStatusRefreshTimeSec(statusRefeshTimeSec);
  d->logic()->SetStatusRefreshTimeSec(statusRefeshTimeSec);
  updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onCurrentCellChanged(int currentRow, int currentColumn)
{
  if(this->CurrentCellPosition[0]!=currentRow && this->CurrentCellPosition[1]!=currentColumn)
  {
    return;
  }
  Q_D( qSlicerWatchdogModuleWidget );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  watchdogNode->GetToolNode(currentRow)->label=d->ToolsTableWidget->item(currentRow,currentColumn)->text().toStdString();
  qMRMLWatchdogToolBar *watchdogToolBar = d->ToolBarManager->GetToolBarHash()->value(watchdogNode->GetID());
  watchdogToolBar->SetNodeLabel(currentRow, watchdogNode->GetToolNode(currentRow)->label.c_str());
  disconnect( d->ToolsTableWidget, SIGNAL( cellChanged( int , int ) ), this, SLOT( onCurrentCellChanged( int, int ) ) );
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onTableItemDoubleClicked()
{
  Q_D( qSlicerWatchdogModuleWidget );
  if(d->ToolsTableWidget->currentColumn()!=0)
  {
    return;
  }
  this->CurrentCellPosition[0]=d->ToolsTableWidget->currentRow();
  this->CurrentCellPosition[1]=d->ToolsTableWidget->currentColumn();
  connect( d->ToolsTableWidget, SIGNAL( cellChanged( int , int ) ), this, SLOT( onCurrentCellChanged( int , int ) ) );
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onWatchdogNodeChanged()
{

  Q_D( qSlicerWatchdogModuleWidget );
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onWatchdogNodeAddedByUser(vtkMRMLNode* nodeAdded)
{
  Q_D( qSlicerWatchdogModuleWidget );
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default module.
  if(nodeAdded==NULL)
  {
    return;
  }
  vtkMRMLWatchdogNode* watchdogNodeAdded = vtkMRMLWatchdogNode::SafeDownCast( nodeAdded );
  if(watchdogNodeAdded==NULL)
  {
    return;
  }

  if(d->ToolBarManager->GetToolBarHash()==NULL)
  {
    return;
  }

  qMRMLWatchdogToolBar *watchdogToolBar=d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNodeAdded->GetID()));
  connect(watchdogToolBar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolBarVisibilityChanged(bool)) );

  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onModuleNodeAboutToBeRemoved(vtkMRMLNode* nodeToBeRemoved)
{
  Q_D( qSlicerWatchdogModuleWidget );
  if(nodeToBeRemoved==NULL && d->ToolBarManager->GetToolBarHash()==NULL)
  {
    return;
  }

  vtkMRMLWatchdogNode* watchdogNodeToBeRemoved = vtkMRMLWatchdogNode::SafeDownCast( nodeToBeRemoved );
  if(watchdogNodeToBeRemoved==NULL)
  {
    return;
  }

  qMRMLWatchdogToolBar *watchdogToolBar = d->ToolBarManager->GetToolBarHash()->value(watchdogNodeToBeRemoved->GetID());
  disconnect(watchdogToolBar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolBarVisibilityChanged(bool)) );

  this->updateFromMRMLNode();
  updateWidget();
  d->ToolsTableWidget->clear();
  d->ToolsTableWidget->setRowCount( 0 );
  d->ToolsTableWidget->setColumnCount( 0 );
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onToolBarVisibilityButtonClicked()
{
  Q_D( qSlicerWatchdogModuleWidget );

  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( watchdogNode == NULL )
  {
    qCritical( "Tool node should not be changed when no module node selected" );
    return;
  }

  if(d->ToolBarManager->GetToolBarHash()==NULL)
  {
    return;
  }
  qMRMLWatchdogToolBar *watchdogToolBar = d->ToolBarManager->GetToolBarHash()->value(watchdogNode->GetID());
  watchdogToolBar->toggleViewAction()->toggle();
  if(watchdogToolBar->isVisible())
  {
    watchdogToolBar->setVisible(false);
  }
  else
  {
    watchdogToolBar->setVisible(true);
  }
  //watchdogToolBar->visibilityChanged();
  updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::updateFromMRMLNode()
{
  Q_D( qSlicerWatchdogModuleWidget );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  d->ToolComboBox->setEnabled( true );
  updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onTimeout()
{
  updateTable();
}

//-----------------------------------------------------------------------------
void  qSlicerWatchdogModuleWidget::updateTable()
{
  Q_D( qSlicerWatchdogModuleWidget );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  std::list<WatchedTool> *toolsVectorPtr = watchdogNode->GetToolNodes();
  int numberTools= toolsVectorPtr->size();
  if ( toolsVectorPtr == NULL || numberTools != d->ToolsTableWidget->rowCount())
  {
    return;
  }
  int row=0;
  for (std::list<WatchedTool>::iterator it = toolsVectorPtr->begin() ; it != toolsVectorPtr->end(); ++it)
  {
    if((*it).tool==NULL)
    {
      return;
    }

    d->ToolsTableWidget->blockSignals( true );
    if((*it).status==0)
    {
      d->ToolsTableWidget->item( row, TOOL_TIMESTAMP_COLUMN)->setBackground(Qt::red);
      QString timeDisconnectedStringSec ("Disconnected ");
      timeDisconnectedStringSec += QString::number( floor(d->logic()->GetElapsedTimeSec()-(*it).lastElapsedTimeStamp) )+ " [s] ";
      d->ToolsTableWidget->item( row, TOOL_TIMESTAMP_COLUMN)->setText(timeDisconnectedStringSec);
    }
    else
    {
      d->ToolsTableWidget->item( row, TOOL_TIMESTAMP_COLUMN)->setText("");
      d->ToolsTableWidget->item( row, TOOL_TIMESTAMP_COLUMN)->setBackground(QBrush(QColor(45,224,90)));
    }
    d->ToolsTableWidget->blockSignals( false );
    row++;
  }
}

//-----------------------------------------------------------------------------
void  qSlicerWatchdogModuleWidget::onDownButtonClicked()
{
  Q_D( qSlicerWatchdogModuleWidget);
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    return;
  }
  int currentTool = d->ToolsTableWidget->currentRow();
  if ( currentTool < watchdogNode->GetNumberOfTools()- 1 && currentTool>=0 )
  {
    watchdogNode->SwapTools( currentTool, currentTool + 1 );
    d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool + 1);
  }
  updateWidget();
}

//-----------------------------------------------------------------------------
void  qSlicerWatchdogModuleWidget::onUpButtonClicked()
{
  Q_D( qSlicerWatchdogModuleWidget);
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    return;
  }
  int currentTool = d->ToolsTableWidget->currentRow();
  if ( currentTool > 0 )
  {
    watchdogNode->SwapTools( currentTool, currentTool - 1 );
    d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool - 1);
  }
  updateWidget();
}

//-----------------------------------------------------------------------------
void  qSlicerWatchdogModuleWidget::onDeleteButtonClicked()
{
  Q_D( qSlicerWatchdogModuleWidget);

  QItemSelectionModel* selectionModel = d->ToolsTableWidget->selectionModel();
  std::vector< int > deleteFiducials;
  // Need to find selected before removing because removing automatically refreshes the table
  for ( int i = 0; i < d->ToolsTableWidget->rowCount(); i++ )
  {
    if ( selectionModel->rowIntersectsSelection( i, d->ToolsTableWidget->rootIndex() ) )
    {
      deleteFiducials.push_back( i );
    }
  }
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    return;
  }
  for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
  {
    watchdogNode->RemoveTool(deleteFiducials.at( i ));
    d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->DeleteToolNode(deleteFiducials.at( i ));
  }
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onToolNodeAdded( )
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    return;
  }

  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  vtkMRMLDisplayableNode* currentToolNode = vtkMRMLDisplayableNode::SafeDownCast(d->ToolComboBox->currentNode());
  if ( currentToolNode  == NULL )
  {
    return;
  }

  d->logic()->AddToolNode(watchdogNode, currentToolNode ); // Make sure there is an associated display node
  const char * addedToolLabel = watchdogNode->GetToolNode(watchdogNode->GetNumberOfTools()-1)->label.c_str();
  d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->SetToolNodeAddedLabel(addedToolLabel);
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onToolsTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerWatchdogModuleWidget);

  QPoint globalPosition = d->ToolsTableWidget->viewport()->mapToGlobal( position );

  QMenu* trasnformsMenu = new QMenu( d->ToolsTableWidget );
  QAction* activateAction = new QAction( "Make list active", trasnformsMenu );
  QAction* deleteAction = new QAction( "Delete highlighted row", trasnformsMenu );
  QAction* upAction = new QAction( "Move current element up", trasnformsMenu );
  QAction* downAction = new QAction( "Move current row down", trasnformsMenu );

  trasnformsMenu->addAction( activateAction );
  trasnformsMenu->addAction( deleteAction );
  trasnformsMenu->addAction( upAction );
  trasnformsMenu->addAction( downAction );

  QAction* selectedAction = trasnformsMenu->exec( globalPosition );

  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }

  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );
  if ( watchdogNode == NULL )
  {
    return;
  }

  if ( selectedAction == deleteAction )
  {
    QItemSelectionModel* selectionModel = d->ToolsTableWidget->selectionModel();
    std::vector< int > deleteFiducials;
    // Need to find selected before removing because removing automatically refreshes the table
    for ( int i = 0; i < d->ToolsTableWidget->rowCount(); i++ )
    {
      if ( selectionModel->rowIntersectsSelection( i, d->ToolsTableWidget->rootIndex() ) )
      {
        deleteFiducials.push_back( i );
      }
    }
    //Traversing this way should be more efficient and correct
    QString toolKey;
    for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
    {
      watchdogNode->RemoveTool(deleteFiducials.at( i ));
      d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->DeleteToolNode(deleteFiducials.at( i ));
    }
  }
  int currentTool = d->ToolsTableWidget->currentRow();
  if ( selectedAction == upAction )
  {
    if ( currentTool > 0 )
    {
      watchdogNode->SwapTools( currentTool, currentTool - 1 );
      d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool - 1);
    }
  }

  if ( selectedAction == downAction )
  {
    if ( currentTool < watchdogNode->GetNumberOfTools()- 1 )
    {
      watchdogNode->SwapTools( currentTool, currentTool + 1 );
      d->ToolBarManager->GetToolBarHash()->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool + 1);
    }
  }
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::updateWidget()
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLNode* currentModuleNode = d->ModuleNodeComboBox->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    d->ToolsTableWidget->clear();
    d->ToolsTableWidget->setRowCount( 0 );
    d->ToolsTableWidget->setColumnCount( 0 );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentModuleNode );
  if ( watchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
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
    // Set the button indicating if this list is active
    if ( !watchdogNode->HasTool(currentToolNode->GetName()))
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

  for (int row=0; row<d->ToolsTableWidget->rowCount(); row++)
  {
    QCheckBox *soundCheckBox = dynamic_cast <QCheckBox *> (d->ToolsTableWidget->cellWidget(row,TOOL_SOUND_COLUMN));
    disconnect(soundCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSoundCheckBoxStateChanged(int)));
  }

  d->ToolsTableWidget->clear();
  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Label" << "Name" << "Sound" << "Status";
  d->ToolsTableWidget->setRowCount( watchdogNode->GetNumberOfTools() );
  d->ToolsTableWidget->setColumnCount( TOOL_COLUMNS );
  d->ToolsTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
  d->ToolsTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  std::string fiducialLabel = "";

  std::list<WatchedTool> *toolsVectorPtr = watchdogNode->GetToolNodes();
  int numberTools= toolsVectorPtr->size();
  if ( toolsVectorPtr == NULL || numberTools != d->ToolsTableWidget->rowCount())
  {
    return;
  }
  int row=0;
  for (std::list<WatchedTool>::iterator it = toolsVectorPtr->begin() ; it != toolsVectorPtr->end(); ++it)
  {
    if((*it).tool==NULL)
    {
      return;
    }
    QTableWidgetItem* nameItem = new QTableWidgetItem( (*it).tool->GetName() );
    QTableWidgetItem* labelItem = new QTableWidgetItem( (*it).label.c_str() );
    QTableWidgetItem* lastElapsedTimeStatus = new QTableWidgetItem( "" );
    lastElapsedTimeStatus->setTextAlignment(Qt::AlignCenter);

    QCheckBox *pCheckBox = new QCheckBox();
    pCheckBox->setAccessibleName(QString::number(row));
    if((*it).playSound)
    {
      pCheckBox->setCheckState(Qt::Checked);
    }
    else
    {
      pCheckBox->setCheckState(Qt::Unchecked);
    }
    pCheckBox->setStyleSheet("margin-left:50%; margin-right:50%;");
    connect(pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSoundCheckBoxStateChanged(int)));

    d->ToolsTableWidget->setItem( row, TOOL_TIMESTAMP_COLUMN, lastElapsedTimeStatus );
    d->ToolsTableWidget->setItem( row, TOOL_NAME_COLUMN, nameItem );
    d->ToolsTableWidget->setItem( row, TOOL_LABEL_COLUMN, labelItem );
    d->ToolsTableWidget->setCellWidget(row,TOOL_SOUND_COLUMN, pCheckBox);

    row++;
  }
  updateTable();
  if(d->ToolBarManager->GetToolBarHash()!=NULL)
  {
    qMRMLWatchdogToolBar *watchdogToolBar = d->ToolBarManager->GetToolBarHash()->value(watchdogNode->GetID());
    if(watchdogToolBar && watchdogToolBar->isVisible())
    {
      d->ToolBarVisibilityCheckBox->setChecked(true);
    }
    else
    {
      d->ToolBarVisibilityCheckBox->setChecked(false);
    }
  }

  d->ToolsTableWidget->blockSignals( false );
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onSoundCheckBoxStateChanged(int state)
{
  Q_D(qSlicerWatchdogModuleWidget);

  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentNode );

  if ( watchdogNode == NULL )
  {
    return;
  }
  QCheckBox *pCheckBox = dynamic_cast <QCheckBox *>(QObject::sender());
  int cbRow = pCheckBox->accessibleName().toInt();
  if(cbRow>=0&&cbRow<watchdogNode->GetNumberOfTools())
  {
    watchdogNode->GetToolNode(cbRow)->playSound=state;
  }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::onToolBarVisibilityChanged( bool visible )
{
  Q_D(qSlicerWatchdogModuleWidget);
  vtkMRMLNode* currentModuleNode = d->ModuleNodeComboBox->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( currentModuleNode );
  qMRMLWatchdogToolBar *watchdogToolBar = d->ToolBarManager->GetToolBarHash()->value(watchdogNode->GetID());
  if(watchdogToolBar && watchdogToolBar->isVisible())
  {
    d->ToolBarVisibilityCheckBox->setChecked(true);
  }
  else
  {
    d->ToolBarVisibilityCheckBox->setChecked(false);
  }
}

