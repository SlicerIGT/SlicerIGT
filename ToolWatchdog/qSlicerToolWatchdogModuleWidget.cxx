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
//#include <QHash>
#include <QtGui>

//#include "qSlicerApplication.h"
//#include "qSlicerModuleManager.h"
//#include "qSlicerAbstractCoreModule.h"




// SlicerQt includes
#include "qSlicerToolWatchdogModuleWidget.h"
#include "ui_qSlicerToolWatchdogModuleWidget.h"

#include "vtkSlicerToolWatchdogLogic.h"

#include "vtkMRMLToolWatchdogNode.h"
#include "vtkMRMLLinearTransformNode.h"

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
};

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidgetPrivate::qSlicerToolWatchdogModuleWidgetPrivate( qSlicerToolWatchdogModuleWidget& object )
: q_ptr( &object )
{
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
  connect( d->TransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformChanged() ) );

  connect( d->AddTransformButton, SIGNAL( clicked() ), this, SLOT( onTransformNodeAdded() ) );
  connect( d->DeleteTransformButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked()) );

  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );

  d->TransformsTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->TransformsTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onTransformsTableContextMenu(const QPoint&) ) );
  connect( d->TransformsTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMarkupsFiducialEdited( int, int ) ) );
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
//  return d->TransformComboBox->currentNode();
//}


void qSlicerToolWatchdogModuleWidget
::SetCurrentNode( vtkMRMLNode* currentNode )
{
  Q_D(qSlicerToolWatchdogModuleWidget);

  vtkMRMLTransformNode* currentTransformNode = vtkMRMLTransformNode::SafeDownCast( currentNode );

  // Don't change the active fiducial list if the current node is changed programmatically
  disconnect( d->TransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformChanged() ) );
  d->TransformComboBox->setCurrentNode( currentTransformNode );
  connect( d->TransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformChanged() ) );

  // Reconnect the appropriate nodes
  this->qvtkDisconnectAll();
  //this->ConnectInteractionAndSelectionNodes();
  this->qvtkConnect( currentTransformNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
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

//-----------------------------------------------------------------------------
void qSlicerToolWatchdogModuleWidget::onTransformChanged()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  vtkMRMLToolWatchdogNode* moduleNode = vtkMRMLToolWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( moduleNode == NULL )
  {
    qCritical( "Transform node should not be changed when no module node selected" );
    return;
  }

  vtkMRMLNode* currentNode = d->TransformComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->logic()->SetObservedTransformNode( NULL, moduleNode );
  }
  else
  {
    d->logic()->SetObservedTransformNode( vtkMRMLLinearTransformNode::SafeDownCast( currentNode ), moduleNode );
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
    d->TransformComboBox->setCurrentNodeID( "" );
    d->TransformComboBox->setEnabled( false );
    return;
  }

  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );
  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  d->TransformComboBox->setEnabled( true );

  if ( toolWatchdogNode->GetTransformNode() != NULL )
  {
    d->TransformComboBox->setCurrentNodeID( QString::fromStdString( toolWatchdogNode->GetTransformNode()->GetID() ) );
  }
  else
  {
    d->TransformComboBox->setCurrentNodeID( "" );
  }

  updateTable();
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
    d->TransformComboBox->setCurrentNodeID( "" );
    d->TransformComboBox->setEnabled( false );
    return;
  }
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );

  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  d->logic()->UpdateToolState( toolWatchdogNode );

  std::list<WatchedTransform> *toolsVectorPtr = toolWatchdogNode->GetTransformNodes();

  if ( toolsVectorPtr == NULL )
  {
    return;
  }
   int row=0;
  for (std::list<WatchedTransform>::iterator it = toolsVectorPtr->begin() ; it != toolsVectorPtr->end(); ++it)
  {
    d->TransformsTableWidget->blockSignals( true );
    QTableWidgetItem* status = new QTableWidgetItem( QString::number( (*it).LastTimeStamp ) );
    d->TransformsTableWidget->setItem( row, 1, status );
    QTableWidgetItem* labelItem = new QTableWidgetItem( (*it).transform->GetName() );
    d->TransformsTableWidget->setItem( row, 0, labelItem );
    if((*it).status==0)
    {
       d->TransformsTableWidget->item( row, 1)->setBackground(Qt::red);
    }
    else
    {
       d->TransformsTableWidget->item( row, 1)->setBackground(Qt::blue);
    }
    d->TransformsTableWidget->blockSignals( false );
    row++;
  }
}

void  qSlicerToolWatchdogModuleWidget
::onDeleteButtonClicked()
{
  Q_D( qSlicerToolWatchdogModuleWidget);

  //vtkMRMLTransformNode* currentTransformNode = vtkMRMLMarkupsNode::SafeDownCast( d->TransformComboBox->currentNode() );
  //if ( currentTransformNode  == NULL )
  //{
  //  return;
  //}
  QItemSelectionModel* selectionModel = d->TransformsTableWidget->selectionModel();

  std::vector< int > deleteFiducials;
  // Need to find selected before removing because removing automatically refreshes the table
  for ( int i = 0; i < d->TransformsTableWidget->rowCount(); i++ )
  {
    if ( selectionModel->rowIntersectsSelection( i, d->TransformsTableWidget->rootIndex() ) )
    {
      deleteFiducials.push_back( i );
    }
  }

  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->TransformComboBox->setCurrentNodeID( "" );
    d->TransformComboBox->setEnabled( false );
    return;
  }
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );
  if ( toolWatchdogNode == NULL )
  {
    return;
  }
  //Traversing this way should be more efficient and correct
  QString transformKey;
  for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
  {
    toolWatchdogNode->RemoveTransform(deleteFiducials.at( i ));
  }
  this->updateWidget();
}

void qSlicerToolWatchdogModuleWidget
::onTransformNodeAdded( )
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

  vtkMRMLTransformNode* currentTransformNode = vtkMRMLTransformNode::SafeDownCast(d->TransformComboBox->currentNode());
  d->logic()->AddTransformNode(toolWatchdogNode, currentTransformNode ); // Make sure there is an associated display node
  this->updateWidget();


  //this->onMarkupsFiducialNodeChanged();
}

void qSlicerToolWatchdogModuleWidget
::onTransformsTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerToolWatchdogModuleWidget);

  QPoint globalPosition = d->TransformsTableWidget->viewport()->mapToGlobal( position );

  QMenu* trasnformsMenu = new QMenu( d->TransformsTableWidget );
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

  int currentTrasform = d->TransformsTableWidget->currentRow();
  //vtkMRMLTransformNode* currentNode = vtkMRMLTransformNode::SafeDownCast( d->TransformComboBox->currentNode() );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->TransformComboBox->setCurrentNodeID( "" );
    d->TransformComboBox->setEnabled( false );
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
    QItemSelectionModel* selectionModel = d->TransformsTableWidget->selectionModel();
    std::vector< int > deleteFiducials;
    // Need to find selected before removing because removing automatically refreshes the table
    for ( int i = 0; i < d->TransformsTableWidget->rowCount(); i++ )
    {
      if ( selectionModel->rowIntersectsSelection( i, d->TransformsTableWidget->rootIndex() ) )
      {
        deleteFiducials.push_back( i );
      }
    }
    //Traversing this way should be more efficient and correct
    QString transformKey;
    for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
    {
      toolWatchdogNode->RemoveTransform(deleteFiducials.at( i ));
    }
  }


  if ( selectedAction == upAction )
  {
    if ( currentTrasform > 0 )
    {
      toolWatchdogNode->SwapMarkups( currentTrasform, currentTrasform - 1 );
    }
  }

  if ( selectedAction == downAction )
  {
    if ( currentTrasform < toolWatchdogNode->GetNumberOfTransforms()- 1 )
    {
      toolWatchdogNode->SwapMarkups( currentTrasform, currentTrasform + 1 );
    }
  }

  if ( selectedAction == jumpAction )
  {
    //this->MarkupsLogic->JumpSlicesToNthPointInMarkup( this->GetCurrentNode()->GetID(), currentTrasform );
  }

  this->updateWidget();
}


void qSlicerToolWatchdogModuleWidget
::updateWidget()
{
  Q_D(qSlicerToolWatchdogModuleWidget);
  vtkMRMLNode* currentModuleNode = d->ModuleNodeComboBox->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->TransformComboBox->setCurrentNodeID( "" );
    d->TransformComboBox->setEnabled( false );
    return;
  }
  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentModuleNode );
  if ( toolWatchdogNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  vtkMRMLTransformNode* currentTransformNode = vtkMRMLTransformNode::SafeDownCast( d->TransformComboBox->currentNode() );
  if ( currentTransformNode == NULL )
  {
    d->TransformsTableWidget->clear();
    d->TransformsTableWidget->setRowCount( 0 );
    d->TransformsTableWidget->setColumnCount( 0 );
    d->AddTransformButton->setChecked( Qt::Unchecked );
    // This will ensure that we refresh the widget next time we move to a non-null widget (since there is guaranteed to be a modified status of larger than zero)
    return;
  }

  // Set the button indicating if this list is active
  d->AddTransformButton->blockSignals( true );
  if ( toolWatchdogNode->HasTransform(currentTransformNode->GetName()))
  {
    //d->AddTransformButton->setChecked( Qt::Unchecked );
    d->AddTransformButton->setEnabled(false);
  }
  else
  {
    //d->AddTransformButton->setChecked( Qt::Checked );
    d->AddTransformButton->setEnabled(true);
  }
  d->AddTransformButton->blockSignals( false );

  // Update the fiducials table
  d->TransformsTableWidget->blockSignals( true );

  d->TransformsTableWidget->clear();
  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Label" << "Status";
  d->TransformsTableWidget->setRowCount( toolWatchdogNode->GetNumberOfTransforms() );
  d->TransformsTableWidget->setColumnCount( TRANSFORMS_COLUMNS );
  d->TransformsTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
  d->TransformsTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

  std::string fiducialLabel = "";

  updateTable();

  d->TransformsTableWidget->blockSignals( false );

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





