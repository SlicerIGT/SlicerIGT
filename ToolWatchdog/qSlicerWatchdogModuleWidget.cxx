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

//#include "qSlicerModuleManager.h"
#include "qSlicerApplication.h"

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerWatchdogModuleWidget.h"
#include "qMRMLWatchdogToolBar.h"
#include "ui_qSlicerWatchdogModuleWidget.h"

#include "vtkMRMLDisplayableNode.h"
#include "vtkSlicerWatchdogLogic.h"
#include "QVTKSlicerWatchdogLogicInternal.h"
#include "vtkMRMLWatchdogNode.h"

#include <vtkCollection.h>
#include <vtkCollectionIterator.h>

int TOOL_LABEL_COLUMN = 0;
int TOOL_NAME_COLUMN = 1;
int TOOL_SOUND_COLUMN = 2;
int TOOL_TIMESTAMP_COLUMN = 3;
int TOOL_COLUMNS = 4;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerWatchdogModuleWidgetPrivate: public Ui_qSlicerWatchdogModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerWatchdogModuleWidget ); 

protected:
  qSlicerWatchdogModuleWidget* const q_ptr;
public:
  qSlicerWatchdogModuleWidgetPrivate( qSlicerWatchdogModuleWidget& object );
  vtkSlicerWatchdogLogic* logic() const;
  QHash<QString, qMRMLWatchdogToolBar *> * WatchdogToolbarHash;
};

//-----------------------------------------------------------------------------
// qSlicerWatchdogModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidgetPrivate::qSlicerWatchdogModuleWidgetPrivate( qSlicerWatchdogModuleWidget& object )
: q_ptr( &object )
{
  this->WatchdogToolbarHash=NULL;
  qDebug() << "Initialize watchdog widget private!";
}

vtkSlicerWatchdogLogic* qSlicerWatchdogModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerWatchdogModuleWidget );
  return vtkSlicerWatchdogLogic::SafeDownCast( q->logic() );
  qDebug() << "Logic";
}

//-----------------------------------------------------------------------------
// qSlicerWatchdogModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidget::qSlicerWatchdogModuleWidget(QWidget* _parent)
: Superclass( _parent )
, d_ptr( new qSlicerWatchdogModuleWidgetPrivate ( *this ) )
{
  qDebug() << "Initialize watchdog widget!";
  CurrentCellPosition[0]=0;
  CurrentCellPosition[1]=0;
}

//-----------------------------------------------------------------------------
qSlicerWatchdogModuleWidget::~qSlicerWatchdogModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModuleWidget::setup()
{
  Q_D(qSlicerWatchdogModuleWidget);
  qDebug() << "Setup watchdog widget!";

  d->setupUi(this);
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWatchdogNodeChanged() ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode* )), this, SLOT(onWatchdogNodeAddedByUser(vtkMRMLNode* ) ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode* )), this, SLOT(onModuleNodeAboutToBeRemoved(vtkMRMLNode* ) ) );

  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );

  connect( d->ToolbarVisibilityCheckBox, SIGNAL( clicked() ), this, SLOT( onToolbarVisibilityButtonClicked() ) );
  d->ToolbarVisibilityCheckBox->setChecked(true);

  connect( d->StatusRefreshRateSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( onStatusRefreshRateSpinBoxChanged(int) ) );

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

  connect( d->logic()->GetQVTKLogicInternal(), SIGNAL( updateTable() ), this, SLOT( onTimeout() ) );

  this->updateFromMRMLNode();
}


void qSlicerWatchdogModuleWidget::InitializeToolbar(vtkMRMLWatchdogNode* watchdogNodeAdded )
{
  Q_D(qSlicerWatchdogModuleWidget);
  qDebug() << "Initialize toolBAR";

  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLWatchdogToolBar *watchdogToolbar = new qMRMLWatchdogToolBar (window);
  watchdogToolbar->SetFirstlabel(watchdogNodeAdded->GetName());
  d->WatchdogToolbarHash->insert(QString(watchdogNodeAdded->GetID()), watchdogToolbar);
  window->addToolBar(watchdogToolbar);
  watchdogToolbar->setWindowTitle(QApplication::translate("qSlicerAppMainWindow", watchdogNodeAdded->GetName(), 0, QApplication::UnicodeUTF8));
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolbar->toggleViewAction());
      connect(watchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );
      break;
    }
  }

  //if(this->WatchdogToolbar==NULL)
  //{
  //  //vtkDebugMacro("Initilize toolBAR");
  //  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  //  this->WatchdogToolbar = new qMRMLWatchdogToolBar (window);
  //  window->addToolBar(this->WatchdogToolbar);
  //  this->WatchdogToolbar->setWindowTitle(QApplication::translate("qSlicerAppMainWindow",this->GetName(), 0, QApplication::UnicodeUTF8));
  //  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  //  {
  //    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
  //    {
  //      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
  //      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
  //      toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),this->WatchdogToolbar->toggleViewAction());
  //      break;
  //    }
  //  }
  //  this->WatchdogToolbar->SetFirstlabel(this->GetName());
  //}
}


void qSlicerWatchdogModuleWidget::RemoveToolbar()
{
  //vtkWarningMacro("DELETE WATCHDOG NODE : "<< this->GetName());




  //if(this->WatchdogToolbar!= NULL)
  //{
  //  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  //  window->removeToolBar(this->WatchdogToolbar);
  //  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  //  {
  //    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
  //    {
  //      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
  //      toolBarMenu->removeAction(this->WatchdogToolbar->toggleViewAction());
  //      break;
  //    }
  //  }
  //  this->WatchdogToolbar=NULL;
  //}
}





void
qSlicerWatchdogModuleWidget
::enter()
{
  Q_D(qSlicerWatchdogModuleWidget);
  qDebug() << "Enter watchdog widget!";

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // Create a module MRML node if there is none in the scene.
  //this->topLevelWidget()
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

  if(d->WatchdogToolbarHash==NULL)
  {
    d->WatchdogToolbarHash = new QHash<QString, qMRMLWatchdogToolBar *>;
  }

  vtkCollection* watchdogNodes = this->mrmlScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
  vtkCollectionIterator* watchdogNodeIt = vtkCollectionIterator::New();
  watchdogNodeIt->SetCollection( watchdogNodes );
  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
    if ( watchdogNode != NULL)
    {
      //watchdogNode->InitializeToolbar();
      if(!d->WatchdogToolbarHash->contains(watchdogNode->GetID()))
      {
        this->InitializeToolbar(watchdogNode);
       
        


      }
      qDebug( "Enter: connect toolbar with visibility changed ");
      //connect(watchdogNode->WatchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );
    }
  }
  watchdogNodeIt->Delete();
  watchdogNodes->Delete();

  this->Superclass::enter();
}

void
qSlicerWatchdogModuleWidget
::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerWatchdogModuleWidget );
  qDebug() << "Set mrml scene";
  this->Superclass::setMRMLScene( scene );
}

void
qSlicerWatchdogModuleWidget
::onSceneImportedEvent()
{
  qDebug() << "On mrml scene imported event";
  this->enter();
  this->updateWidget();
}

void qSlicerWatchdogModuleWidget
::onStatusRefreshRateSpinBoxChanged(int statusRefeshRateMiliSec)
{
  Q_D( qSlicerWatchdogModuleWidget );
  d->logic()->SetStatusRefreshTimeMiliSec(statusRefeshRateMiliSec);
  updateWidget();
}

void qSlicerWatchdogModuleWidget
::onCurrentCellChanged(int currentRow, int currentColumn)
{
  if(CurrentCellPosition[0]!=currentRow && CurrentCellPosition[1]!=currentColumn)
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
  //watchdogNode->WatchdogToolbar->SetNodeLabel(currentRow, watchdogNode->GetToolNode(currentRow)->label.c_str());
  d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SetNodeLabel(currentRow, watchdogNode->GetToolNode(currentRow)->label.c_str());
  disconnect( d->ToolsTableWidget, SIGNAL( cellChanged( int , int ) ), this, SLOT( onCurrentCellChanged( int, int ) ) );
  this->updateWidget();
}

void
qSlicerWatchdogModuleWidget
::onTableItemDoubleClicked()
{
  Q_D( qSlicerWatchdogModuleWidget );
  if(d->ToolsTableWidget->currentColumn()!=0)
  {
    return;
  }
  CurrentCellPosition[0]=d->ToolsTableWidget->currentRow();
  CurrentCellPosition[1]=d->ToolsTableWidget->currentColumn();
  connect( d->ToolsTableWidget, SIGNAL( cellChanged( int , int ) ), this, SLOT( onCurrentCellChanged( int , int ) ) );
}

void
qSlicerWatchdogModuleWidget
::onWatchdogNodeChanged()
{
  qDebug() << "On watchdog node changed";
  Q_D( qSlicerWatchdogModuleWidget );
  this->updateFromMRMLNode();
}

void qSlicerWatchdogModuleWidget
::onWatchdogNodeAddedByUser(vtkMRMLNode* nodeAdded)
{
  Q_D( qSlicerWatchdogModuleWidget );
  qDebug() << "On watchdog node added by user";
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

  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }

  //watchdogNodeAdded->InitializeToolbar();
  this->InitializeToolbar(watchdogNodeAdded);
  //connect(watchdogNodeAdded->WatchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );

  this->updateFromMRMLNode();
}

void qSlicerWatchdogModuleWidget
::onModuleNodeAboutToBeRemoved(vtkMRMLNode* nodeToBeRemoved)
{
  Q_D( qSlicerWatchdogModuleWidget );
  qDebug() << "On watchdog node to be removed";
  if(nodeToBeRemoved==NULL)
  {
    return;
  }

  vtkMRMLWatchdogNode* watchdogNodeToBeRemoved = vtkMRMLWatchdogNode::SafeDownCast( nodeToBeRemoved );
  if(watchdogNodeToBeRemoved==NULL)
  {
    return;
  }
  //disconnect(watchdogNodeToBeRemoved->WatchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );
  //watchdogNodeToBeRemoved->RemoveToolbar();

  /////////////////////////////////////////////////////////////////
  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }
  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLWatchdogToolBar *watchdogToolbar = d->WatchdogToolbarHash->value(watchdogNodeToBeRemoved->GetID());
  window->removeToolBar(watchdogToolbar);
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //watchdogToolbar->toggleViewAction()->name()
      //toolBarMenuActions.remove(watchdogToolbar->toggleViewAction());
      toolBarMenu->removeAction(watchdogToolbar->toggleViewAction());
      disconnect(watchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );
      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      //toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolbar->toggleViewAction());
      break;
    }
  }
  /////////////////////////////////////////////////////////////////


  this->updateFromMRMLNode();
  updateWidget();
  d->ToolsTableWidget->clear();
  d->ToolsTableWidget->setRowCount( 0 );
  d->ToolsTableWidget->setColumnCount( 0 );
}

void qSlicerWatchdogModuleWidget
::onToolbarVisibilityButtonClicked()
{
  Q_D( qSlicerWatchdogModuleWidget );

  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( watchdogNode == NULL )
  {
    qCritical( "Tool node should not be changed when no module node selected" );
    return;
  }

  //watchdogNode->WatchdogToolbar->toggleViewAction()->toggle();
  //if(watchdogNode->WatchdogToolbar->isVisible())
  //{
  //  watchdogNode->WatchdogToolbar->setVisible(false);
  //}
  //else
  //{
  //  watchdogNode->WatchdogToolbar->setVisible(true);
  //}


  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }
  qMRMLWatchdogToolBar *watchdogToolbar = d->WatchdogToolbarHash->value(watchdogNode->GetID());
  watchdogToolbar->toggleViewAction()->toggle();
  if(watchdogToolbar->isVisible())
  {
    watchdogToolbar->setVisible(false);
  }
  else
  {
    watchdogToolbar->setVisible(true);
  }
  //watchdogToolbar->visibilityChanged();
  updateWidget();
}

void
qSlicerWatchdogModuleWidget
::updateFromMRMLNode()
{
  Q_D( qSlicerWatchdogModuleWidget );
  qDebug() << "update From MRMLNode";
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

void qSlicerWatchdogModuleWidget
::onTimeout()
{
  updateToolbars();
  updateTable();
}

void  qSlicerWatchdogModuleWidget
::updateToolbars()
{
  Q_D( qSlicerWatchdogModuleWidget );
  //qDebug() << "update toolbars";
  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }

  QList<vtkMRMLNode*> watchdogNodesList = d->ModuleNodeComboBox->nodes();
  QList<vtkMRMLNode*>::Iterator it;
  for (it = watchdogNodesList.begin(); it != watchdogNodesList.end(); ++it)
  {

    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( (*it) );
    if(d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->isVisible())
    {
      //d->logic()->UpdateToolStatus( watchdogNode, (unsigned long) ElapsedTimeSec );
      std::list<WatchedTool>* toolsVectorPtr = watchdogNode->GetToolNodes();
      int numberTools = toolsVectorPtr->size();
      qDebug() << "update toolbars watchnode list number of tools " <<numberTools;
      if ( toolsVectorPtr == NULL /*|| numberTools!= d->ToolsTableWidget->rowCount()*/)
      {
        return;
      }
      int row=0;
      for (std::list<WatchedTool>::iterator itTool = toolsVectorPtr->begin() ; itTool != toolsVectorPtr->end(); ++itTool)
      {
        if((*itTool).tool==NULL)
        {
          return;
        }
        d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SetNodeStatus(row,(*itTool).status);
        row++;
      }
    }
  }
}


void  qSlicerWatchdogModuleWidget
::updateTable()
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

void  qSlicerWatchdogModuleWidget
::onDownButtonClicked()
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
    d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool + 1);
  }
  updateWidget();
}

void  qSlicerWatchdogModuleWidget
::onUpButtonClicked()
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
    d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool - 1);
  }
  updateWidget();
}

void  qSlicerWatchdogModuleWidget
::onDeleteButtonClicked()
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
  //Traversing this way should be more efficient and correct
  QString toolKey;
  for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
  {
    watchdogNode->RemoveTool(deleteFiducials.at( i ));
    d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->DeleteToolNode(deleteFiducials.at( i ));
  }
  this->updateWidget();
}

void qSlicerWatchdogModuleWidget
::onToolNodeAdded( )
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
  d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->ToolNodeAdded(currentToolNode->GetName());
  this->updateWidget();
}

void qSlicerWatchdogModuleWidget
::onToolsTableContextMenu(const QPoint& position)
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

  int currentTool = d->ToolsTableWidget->currentRow();
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
      d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->DeleteToolNode(deleteFiducials.at( i ));
    }
  }

  if ( selectedAction == upAction )
  {
    if ( currentTool > 0 )
    {
      watchdogNode->SwapTools( currentTool, currentTool - 1 );
      d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool - 1);
    }
  }

  if ( selectedAction == downAction )
  {
    if ( currentTool < watchdogNode->GetNumberOfTools()- 1 )
    {
      watchdogNode->SwapTools( currentTool, currentTool + 1 );
      d->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SwapToolNodes(currentTool, currentTool + 1);
    }
  }
  this->updateWidget();
}

void qSlicerWatchdogModuleWidget
::updateWidget()
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
  if(d->WatchdogToolbarHash!=NULL)
  {
    updateToolbars();
    qMRMLWatchdogToolBar *watchdogToolbar = d->WatchdogToolbarHash->value(watchdogNode->GetID());
    if(watchdogToolbar && watchdogToolbar->isVisible())
    {
      d->ToolbarVisibilityCheckBox->setChecked(true);
    }
    else
    {
      d->ToolbarVisibilityCheckBox->setChecked(false);
    }
  }

  d->ToolsTableWidget->blockSignals( false );


}



void qSlicerWatchdogModuleWidget
::onSoundCheckBoxStateChanged(int state)
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
    //QDebug("Changed state checkbox current cell position = %d", cbRow);
    watchdogNode->GetToolNode(cbRow)->playSound=state;
  }
}


void qSlicerWatchdogModuleWidget
::onToolbarVisibilityChanged( bool visible )
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
  qMRMLWatchdogToolBar *watchdogToolbar = d->WatchdogToolbarHash->value(watchdogNode->GetID());
  if(watchdogToolbar && watchdogToolbar->isVisible())
  {
    d->ToolbarVisibilityCheckBox->setChecked(true);
  }
  else
  {
    d->ToolbarVisibilityCheckBox->setChecked(false);
  }
}
