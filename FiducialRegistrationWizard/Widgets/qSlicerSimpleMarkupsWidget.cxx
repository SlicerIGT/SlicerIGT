/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerSimpleMarkupsWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

#include <QtGui>


int FIDUCIAL_LABEL_COLUMN = 0;
int FIDUCIAL_X_COLUMN = 1;
int FIDUCIAL_Y_COLUMN = 2;
int FIDUCIAL_Z_COLUMN = 3;
int FIDUCIAL_COLUMNS = 4;


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerSimpleMarkupsWidgetPrivate
  : public Ui_qSlicerSimpleMarkupsWidget
{
  Q_DECLARE_PUBLIC(qSlicerSimpleMarkupsWidget);
protected:
  qSlicerSimpleMarkupsWidget* const q_ptr;

public:
  qSlicerSimpleMarkupsWidgetPrivate( qSlicerSimpleMarkupsWidget& object);
  ~qSlicerSimpleMarkupsWidgetPrivate();
  virtual void setupUi(qSlicerSimpleMarkupsWidget*);
};

// --------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPrivate
::qSlicerSimpleMarkupsWidgetPrivate( qSlicerSimpleMarkupsWidget& object) : q_ptr(&object)
{
}

qSlicerSimpleMarkupsWidgetPrivate
::~qSlicerSimpleMarkupsWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerSimpleMarkupsWidgetPrivate
::setupUi(qSlicerSimpleMarkupsWidget* widget)
{
  this->Ui_qSlicerSimpleMarkupsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerSimpleMarkupsWidget methods

//-----------------------------------------------------------------------------
qSlicerSimpleMarkupsWidget
::qSlicerSimpleMarkupsWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerSimpleMarkupsWidgetPrivate(*this) )
{
  qSlicerAbstractCoreModule* MarkupsModule = qSlicerApplication::application()->moduleManager()->module( "Markups" );
  if ( MarkupsModule != NULL )
  {
    this->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast( MarkupsModule->logic() );
  }
  else
  {
    this->MarkupsLogic = NULL;
  }
  this->setup();
}


qSlicerSimpleMarkupsWidget
::~qSlicerSimpleMarkupsWidget()
{
}


void qSlicerSimpleMarkupsWidget
::setup()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->setupUi(this);
  this->setMRMLScene( this->MarkupsLogic->GetMRMLScene() );

  this->DefaultNodeColor[ 0 ] = 0.0;
  this->DefaultNodeColor[ 1 ] = 0.0;
  this->DefaultNodeColor[ 2 ] = 0.0;

  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );
  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeAdded( vtkMRMLNode* ) ) );

  // Use the pressed signal (otherwise we can unpress buttons without clicking them)
  connect( d->ColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( onColorButtonChanged( QColor ) ) );

  connect( d->VisibilityButton, SIGNAL( clicked() ), this, SLOT( onVisibilityButtonClicked() ) );
  //d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );
  connect( d->LockButton, SIGNAL( clicked() ), this, SLOT( onLockButtonClicked() ) );
  //d->LockButton->setIcon( QIcon( ":/Icons/Small/SlicerUnlock.png" ) );
  connect( d->DeleteButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked() ) );
  //d->DeleteButton->setIcon( QIcon( ":/Icons/MarkupsDelete.png" ) );

  connect( d->ActiveButton, SIGNAL( toggled( bool ) ), this, SLOT( onActiveButtonClicked() ) );
  //d->ActiveButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_DialogApplyButton ) );

  connect( d->PlaceButton, SIGNAL( toggled( bool ) ), this, SLOT( onPlaceButtonClicked() ) ); 
  //d->PlaceButton->setIcon( QIcon( ":/Icons/MarkupsMouseModePlace.png" ) );

  d->ExtraButtonsFrame->setVisible( d->ExpandButton->isChecked() );

  d->MarkupsFiducialTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->MarkupsFiducialTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onMarkupsFiducialTableContextMenu(const QPoint&) ) );
  connect( d->MarkupsFiducialTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMarkupsFiducialEdited( int, int ) ) );

  // Connect to the selection singleton node - that way we can update the GUI if the Active node changes
  // Note that only the GUI cares about the active node (the logic and mrml don't)
  this->ConnectInteractionAndSelectionNodes();

  this->updateWidget();  
}


void qSlicerSimpleMarkupsWidget
::enter()
{
}


void qSlicerSimpleMarkupsWidget
::ConnectInteractionAndSelectionNodes()
{
  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( this->MarkupsLogic->GetSelectionNodeID() ) );
  if ( selectionNode != NULL )
  {
    this->qvtkConnect( selectionNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  }

  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );
  if ( interactionNode != NULL )
  {
    this->qvtkConnect( interactionNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  }
}



vtkMRMLNode* qSlicerSimpleMarkupsWidget
::GetCurrentNode()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  return d->MarkupsFiducialNodeComboBox->currentNode();
}


void qSlicerSimpleMarkupsWidget
::SetCurrentNode( vtkMRMLNode* currentNode )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( currentNode );

  // Don't change the active fiducial list if the current node is changed programmatically
  disconnect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );
  d->MarkupsFiducialNodeComboBox->setCurrentNode( currentMarkupsNode );
  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );

  // Reconnect the appropriate nodes
  this->qvtkDisconnectAll();
  this->ConnectInteractionAndSelectionNodes();
  this->qvtkConnect( currentMarkupsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->qvtkConnect( currentMarkupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent, d->MarkupsFiducialTableWidget, SLOT( scrollToBottom() ) );

  this->updateWidget(); // Must call this to update widget even if the node hasn't changed - this will cause the active button and table to update
}


void qSlicerSimpleMarkupsWidget
::SetNodeBaseName( const char* newNodeBaseName )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->MarkupsFiducialNodeComboBox->setBaseName( QString( newNodeBaseName ) );
}


void qSlicerSimpleMarkupsWidget
::SetDefaultNodeColor( double rgb[3] )
{
  this->DefaultNodeColor[ 0 ] = rgb[ 0 ];
  this->DefaultNodeColor[ 1 ] = rgb[ 1 ];
  this->DefaultNodeColor[ 2 ] = rgb[ 2 ];
}


void qSlicerSimpleMarkupsWidget
::SetNodeColor( double rgb[3] )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  if ( currentMarkupsNode == NULL )
  {
    return;
  }
  
  vtkMRMLDisplayNode* currentMarkupsDisplayNode = currentMarkupsNode->GetDisplayNode();
  if ( currentMarkupsDisplayNode == NULL )
  {
    return;
  }

  currentMarkupsDisplayNode->SetColor( rgb );
  currentMarkupsDisplayNode->SetSelectedColor( rgb );
}


void qSlicerSimpleMarkupsWidget
::GetNodeColor( double rgb[3] )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  if ( currentMarkupsNode == NULL )
  {
    return;
  }
  
  vtkMRMLDisplayNode* currentMarkupsDisplayNode = currentMarkupsNode->GetDisplayNode();
  if ( currentMarkupsDisplayNode == NULL )
  {
    return;
  }

  return currentMarkupsDisplayNode->GetSelectedColor( rgb );
}


void qSlicerSimpleMarkupsWidget
::highlightNthFiducial( int n )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  if ( n >= 0 && n < d->MarkupsFiducialTableWidget->rowCount() )
  {
    d->MarkupsFiducialTableWidget->selectRow( n );
  }
  else
  {
    d->MarkupsFiducialTableWidget->clearSelection();
  }
}

void qSlicerSimpleMarkupsWidget
::onColorButtonChanged( QColor qColor )
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  double color[ 3 ];
  qMRMLUtils::qColorToColor( qColor, color );

  if ( currentMarkupsNode != NULL && currentMarkupsNode->GetDisplayNode() != NULL )
  {
    currentMarkupsNode->GetDisplayNode()->SetSelectedColor( color );
  }

  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onVisibilityButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  if ( currentMarkupsNode != NULL && currentMarkupsNode->GetDisplayNode() != NULL )
  {
    currentMarkupsNode->GetDisplayNode()->SetVisibility( ! currentMarkupsNode->GetDisplayNode()->GetVisibility() );
  }

  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onLockButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  if ( currentMarkupsNode != NULL )
  {
    currentMarkupsNode->SetLocked( ! currentMarkupsNode->GetLocked() );
  }

  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onDeleteButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  if ( currentMarkupsNode  == NULL )
  {
    return;
  }

  QItemSelectionModel* selectionModel = d->MarkupsFiducialTableWidget->selectionModel();
  std::vector< int > deleteFiducials;
  // Need to find selected before removing because removing automatically refreshes the table
  for ( int i = 0; i < d->MarkupsFiducialTableWidget->rowCount(); i++ )
  {
    if ( selectionModel->rowIntersectsSelection( i, d->MarkupsFiducialTableWidget->rootIndex() ) )
    {
      deleteFiducials.push_back( i );
    }
  }
  //Traversing this way should be more efficient and correct
  for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
  {
    currentMarkupsNode->RemoveMarkup( deleteFiducials.at( i ) );
  }

  this->updateWidget();
}    


void qSlicerSimpleMarkupsWidget
::onPlaceButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  bool isActive = currentMarkupsNode != NULL && this->MarkupsLogic->GetActiveListID().compare( currentMarkupsNode->GetID() ) == 0;
  bool isPlace = interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place;

  if ( isPlace && isActive )
  {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
  }
  else
  {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::Place );
    // interactionNode->SetPlaceModePersistence( true ); // Use whatever persistence the user has already set
  }

  if ( currentMarkupsNode != NULL )
  {
    this->MarkupsLogic->SetActiveListID( currentMarkupsNode ); // If there are other widgets, they are responsible for updating themselves
  }

  this->updateWidget();

  emit markupsFiducialPlaceModeChanged();
}


void qSlicerSimpleMarkupsWidget
::onActiveButtonClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  bool isActive = currentMarkupsNode != NULL && this->MarkupsLogic->GetActiveListID().compare( currentMarkupsNode->GetID() ) == 0;

  if ( ! isActive )
  {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
  }

  if ( currentMarkupsNode != NULL )
  {
    this->MarkupsLogic->SetActiveListID( currentMarkupsNode ); // If there are other widgets, they are responsible for updating themselves
  }

  this->updateWidget();

  emit markupsFiducialActivated();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialNodeChanged()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Reconnect the appropriate nodes
  this->qvtkDisconnectAll();
  this->ConnectInteractionAndSelectionNodes();
  this->qvtkConnect( currentMarkupsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->qvtkConnect( currentMarkupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent, d->MarkupsFiducialTableWidget, SLOT( scrollToBottom() ) );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  if ( currentMarkupsNode != NULL )
  {
    this->MarkupsLogic->SetActiveListID( currentMarkupsNode ); // If there are other widgets, they are responsible for updating themselves
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::Place );
    // interactionNode->SetPlaceModePersistence( true ); // Use whatever persistence the user has already set
  }
  else
  {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
  }

  this->updateWidget();

  emit markupsFiducialNodeChanged();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialNodeAdded( vtkMRMLNode* newNode )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsFiducialNode* newMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( newNode );
  this->MarkupsLogic->AddNewDisplayNodeForMarkupsNode( newMarkupsFiducialNode ); // Make sure there is an associated display node
  d->MarkupsFiducialNodeComboBox->setCurrentNode( newMarkupsFiducialNode );
  this->SetNodeColor( this->DefaultNodeColor );
  this->onMarkupsFiducialNodeChanged();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  QPoint globalPosition = d->MarkupsFiducialTableWidget->viewport()->mapToGlobal( position );

  QMenu* fiducialsMenu = new QMenu( d->MarkupsFiducialTableWidget );
  QAction* activateAction = new QAction( "Make fiducial list active", fiducialsMenu );
  QAction* deleteAction = new QAction( "Delete highlighted fiducials", fiducialsMenu );
  QAction* upAction = new QAction( "Move current fiducial up", fiducialsMenu );
  QAction* downAction = new QAction( "Move current fiducial down", fiducialsMenu );
  QAction* jumpAction = new QAction( "Jump slices to fiducial", fiducialsMenu );

  fiducialsMenu->addAction( activateAction );
  fiducialsMenu->addAction( deleteAction );
  fiducialsMenu->addAction( upAction );
  fiducialsMenu->addAction( downAction );
  fiducialsMenu->addAction( jumpAction );

  QAction* selectedAction = fiducialsMenu->exec( globalPosition );

  int currentFiducial = d->MarkupsFiducialTableWidget->currentRow();
  vtkMRMLMarkupsFiducialNode* currentNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  
  if ( currentNode == NULL )
  {
    return;
  }

  // Only do this for non-null node
  if ( selectedAction == activateAction )
  {
    this->MarkupsLogic->SetActiveListID( currentNode ); // If there are other widgets, they are responsible for updating themselves
    emit markupsFiducialNodeChanged();
  }

  if ( selectedAction == deleteAction )
  {
    QItemSelectionModel* selectionModel = d->MarkupsFiducialTableWidget->selectionModel();
    std::vector< int > deleteFiducials;
    // Need to find selected before removing because removing automatically refreshes the table
    for ( int i = 0; i < d->MarkupsFiducialTableWidget->rowCount(); i++ )
    {
      if ( selectionModel->rowIntersectsSelection( i, d->MarkupsFiducialTableWidget->rootIndex() ) )
      {
        deleteFiducials.push_back( i );
      }
    }
    //Traversing this way should be more efficient and correct
    for ( int i = deleteFiducials.size() - 1; i >= 0; i-- )
    {
      currentNode->RemoveMarkup( deleteFiducials.at( i ) );
    }
  }

  
  if ( selectedAction == upAction )
  {
    if ( currentFiducial > 0 )
    {
      currentNode->SwapMarkups( currentFiducial, currentFiducial - 1 );
    }
  }

  if ( selectedAction == downAction )
  {
    if ( currentFiducial < currentNode->GetNumberOfFiducials() - 1 )
    {
      currentNode->SwapMarkups( currentFiducial, currentFiducial + 1 );
    }
  }

  if ( selectedAction == jumpAction )
  {
    this->MarkupsLogic->JumpSlicesToNthPointInMarkup( this->GetCurrentNode()->GetID(), currentFiducial );
  }

  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialEdited( int row, int column )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsFiducialNode* currentMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetCurrentNode() );

  if ( currentMarkupsFiducialNode == NULL )
  {
    return;
  }

  // Find the fiducial's current properties
  double currentFiducialPosition[3] = { 0, 0, 0 };
  currentMarkupsFiducialNode->GetNthFiducialPosition( row, currentFiducialPosition );
  std::string currentFiducialLabel = currentMarkupsFiducialNode->GetNthFiducialLabel( row );

  // Find the entry that we changed
  QTableWidgetItem* qItem = d->MarkupsFiducialTableWidget->item( row, column );
  QString qText = qItem->text();

  if ( column == FIDUCIAL_LABEL_COLUMN )
  {
    currentMarkupsFiducialNode->SetNthFiducialLabel( row, qText.toStdString() );
  }

  // Check if the value can be converted to double is already performed implicitly
  double newFiducialPosition = qText.toDouble();

  // Change the position values
  if ( column == FIDUCIAL_X_COLUMN )
  {
    currentFiducialPosition[ 0 ] = newFiducialPosition;
  }
  if ( column == FIDUCIAL_Y_COLUMN )
  {
    currentFiducialPosition[ 1 ] = newFiducialPosition;
  }
  if ( column == FIDUCIAL_Z_COLUMN )
  {
    currentFiducialPosition[ 2 ] = newFiducialPosition;
  }

  currentMarkupsFiducialNode->SetNthFiducialPositionFromArray( row, currentFiducialPosition );

  this->updateWidget(); // This may not be necessary the widget is updated whenever a fiducial is changed
}



void qSlicerSimpleMarkupsWidget
::updateWidget()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  vtkMRMLMarkupsFiducialNode* currentMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  if ( currentMarkupsFiducialNode == NULL )
  {
    d->MarkupsFiducialTableWidget->clear();
    d->MarkupsFiducialTableWidget->setRowCount( 0 );
    d->MarkupsFiducialTableWidget->setColumnCount( 0 );
    d->ActiveButton->setChecked( Qt::Unchecked );
    d->PlaceButton->setChecked( Qt::Unchecked );
    // This will ensure that we refresh the widget next time we move to a non-null widget (since there is guaranteed to be a modified status of larger than zero)
    return;
  }

  // Set the button indicating if this list is active
  d->ActiveButton->blockSignals( true );
  d->PlaceButton->blockSignals( true );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  if ( currentMarkupsFiducialNode->GetDisplayNode() != NULL  )
  {
    double* color;
    QColor qColor;

    color = currentMarkupsFiducialNode->GetDisplayNode()->GetSelectedColor();
    qMRMLUtils::colorToQColor( color, qColor );

    d->ColorButton->setColor( qColor );
  }

  if ( this->MarkupsLogic->GetActiveListID().compare( currentMarkupsFiducialNode->GetID() ) == 0 )
  {
    d->ActiveButton->setChecked( Qt::Checked );
  }
  else
  {
    d->ActiveButton->setChecked( Qt::Unchecked );
  }

  if ( interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place && this->MarkupsLogic->GetActiveListID().compare( currentMarkupsFiducialNode->GetID() ) == 0 )
  {
    d->PlaceButton->setChecked( true );
  }
  else
  {
    d->PlaceButton->setChecked( false );
  }

  if ( currentMarkupsFiducialNode->GetDisplayNode() != NULL && currentMarkupsFiducialNode->GetDisplayNode()->GetVisibility() )
  {
    //d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerVisible.png" ) );
  }
  else
  {
    //d->VisibilityButton->setIcon( QIcon( ":/Icons/Small/SlicerInvisible.png" ) );
  }

  if ( currentMarkupsFiducialNode->GetLocked() )
  {
    //d->LockButton->setIcon( QIcon( ":/Icons/Small/SlicerLock.png" ) );
  }
  else
  {
    //d->LockButton->setIcon( QIcon( ":/Icons/Small/SlicerUnlock.png" ) );
  }

  d->ActiveButton->blockSignals( false );
  d->PlaceButton->blockSignals( false );

  // Update the fiducials table
  d->MarkupsFiducialTableWidget->blockSignals( true );
 
  d->MarkupsFiducialTableWidget->clear();
  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Label" << "X" << "Y" << "Z";
  d->MarkupsFiducialTableWidget->setRowCount( currentMarkupsFiducialNode->GetNumberOfFiducials() );
  d->MarkupsFiducialTableWidget->setColumnCount( FIDUCIAL_COLUMNS );
  d->MarkupsFiducialTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
  d->MarkupsFiducialTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  double fiducialPosition[ 3 ] = { 0, 0, 0 };
  std::string fiducialLabel = "";
  for ( int i = 0; i < currentMarkupsFiducialNode->GetNumberOfFiducials(); i++ )
  {
    fiducialLabel = currentMarkupsFiducialNode->GetNthFiducialLabel( i );
    currentMarkupsFiducialNode->GetNthFiducialPosition( i, fiducialPosition );

    QTableWidgetItem* labelItem = new QTableWidgetItem( QString::fromStdString( fiducialLabel ) );
    QTableWidgetItem* xItem = new QTableWidgetItem( QString::number( fiducialPosition[0], 'f', 3 ) );
    QTableWidgetItem* yItem = new QTableWidgetItem( QString::number( fiducialPosition[1], 'f', 3 ) );
    QTableWidgetItem* zItem = new QTableWidgetItem( QString::number( fiducialPosition[2], 'f', 3 ) );

    d->MarkupsFiducialTableWidget->setItem( i, 0, labelItem );
    d->MarkupsFiducialTableWidget->setItem( i, 1, xItem );
    d->MarkupsFiducialTableWidget->setItem( i, 2, yItem );
    d->MarkupsFiducialTableWidget->setItem( i, 3, zItem );
  }

  d->MarkupsFiducialTableWidget->blockSignals( false );

  emit updateFinished();
}
