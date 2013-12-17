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
  qSlicerTransformPreviewWidget* TransformPreviewWidget;
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
::setup()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->setupUi(this);
  // Embed widgets here
  d->FromMarkupsWidget = qSlicerSimpleMarkupsWidget::New( d->logic()->MarkupsLogic );
  d->FromMarkupsWidget->SetNodeBaseName( "From" );
  d->FromGroupBox->layout()->addWidget( d->FromMarkupsWidget );

  d->ToMarkupsWidget = qSlicerSimpleMarkupsWidget::New( d->logic()->MarkupsLogic );
  d->ToMarkupsWidget->SetNodeBaseName( "To" );
  d->ToGroupBox->layout()->addWidget( d->ToMarkupsWidget );

  d->TransformPreviewWidget = qSlicerTransformPreviewWidget::New( d->logic()->GetMRMLScene() );
  d->PreviewGroupBox->layout()->addWidget( d->TransformPreviewWidget );

  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  // Make connections to update the mrml from the widget
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );

  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( UpdateToMRMLNode() ) );

  // These connections will do work (after being updated from the node)
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordButtonClicked() ) );

  // Watch the logic - it is updated whenver the mrml node is updated
  this->qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( UpdateFromMRMLNode() ) );

  this->UpdateFromMRMLNode();
}



void qSlicerFiducialRegistrationWizardModuleWidget
::UpdateToMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->logic()->GetFiducialRegistrationWizardNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }

  this->qvtkBlockAll( true );

  if ( d->ProbeTransformComboBox->currentNode() == NULL )
  {
    fiducialRegistrationWizardNode->SetProbeTransformID( "", vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }
  else
  {
    fiducialRegistrationWizardNode->SetProbeTransformID( d->ProbeTransformComboBox->currentNode()->GetID(), vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }

  if ( d->OutputTransformComboBox->currentNode() == NULL )
  {
    fiducialRegistrationWizardNode->SetOutputTransformID( "", vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }
  else
  {
    fiducialRegistrationWizardNode->SetOutputTransformID( d->OutputTransformComboBox->currentNode()->GetID(), vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }

  if ( d->FromMarkupsWidget->GetCurrentNode() == NULL )
  {
    fiducialRegistrationWizardNode->SetFromFiducialListID( "", vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }
  else
  {
    fiducialRegistrationWizardNode->SetFromFiducialListID( d->FromMarkupsWidget->GetCurrentNode()->GetID(), vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }

  if ( d->ToMarkupsWidget->GetCurrentNode() == NULL )
  {
    fiducialRegistrationWizardNode->SetToFiducialListID( "", vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }
  else
  {
    fiducialRegistrationWizardNode->SetToFiducialListID( d->ToMarkupsWidget->GetCurrentNode()->GetID(), vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }

  if ( d->SimilarityRadioButton->isChecked() )
  {
    fiducialRegistrationWizardNode->SetRegistrationMode( "Similarity", vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }
  if ( d->RigidRadioButton->isChecked() )
  {
    fiducialRegistrationWizardNode->SetRegistrationMode( "Rigid", vtkMRMLFiducialRegistrationWizardNode::NeverModify );
  }

  this->qvtkBlockAll( false );

  // The modified event will be blocked... Now allow it to happen
  d->logic()->GetFiducialRegistrationWizardNode()->Modified();
  this->UpdateFromMRMLNode();
}


void qSlicerFiducialRegistrationWizardModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->logic()->GetFiducialRegistrationWizardNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }

  // Disconnect to prevent signals form cuing slots
  disconnect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( UpdateToMRMLNode() ) );

  d->ProbeTransformComboBox->setCurrentNodeID( QString::fromStdString( fiducialRegistrationWizardNode->GetProbeTransformID() ) );
  d->OutputTransformComboBox->setCurrentNodeID( QString::fromStdString( fiducialRegistrationWizardNode->GetOutputTransformID() ) );
  d->FromMarkupsWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->GetFromFiducialListID() ) );
  d->ToMarkupsWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->GetToFiducialListID() ) );
  d->TransformPreviewWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->GetOutputTransformID() ) );

  if ( fiducialRegistrationWizardNode->GetRegistrationMode().compare( "Similarity" ) == 0 )
  {
    d->SimilarityRadioButton->setChecked( Qt::Checked );
    d->RigidRadioButton->setChecked( Qt::Unchecked );
  }
  if ( fiducialRegistrationWizardNode->GetRegistrationMode().compare( "Rigid" ) == 0 )
  {
    d->RigidRadioButton->setChecked( Qt::Checked );
    d->SimilarityRadioButton->setChecked( Qt::Unchecked );
  }

  // Unblock all singals from firing
  // TODO: Is there a more efficient way to do this by blokcing slots?
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( UpdateToMRMLNode() ) );

  std::stringstream statusString;
  statusString << "Status: ";
  statusString << d->logic()->GetOutputMessage();
  d->StatusLabel->setText( QString::fromStdString( statusString.str() ) ); // Also update the results
}