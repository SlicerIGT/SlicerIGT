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

#include <QtGui>


int FIDUCIAL_LABEL_COLUMN = 0;
int FIDUCIAL_X_COLUMN = 1;
int FIDUCIAL_Y_COLUMN = 2;
int FIDUCIAL_Z_COLUMN = 3;


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
}


qSlicerSimpleMarkupsWidget
::~qSlicerSimpleMarkupsWidget()
{
}


qSlicerSimpleMarkupsWidget* qSlicerSimpleMarkupsWidget
::New( vtkSlicerMarkupsLogic* newMarkupsLogic )
{
  qSlicerSimpleMarkupsWidget* newSimpleMarkupsWidget = new qSlicerSimpleMarkupsWidget();
  newSimpleMarkupsWidget->MarkupsLogic = newMarkupsLogic;
  newSimpleMarkupsWidget->ModifiedStatus = 0;
  newSimpleMarkupsWidget->IsUpdatingTable = false;
  newSimpleMarkupsWidget->setup();
  return newSimpleMarkupsWidget;
}


void qSlicerSimpleMarkupsWidget
::setup()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  d->setupUi(this);
  this->setMRMLScene( this->MarkupsLogic->GetMRMLScene() );

  connect( d->MarkupsFiducialNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeChanged() ) );

  d->MarkupsFiducialTableWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->MarkupsFiducialTableWidget, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onMarkupsFiducialTableContextMenu(const QPoint&) ) );
  connect( d->MarkupsFiducialTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMarkupsFiducialEdited( int, int ) ) );


  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t, SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

  this->updateWidget();  
}


void qSlicerSimpleMarkupsWidget
::enter()
{
}


vtkMRMLNode* qSlicerSimpleMarkupsWidget
::GetCurrentNode()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  return d->MarkupsFiducialNodeComboBox->currentNode();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialNodeChanged()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  this->MarkupsLogic->SetActiveListID( vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() ) );
  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialTableContextMenu(const QPoint& position)
{
  Q_D(qSlicerSimpleMarkupsWidget);

  QPoint globalPosition = d->MarkupsFiducialTableWidget->viewport()->mapToGlobal( position );

  QMenu* fiducialsMenu = new QMenu( d->MarkupsFiducialTableWidget );
  QAction* activateAction = new QAction( "Make fiducial list active", fiducialsMenu );
  QAction* deleteAction = new QAction( "Delete current fiducial", fiducialsMenu );
  QAction* upAction = new QAction( "Move current fiducial up", fiducialsMenu );
  QAction* downAction = new QAction( "Move current fiducial down", fiducialsMenu );

  fiducialsMenu->addAction( activateAction );
  fiducialsMenu->addAction( deleteAction );
  fiducialsMenu->addAction( upAction );
  fiducialsMenu->addAction( downAction );

  QAction* selectedAction = fiducialsMenu->exec( globalPosition );

  int currentFiducial = d->MarkupsFiducialTableWidget->currentRow();
  vtkMRMLMarkupsFiducialNode* currentNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );
  
  if ( selectedAction == activateAction )
  {
    this->MarkupsLogic->SetActiveListID( vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() ) );
  }

  if ( selectedAction == deleteAction )
  {
    currentNode->RemoveMarkup( currentFiducial );
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

  

  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialEdited( int row, int column )
{
  Q_D(qSlicerSimpleMarkupsWidget);

  if ( this->IsUpdatingTable )
  {
    return;
  }

  vtkMRMLMarkupsFiducialNode* currentMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetCurrentNode() );

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

  if ( d->MarkupsFiducialNodeComboBox->currentNode() == NULL )
  {
    return;
  }

  vtkMRMLMarkupsFiducialNode* currentNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsFiducialNodeComboBox->currentNode() );

  // Only update if there is a modified event
  if ( this->ModifiedStatus == currentNode->GetMTime() )
  {
    return;
  }
  this->ModifiedStatus = currentNode->GetMTime();
  this->IsUpdatingTable = true;
 
  d->MarkupsFiducialTableWidget->clear();
  QStringList MarkupsTableHeaders;
  MarkupsTableHeaders << "Label" << "X" << "Y" << "Z";
  d->MarkupsFiducialTableWidget->setRowCount( currentNode->GetNumberOfFiducials() );
  d->MarkupsFiducialTableWidget->setColumnCount( 4 );
  d->MarkupsFiducialTableWidget->setHorizontalHeaderLabels( MarkupsTableHeaders );
  d->MarkupsFiducialTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  double fiducialPosition[ 3 ] = { 0, 0, 0 };
  std::string fiducialLabel = "";
  for ( int i = 0; i < currentNode->GetNumberOfFiducials(); i++ )
  {
    fiducialLabel = currentNode->GetNthFiducialLabel( i );
    currentNode->GetNthFiducialPosition( i, fiducialPosition );

    QTableWidgetItem* labelItem = new QTableWidgetItem( QString::fromStdString( fiducialLabel ) );
    QTableWidgetItem* xItem = new QTableWidgetItem( QString::number( fiducialPosition[0], 'f', 2 ) );
    QTableWidgetItem* yItem = new QTableWidgetItem( QString::number( fiducialPosition[1], 'f', 2 ) );
    QTableWidgetItem* zItem = new QTableWidgetItem( QString::number( fiducialPosition[2], 'f', 2 ) );

    d->MarkupsFiducialTableWidget->setItem( i, 0, labelItem );
    d->MarkupsFiducialTableWidget->setItem( i, 1, xItem );
    d->MarkupsFiducialTableWidget->setItem( i, 2, yItem );
    d->MarkupsFiducialTableWidget->setItem( i, 3, zItem );
  }

  this->IsUpdatingTable = false;
}
