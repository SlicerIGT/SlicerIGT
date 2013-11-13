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
  connect( d->MarkupsFiducialTableWidget, SIGNAL( clicked() ), this, SLOT( MarkupsFiducialTableClicked() ) );

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

  // TODO: My Slicer is too old - doesn't have this function - test with newer Slicer
  //this->MarkupsLogic->SetActiveListID( d->MarkupsFiducialNodeComboBox->currentNode() );
  this->updateWidget();
}


void qSlicerSimpleMarkupsWidget
::onMarkupsFiducialTableClicked()
{
  Q_D(qSlicerSimpleMarkupsWidget);

  // TODO: My Slicer is too old - doesn't have this function - test with newer Slicer
  //this->MarkupsLogic->SetActiveListID( d->MarkupsFiducialNodeComboBox->currentNode() );
  this->updateWidget();
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

}
