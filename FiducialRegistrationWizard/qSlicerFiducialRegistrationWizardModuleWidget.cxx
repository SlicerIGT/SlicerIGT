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
#include <QtGui>

// SlicerQt includes
#include "qSlicerFiducialRegistrationWizardModuleWidget.h"
#include "ui_qSlicerFiducialRegistrationWizardModule.h"

#include "vtkSlicerFiducialRegistrationWizardLogic.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_FiducialRegistrationWizard
class qSlicerFiducialRegistrationWizardModuleWidgetPrivate: public Ui_qSlicerFiducialRegistrationWizardModule
{
  Q_DECLARE_PUBLIC( qSlicerFiducialRegistrationWizardModuleWidget ); 
  
protected:
  qSlicerFiducialRegistrationWizardModuleWidget* const q_ptr;
public:
  qSlicerFiducialRegistrationWizardModuleWidgetPrivate( qSlicerFiducialRegistrationWizardModuleWidget& object );
  vtkSlicerFiducialRegistrationWizardLogic* logic() const;

  // Add embedded widgets here
  qSlicerSimpleMarkupsWidget* FromMarkupsWidget;
  qSlicerSimpleMarkupsWidget* ToMarkupsWidget;
};



//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidgetPrivate methods


qSlicerFiducialRegistrationWizardModuleWidgetPrivate::qSlicerFiducialRegistrationWizardModuleWidgetPrivate( qSlicerFiducialRegistrationWizardModuleWidget& object ) : q_ptr( &object )
{
}


vtkSlicerFiducialRegistrationWizardLogic* qSlicerFiducialRegistrationWizardModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerFiducialRegistrationWizardModuleWidget );
  return vtkSlicerFiducialRegistrationWizardLogic::SafeDownCast( q->logic() );
}



//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidget methods



qSlicerFiducialRegistrationWizardModuleWidget
::qSlicerFiducialRegistrationWizardModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerFiducialRegistrationWizardModuleWidgetPrivate( *this ) )
{
}



qSlicerFiducialRegistrationWizardModuleWidget
::~qSlicerFiducialRegistrationWizardModuleWidget()
{
}


void qSlicerFiducialRegistrationWizardModuleWidget
::onRecordButtonClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  
  vtkMRMLLinearTransformNode* probeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->ProbeTransformComboBox->currentNode() );
  d->logic()->AddFiducial( probeTransformNode );
}


void qSlicerFiducialRegistrationWizardModuleWidget
::updateWidget()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  if ( this->FromModifiedStatus == d->FromMarkupsWidget->ModifiedStatus && this->ToModifiedStatus == d->FromMarkupsWidget->ModifiedStatus )
  {
    return;
  }
  this->FromModifiedStatus = d->FromMarkupsWidget->ModifiedStatus;
  this->ToModifiedStatus = d->ToMarkupsWidget->ModifiedStatus;
  
  vtkMRMLMarkupsFiducialNode* fromMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->GetCurrentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->GetCurrentNode() );
  vtkMRMLLinearTransformNode* outputTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->OutputTransformComboBox->currentNode() );

  std::string transformType = "";
  if ( d->SimilarityRadioButton->isChecked() )
  {
    transformType = "Similarity";
  }
  if ( d->RigidRadioButton->isChecked() )
  {
    transformType = "Rigid";
  }

  std::stringstream statusString;
  statusString << "Status: ";
  statusString << d->logic()->CalculateTransform( fromMarkupsFiducialNode, toMarkupsFiducialNode, outputTransformNode, transformType );
  d->StatusLabel->setText( QString::fromStdString( statusString.str() ) );
}



void qSlicerFiducialRegistrationWizardModuleWidget
::setup()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->setupUi(this);
  // Embed widgets here
  d->FromMarkupsWidget = qSlicerSimpleMarkupsWidget::New( d->logic()->MarkupsLogic );
  d->FromGroupBox->layout()->addWidget( d->FromMarkupsWidget );
  d->ToMarkupsWidget = qSlicerSimpleMarkupsWidget::New( d->logic()->MarkupsLogic );
  d->ToGroupBox->layout()->addWidget( d->ToMarkupsWidget );
  this->Superclass::setup();

  this->FromModifiedStatus = d->FromMarkupsWidget->ModifiedStatus;
  this->ToModifiedStatus = d->ToMarkupsWidget->ModifiedStatus;
  this->setMRMLScene( d->logic()->GetMRMLScene() );
   
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordButtonClicked() ) );

  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t, SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

  this->updateWidget();

}

