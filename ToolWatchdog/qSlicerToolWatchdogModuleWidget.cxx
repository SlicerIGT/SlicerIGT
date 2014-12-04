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
#include <QTimer>
#include <QMenu>
#include <QtGui>
#include <QHash>

//#include "qSlicerAppMainWindow.h"
#include "qSlicerModuleManager.h"
//#include "qSlicerAbstractCoreModule.h"

#include "qSlicerApplication.h"


// SlicerQt includes
#include "qSlicerToolWatchdogModuleWidget.h"
#include "qMRMLToolWatchdogToolBar.h"
#include "ui_qSlicerToolWatchdogModuleWidget.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkSlicerToolWatchdogLogic.h"

#include "vtkMRMLToolWatchdogNode.h"

int TRANSFORM_LABEL_COLUMN = 0;
int TRANSFORM_TIMESTAMP_COLUMN = 1;
int TRANSFORMS_COLUMNS = 2;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerToolWatchdogModuleWidgetPrivate: public Ui_qSlicerToolWatchdogModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerToolWatchdogModuleWidget ); 

protected:
  qSlicerToolWatchdogModuleWidget* const q_ptr;
public:
  qSlicerToolWatchdogModuleWidgetPrivate( qSlicerToolWatchdogModuleWidget& object );
  vtkSlicerToolWatchdogLogic* logic() const;
  QHash<QString, qMRMLToolWatchdogToolBar *> * WatchdogToolbarHash;
};

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidgetPrivate::qSlicerToolWatchdogModuleWidgetPrivate( qSlicerToolWatchdogModuleWidget& object )
: q_ptr( &object )
{
  this->WatchdogToolbarHash=NULL;

}



vtkSlicerToolWatchdogLogic* qSlicerToolWatchdogModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerToolWatchdogModuleWidget );
  return vtkSlicerToolWatchdogLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidget::qSlicerToolWatchdogModuleWidget(QWidget* _parent)
: Superclass( _parent )
, d_ptr( new qSlicerToolWatchdogModuleWidgetPrivate ( *this ) )
{
  this->Timer = new QTimer( this );
  //Q_D(qSlicerToolWatchdogModuleWidget);


}

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidget::~qSlicerToolWatchdogModuleWidget()
{
    this->Timer->stop();
}

//-----------------------------------------------------------------------------
void qSlicerToolWatchdogModuleWidget::setup()
{
  Q_D(qSlicerToolWatchdogModuleWidget);
  
  d->setupUi(this);
  this->Superclass::setup();


  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeChanged() ) );

  connect( d->ModuleNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode* )), this, SLOT(onModuleNodeAddedByUser(vtkMRMLNode* ) ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode* )), this, SLOT(onModuleNodeAboutToBeRemoved(vtkMRMLNode* ) ) );

  connect( d->VisibilityButton, SIGNAL( clicked() ), this, SLOT( onVisibilityButtonClicked() ) );
  d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );


  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolChanged() ) );
  connect( d->AddToolButton, SIGNAL( clicked() ), this, SLOT( onToolNodeAdded() ) );
  //connect(d->AddToolButton, SIGNAL( clicked() ), WatchdogToolbar, SLOT( onTransformNodeAdded() ));
  connect( d->DeleteToolButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked()) );
  d->DeleteToolButton->setIcon( QIcon( ":/Icons/MarkupsDelete.png" ) );
  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );

  d->ToolsTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->ToolsTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onToolsTableContextMenu(const QPoint&) ) );
  connect( d->ToolsTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMarkupsFiducialEdited( int, int ) ) );
  this->UpdateFromMRMLNode();
}



void
qSlicerToolWatchdogModuleWidget
::enter()
{
  Q_D(qSlicerToolWatchdogModuleWidget);

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // Create a module MRML node if there is none in the scene.
//this->topLevelWidget()
  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLToolWatchdogNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLToolWatchdogNode > newNode = vtkSmartPointer< vtkMRMLToolWatchdogNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLToolWatchdogNode" );
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
    QMainWindow* window = qSlicerApplication::application()->mainWindow();
    qMRMLToolWatchdogToolBar *watchdogToolbar = new qMRMLToolWatchdogToolBar (window);
    watchdogToolbar->SetFirstlabel(node->GetName());
    d->WatchdogToolbarHash = new QHash<QString, qMRMLToolWatchdogToolBar *>;
    d->WatchdogToolbarHash->insert(QString(node->GetID()), watchdogToolbar);
    window->addToolBar(watchdogToolbar);
    watchdogToolbar->setWindowTitle(QApplication::translate("qSlicerAppMainWindow", node->GetName(), 0, QApplication::UnicodeUTF8));
    foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
    {
      if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
      {
        QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
        //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
        toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolbar->toggleViewAction());
        break;
      }
    }
  }




  this->Superclass::enter();
}



void
qSlicerToolWatchdogModuleWidget
::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  this->Superclass::setMRMLScene( scene );
}


//vtkMRMLNode* qSlicerToolWatchdogModuleWidget
//::GetCurrentNode()
//{
//  Q_D(qSlicerToolWatchdogModuleWidget);
//
//  return d->ToolComboBox->currentNode();
//}


void qSlicerToolWatchdogModuleWidget
::SetCurrentNode( vtkMRMLNode* currentNode )
{
  Q_D(qSlicerToolWatchdogModuleWidget);
  if(currentNode==NULL)
  {
    return;
  }

  vtkMRMLDisplayableNode* currentDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast( currentNode );

  // Don't change the active fiducial list if the current node is changed programmatically
  disconnect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolChanged() ) );
  d->ToolComboBox->setCurrentNode( currentDisplayableNode );
  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolChanged() ) );

  // Reconnect the appropriate nodes
  this->qvtkDisconnectAll();
  //this->ConnectInteractionAndSelectionNodes();
  this->qvtkConnect( currentDisplayableNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  //this->qvtkConnect( currentTransformNode, vtkMRMLTransformNode::MarkupAddedEvent, d->TransformsTableWidget, SLOT( scrollToBottom() ) );

  this->updateWidget(); // Must call this to update widget even if the node hasn't changed - this will cause the active button and table to update
}



void
qSlicerToolWatchdogModuleWidget
::onSceneImportedEvent()
{
  this->enter();
}



void
qSlicerToolWatchdogModuleWidget
::onModuleNodeChanged()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  this->UpdateFromMRMLNode();
}

void qSlicerToolWatchdogModuleWidget
::onModuleNodeAddedByUser(vtkMRMLNode* nodeAdded)
{
  Q_D( qSlicerToolWatchdogModuleWidget );
  //if ( this->mrmlScene() == NULL )
  //{
  //  qCritical() << "Invalid scene!";
  //  return;
  //}

  //// Create a module MRML node if there is none in the scene.
  ////this->topLevelWidget()
  //vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLToolWatchdogNode");
  //if ( node == NULL )
  //{
  //  vtkSmartPointer< vtkMRMLToolWatchdogNode > newNode = vtkSmartPointer< vtkMRMLToolWatchdogNode >::New();
  //  this->mrmlScene()->AddNode( newNode );
  //}

  //node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLToolWatchdogNode" );
  //if ( node == NULL )
  //{
  //  qCritical( "Failed to create module node" );
  //  return;
  //}

  // For convenience, select a default module.
  if(nodeAdded==NULL)
  {
    return;
  }
  vtkMRMLToolWatchdogNode* watchdogNodeAdded = vtkMRMLToolWatchdogNode::SafeDownCast( nodeAdded );
  if(watchdogNodeAdded==NULL)
  {
    return;
  }

  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }

  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLToolWatchdogToolBar *watchdogToolbar = new qMRMLToolWatchdogToolBar (window);
  watchdogToolbar->SetFirstlabel(nodeAdded->GetName());
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
      break;
    }
  }

  this->UpdateFromMRMLNode();
}


void qSlicerToolWatchdogModuleWidget
::onModuleNodeAboutToBeRemoved(vtkMRMLNode* nodeToBeRemoved)
{
  Q_D( qSlicerToolWatchdogModuleWidget );
  if(nodeToBeRemoved==NULL)
  {
    return;
  }
  vtkMRMLToolWatchdogNode* watchdogNodeAdded = vtkMRMLToolWatchdogNode::SafeDownCast( nodeToBeRemoved );
  if(watchdogNodeAdded==NULL)
  {
    return;
  }
  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }

  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLToolWatchdogToolBar *watchdogToolbar = d->WatchdogToolbarHash->value(nodeToBeRemoved->GetID());
  window->removeToolBar(watchdogToolbar);
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //watchdogToolbar->toggleViewAction()->name()
      //toolBarMenuActions.remove(watchdogToolbar->toggleViewAction());
      toolBarMenu->removeAction(watchdogToolbar->toggleViewAction());
      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      //toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolbar->toggleViewAction());
      break;
    }
  }

  this->UpdateFromMRMLNode();
  updateWidget();
  d->ToolsTableWidget->clear();
  d->ToolsTableWidget->setRowCount( 0 );
  d->ToolsTableWidget->setColumnCount( 0 );
}




void qSlicerToolWatchdogModuleWidget
::onVisibilityButtonClicked()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  vtkMRMLToolWatchdogNode* watchdogModuleNode = vtkMRMLToolWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( watchdogModuleNode == NULL )
  {
    qCritical( "Tool node should not be changed when no module node selected" );
    return;
  }


  if(d->WatchdogToolbarHash==NULL)
  {
    return;
  }

  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLToolWatchdogToolBar *watchdogToolbar = d->WatchdogToolbarHash->value(watchdogModuleNode->GetID());
  watchdogToolbar->toggleViewAction()->toggle();
  
  if(watchdogToolbar->isVisible())
  {
    d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerInvisible.png" ) );
    watchdogToolbar->setVisible(false);
  }
  else
  {
    d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );
    watchdogToolbar->setVisible(true);
  }
  //watchdogToolbar->visibilityChanged();

}



//-----------------------------------------------------------------------------
void qSlicerToolWatchdogModuleWidget::onToolChanged()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  vtkMRMLToolWatchdogNode* moduleNode = vtkMRMLToolWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( moduleNode == NULL )
  {
    qCritical( "Tool node should not be changed when no module node selected" );
    return;
  }

  vtkMRMLNode* currentNode = d->ToolComboBox->currentNode();
  if ( currentNode == NULL )
  {
    //d->logic()->SetObservedToolNode( NULL, moduleNode );
  }
  else
  {
    //d->logic()->SetObservedToolNode( vtkMRMLDisplayableNode::SafeDownCast( currentNode ), moduleNode );
    this->Timer->start( 1000 );
    updateWidget();
  }
}


void
qSlicerToolWatchdogModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }

  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );
  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  d->ToolComboBox->setEnabled( true );

  //if ( toolWatchdogNode->GetToolNode() != NULL )
  //{
  //  d->ToolComboBox->setCurrentNodeID( QString::fromStdString( toolWatchdogNode->GetToolNode()->GetID() ) );
  //}
  //else
  //{
  //  d->ToolComboBox->setCurrentNodeID( "" );
  //}

  updateWidget();
}


void qSlicerToolWatchdogModuleWidget
::OnTimeout()
{
  updateTable();
}


void  qSlicerToolWatchdogModuleWidget
::updateTable()
{
  Q_D( qSlicerToolWatchdogModuleWidget );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );

  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  d->logic()->UpdateToolState( toolWatchdogNode );

  std::list<WatchedTool> *toolsVectorPtr = toolWatchdogNode->GetToolNodes();
  int numberTools= toolsVectorPtr->size();
  if ( toolsVectorPtr == NULL || numberTools!= d->ToolsTableWidget->rowCount())
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
    QTableWidgetItem* status = new QTableWidgetItem( QString::number( (*it).LastTimeStamp ) );
    d->ToolsTableWidget->setItem( row, 1, status );

    QTableWidgetItem* labelItem = new QTableWidgetItem( (*it).tool->GetName() );
    d->WatchdogToolbarHash->value(QString(toolWatchdogNode->GetID()))->SetNodeStatus(row,(*it).status);
    d->ToolsTableWidget->setItem( row, 0, labelItem );
    if((*it).status==0)
    {
       d->ToolsTableWidget->item( row, 1)->setBackground(Qt::red);
    }
    else
    {
       d->ToolsTableWidget->item( row, 1)->setBackground(QBrush(QColor(45,224,90)));
    }
    d->ToolsTableWidget->blockSignals( false );
    row++;
  }
}

void  qSlicerToolWatchdogModuleWidget
::onDeleteButtonClicked()
{
  Q_D( qSlicerToolWatchdogModuleWidget);

  //vtkMRMLTransformNode* currentTransformNode = vtkMRMLMarkupsNode::SafeDownCast( d->ToolComboBox->currentNode() );
  //if ( currentTransformNode  == NULL )
  //{
  //  return;
  //}
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
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );
  if ( toolWatchdogNode == NULL )
  {
    return;
  }
  //Traversing this way should be more efficient and correct
  QString toolKey;
  for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
  {
    toolWatchdogNode->RemoveTool(deleteFiducials.at( i ));
    d->WatchdogToolbarHash->value(QString(toolWatchdogNode->GetID()))->ToolNodeDeleted();
  }
  this->updateWidget();
 
}

void qSlicerToolWatchdogModuleWidget
::onToolNodeAdded( )
{
  Q_D(qSlicerToolWatchdogModuleWidget);
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    return;
  }

  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );
  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  vtkMRMLDisplayableNode* currentToolNode = vtkMRMLDisplayableNode::SafeDownCast(d->ToolComboBox->currentNode());
  if ( currentToolNode  == NULL )
  {
    return;
  }

  d->logic()->AddToolNode(toolWatchdogNode, currentToolNode ); // Make sure there is an associated display node
  this->updateWidget();
  d->WatchdogToolbarHash->value(QString(toolWatchdogNode->GetID()))->ToolNodeAdded(currentToolNode->GetName());
  //this->onMarkupsFiducialNodeChanged();
}

void qSlicerToolWatchdogModuleWidget
::onToolsTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerToolWatchdogModuleWidget);

  QPoint globalPosition = d->ToolsTableWidget->viewport()->mapToGlobal( position );

  QMenu* trasnformsMenu = new QMenu( d->ToolsTableWidget );
  QAction* activateAction = new QAction( "Make transform list active", trasnformsMenu );
  QAction* deleteAction = new QAction( "Delete highlighted transforms", trasnformsMenu );
  QAction* upAction = new QAction( "Move current transform up", trasnformsMenu );
  QAction* downAction = new QAction( "Move current transform down", trasnformsMenu );
  QAction* jumpAction = new QAction( "Jump slices to transform", trasnformsMenu );

  trasnformsMenu->addAction( activateAction );
  trasnformsMenu->addAction( deleteAction );
  trasnformsMenu->addAction( upAction );
  trasnformsMenu->addAction( downAction );
  trasnformsMenu->addAction( jumpAction );

  QAction* selectedAction = trasnformsMenu->exec( globalPosition );

  int currentTrasform = d->ToolsTableWidget->currentRow();
  //vtkMRMLTransformNode* currentNode = vtkMRMLTransformNode::SafeDownCast( d->ToolComboBox->currentNode() );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );

  if ( toolWatchdogNode == NULL )
  {
    return;
  }

  //// Only do this for non-null node
  //if ( selectedAction == activateAction )
  //{
  //  this->MarkupsLogic->SetActiveListID( toolWatchdogNode ); // If there are other widgets, they are responsible for updating themselves
  //  emit markupsFiducialNodeChanged();
  //}

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
    QString transformKey;
    for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
    {
      toolWatchdogNode->RemoveTool(deleteFiducials.at( i ));
      d->WatchdogToolbarHash->value(QString(toolWatchdogNode->GetID()))->DeleteToolNode(deleteFiducials.at( i ));
    }
  }

  if ( selectedAction == upAction )
  {
    if ( currentTrasform > 0 )
    {
      toolWatchdogNode->SwapMarkups( currentTrasform, currentTrasform - 1 );
      d->WatchdogToolbarHash->value(QString(toolWatchdogNode->GetID()))->SwapToolNodes(currentTrasform, currentTrasform - 1);
    }
  }

  if ( selectedAction == downAction )
  {
    if ( currentTrasform < toolWatchdogNode->GetNumberOfTools()- 1 )
    {
      toolWatchdogNode->SwapMarkups( currentTrasform, currentTrasform + 1 );
      d->WatchdogToolbarHash->value(QString(toolWatchdogNode->GetID()))->SwapToolNodes(currentTrasform, currentTrasform + 1);
    }
  }

  //if ( selectedAction == jumpAction )
  //{
  //  //this->MarkupsLogic->JumpSlicesToNthPointInMarkup( this->GetCurrentNode()->GetID(), currentTrasform );
  //}

  this->updateWidget();
}


void qSlicerToolWatchdogModuleWidget
::updateWidget()
{
  Q_D(qSlicerToolWatchdogModuleWidget);
  vtkMRMLNode* currentModuleNode = d->ModuleNodeComboBox->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentModuleNode );
  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  vtkMRMLDisplayableNode* currentToolNode = vtkMRMLDisplayableNode::SafeDownCast( d->ToolComboBox->currentNode() );
  if ( currentToolNode == NULL )
  {
    d->ToolsTableWidget->clear();
    d->ToolsTableWidget->setRowCount( 0 );
    d->ToolsTableWidget->setColumnCount( 0 );
    d->AddToolButton->setChecked( Qt::Unchecked );
    // This will ensure that we refresh the widget next time we move to a non-null widget (since there is guaranteed to be a modified status of larger than zero)
    return;
  }

  // Set the button indicating if this list is active
  d->AddToolButton->blockSignals( true );
  if ( toolWatchdogNode->HasTool(currentToolNode->GetName()))
  {
    d->AddToolButton->setEnabled(false);
  }
  else
  {
    d->AddToolButton->setEnabled(true);
  }
  d->AddToolButton->blockSignals( false );

  // Update the fiducials table
  d->ToolsTableWidget->blockSignals( true );
  d->ToolsTableWidget->clear();
  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Label" << "Status";
  d->ToolsTableWidget->setRowCount( toolWatchdogNode->GetNumberOfTools() );
  d->ToolsTableWidget->setColumnCount( TRANSFORMS_COLUMNS );
  d->ToolsTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
  d->ToolsTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  std::string fiducialLabel = "";
  updateTable();

  d->ToolsTableWidget->blockSignals( false );

  //emit updateFinished();
}


//void qSlicerToolWatchdogModuleWidget
//::onTransformsEdited( int row, int column )
//{
//  Q_D(qSlicerToolWatchdogModuleWidget);
//
//  vtkMRMLTransformNode* currentTransformNode = vtkMRMLTransformNode::SafeDownCast( this->GetCurrentNode() );
//
//  if ( currentTransformNode == NULL )
//  {
//    return;
//  }
//
//  // Find the fiducial's current properties
//  double currentFiducialPosition[3] = { 0, 0, 0 };
//  currentTransformNode->GetNthFiducialPosition( row, currentFiducialPosition );
//  std::string currentFiducialLabel = currentTransformNode->GetNthFiducialLabel( row );
//
//  // Find the entry that we changed
//  QTableWidgetItem* qItem = d->TransformsTableWidget->item( row, column );
//  QString qText = qItem->text();
//
//  if ( column == FIDUCIAL_LABEL_COLUMN )
//  {
//    currentTransformNode->SetNthFiducialLabel( row, qText.toStdString() );
//  }
//
//  //// Check if the value can be converted to double is already performed implicitly
//  //double newFiducialPosition = qText.toDouble();
//  //// Change the position values
//  //if ( column == FIDUCIAL_X_COLUMN )
//  //{
//  //  currentFiducialPosition[ 0 ] = newFiducialPosition;
//  //}
//  //if ( column == FIDUCIAL_Y_COLUMN )
//  //{
//  //  currentFiducialPosition[ 1 ] = newFiducialPosition;
//  //}
//  //if ( column == FIDUCIAL_Z_COLUMN )
//  //{
//  //  currentFiducialPosition[ 2 ] = newFiducialPosition;
//  //}
//  //currentTransformNode->SetNthFiducialPositionFromArray( row, currentFiducialPosition );
//
//  this->updateWidget(); // This may not be necessary the widget is updated whenever a fiducial is changed
//}





