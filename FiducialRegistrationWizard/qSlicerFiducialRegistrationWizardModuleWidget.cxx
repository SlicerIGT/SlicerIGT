/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QtGui>
#include <QColor>

// SlicerQt includes
#include "qSlicerFiducialRegistrationWizardModuleWidget.h"
#include "qSlicerSimpleMarkupsWidget.h"
#include "qSlicerTransformPreviewWidget.h"
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

  vtkWeakPointer<vtkMRMLFiducialRegistrationWizardNode> FiducialRegistrationWizardNode;

  // Embedded widgets (in addition to widgets in the .ui file)
  qSlicerSimpleMarkupsWidget* FromMarkupsWidget;
  qSlicerSimpleMarkupsWidget* ToMarkupsWidget;
  qSlicerTransformPreviewWidget* TransformPreviewWidget;
};

//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidgetPrivate methods

//------------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModuleWidgetPrivate::qSlicerFiducialRegistrationWizardModuleWidgetPrivate( qSlicerFiducialRegistrationWizardModuleWidget& object ) : q_ptr( &object )
{
}

//------------------------------------------------------------------------------
vtkSlicerFiducialRegistrationWizardLogic* qSlicerFiducialRegistrationWizardModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerFiducialRegistrationWizardModuleWidget );
  return vtkSlicerFiducialRegistrationWizardLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidget methods


//------------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModuleWidget
::qSlicerFiducialRegistrationWizardModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerFiducialRegistrationWizardModuleWidgetPrivate( *this ) )
{
}

//------------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModuleWidget
::~qSlicerFiducialRegistrationWizardModuleWidget()
{
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onRecordButtonClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  
  vtkMRMLLinearTransformNode* probeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->ProbeTransformComboBox->currentNode() );
  d->logic()->AddFiducial( probeTransformNode );
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onUpdateButtonClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  d->logic()->UpdateCalibration( fiducialRegistrationWizardNode );
}

//------------------------------------------------------------------------------
std::string qSlicerFiducialRegistrationWizardModuleWidget::GetCorrespondingFiducialString()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkSlicerMarkupsLogic* markupsLogic = d->logic()->GetMarkupsLogic();
  if (markupsLogic==NULL)
  {
    qCritical("qSlicerFiducialRegistrationWizardModuleWidget::GetCorrespondingFiducialString failed: markups logic is invalid");
    return "";
  }

  std::stringstream correspondingFiducialString;
  correspondingFiducialString << "Place fiducial (";

  vtkMRMLMarkupsNode* activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->mrmlScene()->GetNodeByID( markupsLogic->GetActiveListID() ) );

  vtkMRMLMarkupsFiducialNode* fromMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->currentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->currentNode() );

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

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::setup()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  // Embed widgets here
  d->FromMarkupsWidget = new qSlicerSimpleMarkupsWidget();
  d->FromMarkupsWidget->setNodeBaseName( "From" );
  QColor fromMarkupsDefaultColor;
  fromMarkupsDefaultColor.setRgbF(1, 0, 0);
  d->FromMarkupsWidget->setDefaultNodeColor( fromMarkupsDefaultColor );
  d->FromGroupBox->layout()->addWidget( d->FromMarkupsWidget );

  d->ToMarkupsWidget = new qSlicerSimpleMarkupsWidget();
  d->ToMarkupsWidget->setNodeBaseName( "To" );
  QColor toMarkupsDefaultColor;
  toMarkupsDefaultColor.setRgbF(0, 0, 1);
  d->ToMarkupsWidget->setDefaultNodeColor(toMarkupsDefaultColor);
  d->ToGroupBox->layout()->addWidget( d->ToMarkupsWidget );

  d->TransformPreviewWidget = new qSlicerTransformPreviewWidget;
  d->PreviewGroupBox->layout()->addWidget( d->TransformPreviewWidget );

  connect( d->ModuleNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onFiducialRegistrationWizardNodeSelectionChanged()) );

  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->FromMarkupsWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ToMarkupsWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->TransformPreviewWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  // Make connections to update the mrml from the widget
  connect( d->ProbeTransformComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->OutputTransformComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->RigidRadioButton, SIGNAL(toggled(bool)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->SimilarityRadioButton, SIGNAL(toggled(bool)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->AutoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(UpdateToMRMLNode()));

  connect( d->FromMarkupsWidget, SIGNAL(markupsFiducialNodeChanged()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->FromMarkupsWidget, SIGNAL(markupsFiducialActivated()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->FromMarkupsWidget, SIGNAL(markupsFiducialPlaceModeChanged()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->FromMarkupsWidget, SIGNAL(updateFinished()), this, SLOT(PostProcessMarkupsWidgets()) );
  connect( d->ToMarkupsWidget, SIGNAL(markupsFiducialNodeChanged()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->ToMarkupsWidget, SIGNAL(markupsFiducialActivated()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->ToMarkupsWidget, SIGNAL(markupsFiducialPlaceModeChanged()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->ToMarkupsWidget, SIGNAL(updateFinished()), this, SLOT(PostProcessMarkupsWidgets()) );


  // These connections will do work (after being updated from the node)
  connect( d->RecordButton, SIGNAL(clicked()), this, SLOT(onRecordButtonClicked()) );
  connect( d->UpdateButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonClicked()) );

  this->UpdateFromMRMLNode();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::EnableAllWidgets( bool enable )
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->ProbeTransformComboBox->setEnabled( enable );
  d->OutputTransformComboBox->setEnabled( enable );
  d->RigidRadioButton->setEnabled( enable );
  d->SimilarityRadioButton->setEnabled( enable );
  d->FromMarkupsWidget->setEnabled( enable );
  d->ToMarkupsWidget->setEnabled( enable );
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::enter()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }  

  // Create a module MRML node if there is none in the scene.
  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLFiducialRegistrationWizardNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLFiducialRegistrationWizardNode > newNode = vtkSmartPointer< vtkMRMLFiducialRegistrationWizardNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLFiducialRegistrationWizardNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node vtkMRMLFiducialRegistrationWizardNode" );
    return;
  }

  // For convenience, select a default module.
  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    d->ModuleNodeComboBox->setCurrentNode( node );
  }
  else
  {
    this->UpdateFromMRMLNode();
  }

  this->Superclass::enter();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::UpdateToMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }

  bool allWasBlocked = this->qvtkBlockAll( true );

  fiducialRegistrationWizardNode->SetProbeTransformNodeId(d->ProbeTransformComboBox->currentNode()?d->ProbeTransformComboBox->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetOutputTransformNodeId(d->OutputTransformComboBox->currentNode()?d->OutputTransformComboBox->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetAndObserveFromFiducialListNodeId(d->FromMarkupsWidget->currentNode()?d->FromMarkupsWidget->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetAndObserveToFiducialListNodeId(d->ToMarkupsWidget->currentNode()?d->ToMarkupsWidget->currentNode()->GetID():NULL);

  fiducialRegistrationWizardNode->SetRegistrationMode( d->SimilarityRadioButton->isChecked() ? "Similarity" : "Rigid" );
  fiducialRegistrationWizardNode->SetUpdateMode( d->AutoUpdateCheckBox->isChecked() ? "Automatic" : "Manual" );

  this->qvtkBlockAll(allWasBlocked);

  // The modified event was blocked... Now allow it to happen
  d->ModuleNodeComboBox->currentNode()->Modified();
  this->UpdateFromMRMLNode();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::UpdateFromMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    this->EnableAllWidgets( false );
    d->StatusLabel->setText( "No Fiducial Registration Wizard module node selected." );
    return;
  }

  this->EnableAllWidgets( true );

  // Disconnect to prevent signals form cuing slots  
  bool wasProbeTransformComboBoxBlocked = d->ProbeTransformComboBox->blockSignals(true);
  bool wasOutputTransformComboBoxBlocked = d->OutputTransformComboBox->blockSignals(true);
  bool wasRigidRadioButtonBlocked = d->RigidRadioButton->blockSignals(true);
  bool wasSimilarityRadioButtonBlocked = d->SimilarityRadioButton->blockSignals(true);
  bool wasAutoUpdateCheckBoxBlocked = d->AutoUpdateCheckBox->blockSignals(true);
  bool wasFromMarkupsWidgetBlocked = d->FromMarkupsWidget->blockSignals(true);
  bool wasToMarkupsWidgetBlocked = d->ToMarkupsWidget->blockSignals(true);


  d->ProbeTransformComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetProbeTransformNode() );
  d->OutputTransformComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetOutputTransformNode() );
  d->TransformPreviewWidget->setCurrentNode( fiducialRegistrationWizardNode->GetOutputTransformNode() );
  d->RecordButton->setText( QString::fromStdString( this->GetCorrespondingFiducialString() ) );

  d->FromMarkupsWidget->setCurrentNode( fiducialRegistrationWizardNode->GetFromFiducialListNode() );
  d->ToMarkupsWidget->setCurrentNode( fiducialRegistrationWizardNode->GetToFiducialListNode() );

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

  if ( fiducialRegistrationWizardNode->GetUpdateMode().compare( "Automatic" ) == 0 )
  {
    d->AutoUpdateCheckBox->setChecked( Qt::Checked );
  }
  else
  {
    d->AutoUpdateCheckBox->setChecked( Qt::Unchecked );
  }

  // Unblock all signals from firing
  d->ProbeTransformComboBox->blockSignals(wasProbeTransformComboBoxBlocked);
  d->OutputTransformComboBox->blockSignals(wasOutputTransformComboBoxBlocked);
  d->RigidRadioButton->blockSignals(wasRigidRadioButtonBlocked);
  d->SimilarityRadioButton->blockSignals(wasSimilarityRadioButtonBlocked);
  d->AutoUpdateCheckBox->blockSignals(wasAutoUpdateCheckBoxBlocked);
  d->FromMarkupsWidget->blockSignals(wasFromMarkupsWidgetBlocked);
  d->ToMarkupsWidget->blockSignals(wasToMarkupsWidgetBlocked);

  std::stringstream statusString;
  statusString << "Status: ";
  statusString << d->logic()->GetOutputMessage( d->ModuleNodeComboBox->currentNode()->GetID() );
  d->StatusLabel->setText( QString::fromStdString( statusString.str() ) ); // Also update the results
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::PostProcessMarkupsWidgets()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkSlicerMarkupsLogic* markupsLogic = d->logic()->GetMarkupsLogic();
  if (markupsLogic==NULL)
  {
    qCritical("qSlicerFiducialRegistrationWizardModuleWidget::PostProcessMarkupsWidgets failed: markups logic is invalid");
    return;
  }

  vtkMRMLMarkupsNode* activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->mrmlScene()->GetNodeByID( markupsLogic->GetActiveListID() ) );

  vtkMRMLMarkupsFiducialNode* fromMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->currentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->currentNode() );

  if ( activeMarkupsNode != NULL && fromMarkupsNode != NULL && toMarkupsNode != NULL )
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

//-----------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onFiducialRegistrationWizardNodeSelectionChanged()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* selectedFiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  qvtkReconnect(d->FiducialRegistrationWizardNode, selectedFiducialRegistrationWizardNode, vtkCommand::ModifiedEvent, this, SLOT(UpdateFromMRMLNode()));
  d->FiducialRegistrationWizardNode = selectedFiducialRegistrationWizardNode;
  this->UpdateFromMRMLNode();
}
