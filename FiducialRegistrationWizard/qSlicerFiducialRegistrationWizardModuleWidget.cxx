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

  // Update the mrml node from the widget (since a widget has been changed)
  this->UpdateToMRMLNode();
  
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

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  // Make connections to update widget
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  connect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  connect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( updateWidget() ) );
  connect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( updateWidget() ) );

  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( updateWidget() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( updateWidget() ) );

  // These connections will do work (after being updated from the node)
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordButtonClicked() ) );

  // Watch the module node
  // Note: This module's node is a singleton, so we don't have to worry about it being replaced/deleted etc.
  // The logic function creates the singleton node if it doesn't already exist
  this->qvtkConnect( d->logic()->GetFiducialRegistrationWizardNode(), vtkCommand::ModifiedEvent, this, SLOT( UpdateFromMRMLNode() ) );
  this->qvtkConnect( this->mrmlScene(), vtkMRMLScene::EndImportEvent, this, SLOT( UpdateFromMRMLNode() ) );

  this->UpdateFromMRMLNode();
  this->updateWidget();
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

  fiducialRegistrationWizardNode->ProbeTransformID = d->ProbeTransformComboBox->currentNodeID().toStdString();
  if ( d->FromMarkupsWidget->GetCurrentNode() != NULL )
  {
    fiducialRegistrationWizardNode->FromFiducialListID = d->FromMarkupsWidget->GetCurrentNode()->GetID();
  }
  if ( d->ToMarkupsWidget->GetCurrentNode() != NULL )
  {
    fiducialRegistrationWizardNode->ToFiducialListID = d->ToMarkupsWidget->GetCurrentNode()->GetID();
  }
  fiducialRegistrationWizardNode->ActiveFiducialListID = d->logic()->MarkupsLogic->GetActiveListID();
  fiducialRegistrationWizardNode->OutputTransformID = d->OutputTransformComboBox->currentNodeID().toStdString();

  if ( d->SimilarityRadioButton->isChecked() )
  {
    fiducialRegistrationWizardNode->RegistrationMode = "Similarity";
  }
  if ( d->RigidRadioButton->isChecked() )
  {
    fiducialRegistrationWizardNode->RegistrationMode = "Rigid";
  }

  this->qvtkBlockAll( false );
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

  // Disconnect to prevent signals form queuing slots
  disconnect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  disconnect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  disconnect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( updateWidget() ) );
  disconnect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( updateWidget() ) );
  disconnect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( updateWidget() ) );
  disconnect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( updateWidget() ) );

  d->ProbeTransformComboBox->setCurrentNodeID( QString::fromStdString( fiducialRegistrationWizardNode->ProbeTransformID ) );
  d->FromMarkupsWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->FromFiducialListID ) );
  d->ToMarkupsWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->ToFiducialListID ) );
  vtkMRMLMarkupsNode* activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->ActiveFiducialListID ) );
  if ( activeMarkupsNode != NULL )
  {
    d->logic()->MarkupsLogic->SetActiveListID( activeMarkupsNode );
  }
  // Do this to cause the markups widgets to update their Active buttons
  if ( d->FromMarkupsWidget->GetCurrentNode() != NULL )
  {
    d->FromMarkupsWidget->GetCurrentNode()->Modified();
  }
  if ( d->ToMarkupsWidget->GetCurrentNode() != NULL )
  {
    d->ToMarkupsWidget->GetCurrentNode()->Modified();
  }
  d->OutputTransformComboBox->setCurrentNodeID( QString::fromStdString( fiducialRegistrationWizardNode->OutputTransformID ) );


  if ( fiducialRegistrationWizardNode->RegistrationMode.compare( "Similarity" ) == 0 )
  {
    d->SimilarityRadioButton->setChecked( Qt::Checked );
    d->RigidRadioButton->setChecked( Qt::Unchecked );
  }
  if ( fiducialRegistrationWizardNode->RegistrationMode.compare( "Rigid" ) == 0 )
  {
    d->RigidRadioButton->setChecked( Qt::Checked );
    d->SimilarityRadioButton->setChecked( Qt::Unchecked );
  }

  // Unblock all singals from firing
  // TODO: Is there a more efficient way to do this by blokcing slots?
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  connect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateWidget() ) );
  connect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( updateWidget() ) );
  connect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( updateWidget() ) );
  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( updateWidget() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeModified() ), this, SLOT( updateWidget() ) );

  this->updateWidget();
}