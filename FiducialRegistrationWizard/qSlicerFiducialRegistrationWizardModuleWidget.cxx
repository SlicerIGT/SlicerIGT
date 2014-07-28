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


std::string qSlicerFiducialRegistrationWizardModuleWidget
::GetCorrespondingFiducialString()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  std::stringstream correspondingFiducialString;
  correspondingFiducialString << "Place Fiducial (";

  vtkMRMLMarkupsNode* activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->mrmlScene()->GetNodeByID( d->logic()->MarkupsLogic->GetActiveListID() ) );

  vtkMRMLMarkupsFiducialNode* fromMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->GetCurrentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->GetCurrentNode() );

  if ( fromMarkupsNode != NULL && toMarkupsNode != NULL )
  {
    // Adding to "From" list
    if ( strcmp( activeMarkupsNode->GetID(), fromMarkupsNode->GetID() ) == 0 )
    {
      if ( fromMarkupsNode->GetNumberOfFiducials() < toMarkupsNode->GetNumberOfFiducials() )
      {
        correspondingFiducialString << "corresponding to ";
        correspondingFiducialString << toMarkupsNode->GetNthFiducialLabel( fromMarkupsNode->GetNumberOfFiducials() );
      }
      d->ToMarkupsWidget->highlightNthFiducial( fromMarkupsNode->GetNumberOfFiducials() );
    }

    // Adding to "To" list
    if ( strcmp( activeMarkupsNode->GetID(), toMarkupsNode->GetID() ) == 0 )
    {
      if ( toMarkupsNode->GetNumberOfFiducials() < fromMarkupsNode->GetNumberOfFiducials() )
      {
        correspondingFiducialString << "corresponding to ";
        correspondingFiducialString << fromMarkupsNode->GetNthFiducialLabel( toMarkupsNode->GetNumberOfFiducials() );
      }
      d->FromMarkupsWidget->highlightNthFiducial( toMarkupsNode->GetNumberOfFiducials() );
    }

  }

  correspondingFiducialString << ")";
  return correspondingFiducialString.str();
}


void qSlicerFiducialRegistrationWizardModuleWidget
::setup()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  // Embed widgets here
  d->FromMarkupsWidget = qSlicerSimpleMarkupsWidget::New( d->logic()->MarkupsLogic );
  d->FromMarkupsWidget->SetNodeBaseName( "From" );
  double FROM_MARKUPS_DEFAULT_COLOR[3] = { 1, 0, 0 };
  d->FromMarkupsWidget->SetDefaultNodeColor( FROM_MARKUPS_DEFAULT_COLOR );
  d->FromGroupBox->layout()->addWidget( d->FromMarkupsWidget );

  d->ToMarkupsWidget = qSlicerSimpleMarkupsWidget::New( d->logic()->MarkupsLogic );
  d->ToMarkupsWidget->SetNodeBaseName( "To" );
  double TO_MARKUPS_DEFAULT_COLOR[3] = { 0, 0, 1 };
  d->ToMarkupsWidget->SetDefaultNodeColor( TO_MARKUPS_DEFAULT_COLOR );
  d->ToGroupBox->layout()->addWidget( d->ToMarkupsWidget );

  d->TransformPreviewWidget = qSlicerTransformPreviewWidget::New( d->logic()->GetMRMLScene() );
  d->PreviewGroupBox->layout()->addWidget( d->TransformPreviewWidget );

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateFromMRMLNode() ) );

  // Make connections to update the mrml from the widget
  this->ConnectAllUpdateWidgets();

  // These connections will do work (after being updated from the node)
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordButtonClicked() ) );

  // Watch the logic - it is updated whenver the mrml node is updated
  this->qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( UpdateFromMRMLNode() ) );

  this->UpdateFromMRMLNode();
}


void qSlicerFiducialRegistrationWizardModuleWidget
::ConnectAllUpdateWidgets()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  // Make connections to update the mrml from the widget
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );

  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialActivated() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->FromMarkupsWidget, SIGNAL( markupsFiducialPlaceModeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->FromMarkupsWidget, SIGNAL( updateFinished() ), this, SLOT( PostProcessMarkupsWidgets() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialActivated() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->ToMarkupsWidget, SIGNAL( markupsFiducialPlaceModeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  connect( d->ToMarkupsWidget, SIGNAL( updateFinished() ), this, SLOT( PostProcessMarkupsWidgets() ) );
}


void qSlicerFiducialRegistrationWizardModuleWidget
::DisconnectAllUpdateWidgets()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  // Make connections to update the mrml from the widget
  disconnect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->SimilarityRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );

  disconnect( d->FromMarkupsWidget, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->FromMarkupsWidget, SIGNAL( markupsFiducialActivated() ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->FromMarkupsWidget, SIGNAL( markupsFiducialPlaceModeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->ToMarkupsWidget, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->ToMarkupsWidget, SIGNAL( markupsFiducialActivated() ), this, SLOT( UpdateToMRMLNode() ) );
  disconnect( d->ToMarkupsWidget, SIGNAL( markupsFiducialPlaceModeChanged() ), this, SLOT( UpdateToMRMLNode() ) );
}


void qSlicerFiducialRegistrationWizardModuleWidget
::EnableAllWidgets( bool enable )
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->ProbeTransformComboBox->setEnabled( enable );
  d->OutputTransformComboBox->setEnabled( enable );
  d->RigidRadioButton->setEnabled( enable );
  d->SimilarityRadioButton->setEnabled( enable );
  d->FromMarkupsWidget->setEnabled( enable );
  d->ToMarkupsWidget->setEnabled( enable );
}


void qSlicerFiducialRegistrationWizardModuleWidget
::enter()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  this->qSlicerAbstractModuleWidget::enter();

  // Create a node by default if none already exists
  int numFiducialRegistrationWizardNodes = this->mrmlScene()->GetNumberOfNodesByClass( "vtkMRMLFiducialRegistrationWizardNode" );
  if ( numFiducialRegistrationWizardNodes == 0 )
  {
    vtkSmartPointer< vtkMRMLNode > fiducialRegistrationWizardNode;
    fiducialRegistrationWizardNode.TakeReference( this->mrmlScene()->CreateNodeByClass( "vtkMRMLFiducialRegistrationWizardNode" ) );
    fiducialRegistrationWizardNode->SetScene( this->mrmlScene() );
    this->mrmlScene()->AddNode( fiducialRegistrationWizardNode );
    d->ModuleNodeComboBox->setCurrentNode( fiducialRegistrationWizardNode );
  }
  else
  {
    this->UpdateFromMRMLNode();
  }

}


void qSlicerFiducialRegistrationWizardModuleWidget
::UpdateToMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

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
  d->ModuleNodeComboBox->currentNode()->Modified();
  this->UpdateFromMRMLNode();
}


void qSlicerFiducialRegistrationWizardModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    this->EnableAllWidgets( false );
    d->FromGroupBox->setStyleSheet( "QGroupBox { font-weight : normal; background-color: white }" );
    d->ToGroupBox->setStyleSheet( "QGroupBox { font-weight : normal; background-color: white }" );
    d->StatusLabel->setText( "No Fiducial Registration Wizard module node selected." );
    return;
  }

  this->EnableAllWidgets( true );

  // Disconnect to prevent signals form cuing slots
  this->DisconnectAllUpdateWidgets();

  std::string fromFiducialListID = fiducialRegistrationWizardNode->GetFromFiducialListID();
  std::string toFiducialListID = fiducialRegistrationWizardNode->GetToFiducialListID();

  d->ProbeTransformComboBox->setCurrentNodeID( QString::fromStdString( fiducialRegistrationWizardNode->GetProbeTransformID() ) );
  d->OutputTransformComboBox->setCurrentNodeID( QString::fromStdString( fiducialRegistrationWizardNode->GetOutputTransformID() ) );
  d->TransformPreviewWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->GetOutputTransformID() ) );
  d->RecordButton->setText( QString::fromStdString( this->GetCorrespondingFiducialString() ) );

  d->FromMarkupsWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->GetFromFiducialListID() ) );
  d->ToMarkupsWidget->SetCurrentNode( this->mrmlScene()->GetNodeByID( fiducialRegistrationWizardNode->GetToFiducialListID() ) );


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
  this->ConnectAllUpdateWidgets();

  std::stringstream statusString;
  statusString << "Status: ";
  statusString << d->logic()->GetOutputMessage( d->ModuleNodeComboBox->currentNode()->GetID() );
  d->StatusLabel->setText( QString::fromStdString( statusString.str() ) ); // Also update the results
}


void qSlicerFiducialRegistrationWizardModuleWidget
::PostProcessMarkupsWidgets()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLMarkupsNode* activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->mrmlScene()->GetNodeByID( d->logic()->MarkupsLogic->GetActiveListID() ) );

  vtkMRMLMarkupsFiducialNode* fromMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->GetCurrentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->GetCurrentNode() );

  if ( fromMarkupsNode != NULL && toMarkupsNode != NULL )
  {
    // Adding to "From" list
    if ( strcmp( activeMarkupsNode->GetID(), fromMarkupsNode->GetID() ) == 0 )
    {
      d->ToMarkupsWidget->highlightNthFiducial( fromMarkupsNode->GetNumberOfFiducials() );
    }

    // Adding to "To" list
    if ( strcmp( activeMarkupsNode->GetID(), toMarkupsNode->GetID() ) == 0 )
    {
      d->FromMarkupsWidget->highlightNthFiducial( toMarkupsNode->GetNumberOfFiducials() );
    }
  }

}