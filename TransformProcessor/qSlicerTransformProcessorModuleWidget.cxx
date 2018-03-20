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

  This file was originally developed by Franklin King, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/


// Qt includes
#include <QTimer>
#include <QListWidgetItem>
#include <QMenu>

// SlicerQt includes
#include "qSlicerTransformProcessorModuleWidget.h"
#include "ui_qSlicerTransformProcessorModule.h"

// Transform Processor includes
#include "vtkSlicerTransformProcessorLogic.h"
#include "vtkMRMLTransformProcessorNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLLinearTransformNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformProcessor
class qSlicerTransformProcessorModuleWidgetPrivate: public Ui_qSlicerTransformProcessorModule
{
  Q_DECLARE_PUBLIC( qSlicerTransformProcessorModuleWidget ); 
  
protected:
  qSlicerTransformProcessorModuleWidget* const q_ptr;
public:
  qSlicerTransformProcessorModuleWidgetPrivate( qSlicerTransformProcessorModuleWidget& object );
  vtkSlicerTransformProcessorLogic* logic() const;

  vtkWeakPointer< vtkMRMLTransformProcessorNode > TransformProcessorNode;
};

//-----------------------------------------------------------------------------
// qSlicerTransformProcessorModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerTransformProcessorModuleWidgetPrivate::qSlicerTransformProcessorModuleWidgetPrivate( qSlicerTransformProcessorModuleWidget& object ) : q_ptr( &object )
{
}

vtkSlicerTransformProcessorLogic* qSlicerTransformProcessorModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTransformProcessorModuleWidget );
  return vtkSlicerTransformProcessorLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerTransformProcessorModuleWidget methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
qSlicerTransformProcessorModuleWidget::qSlicerTransformProcessorModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerTransformProcessorModuleWidgetPrivate( *this ) )
{
  this->updateTimer = new QTimer();
  updateTimer->setSingleShot( false );
  updateTimer->setInterval( 17 ); // default is once every sixtieth of a second, = 17 milliseconds
}

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModuleWidget::~qSlicerTransformProcessorModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::setup()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  d->setupUi( this );
  this->Superclass::setup();

  // processing modes
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_QUATERNION_AVERAGE ).c_str() );
  d->processingModeComboBox->setItemData( 0, "Compute the quaternion average of all Source transforms provided.", Qt::ToolTipRole );
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_ROTATION ).c_str() );
  d->processingModeComboBox->setItemData( 1, "Compute a copy of the rotation from the Source to the Reference.", Qt::ToolTipRole );
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_TRANSLATION ).c_str() );
  d->processingModeComboBox->setItemData( 2, "Compute a copy of the translation from the Source to the Reference.", Qt::ToolTipRole );
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM ).c_str() );
  d->processingModeComboBox->setItemData( 3, "Compute a copy of the full transform from the Source to the Reference.", Qt::ToolTipRole );
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_INVERSE ).c_str() );
  d->processingModeComboBox->setItemData( 4, "Compute the inverse of transform to parent, and store it in another node.", Qt::ToolTipRole );
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT ).c_str() );
  d->processingModeComboBox->setItemData( 5, "Compute a constrained version of an Source transform, the translation and z direction are preserved but the other axes resemble the Target coordinate system.", Qt::ToolTipRole );

  d->advancedRotationModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetRotationModeAsString( vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_ALL_AXES ).c_str() );
  d->advancedRotationModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetRotationModeAsString( vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_SINGLE_AXIS ).c_str() );

  d->advancedRotationPrimaryAxisComboBox->addItem( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( vtkMRMLTransformProcessorNode::AXIS_LABEL_X ).c_str() );
  d->advancedRotationPrimaryAxisComboBox->addItem( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( vtkMRMLTransformProcessorNode::AXIS_LABEL_Y ).c_str() );
  d->advancedRotationPrimaryAxisComboBox->addItem( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( vtkMRMLTransformProcessorNode::AXIS_LABEL_Z ).c_str() );

  d->advancedRotationDependentAxesModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetDependentAxesModeAsString( vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_PIVOT ).c_str() );
  d->advancedRotationDependentAxesModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetDependentAxesModeAsString( vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS ).c_str() );
  
  d->advancedRotationSecondaryAxisComboBox->addItem( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( vtkMRMLTransformProcessorNode::AXIS_LABEL_X ).c_str() );
  d->advancedRotationSecondaryAxisComboBox->addItem( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( vtkMRMLTransformProcessorNode::AXIS_LABEL_Y ).c_str() );
  d->advancedRotationSecondaryAxisComboBox->addItem( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( vtkMRMLTransformProcessorNode::AXIS_LABEL_Z ).c_str() );

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  // set up connections
  connect( d->parameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeChanged() ) );

  connect( d->inputFromTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputFromTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputToTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputToTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputInitialTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputInitialTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputChangedTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputChangedTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputAnchorTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputAnchorTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputForwardTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputForwardTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->outputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onOutputTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->addInputCombineTransformButton, SIGNAL( clicked() ), this, SLOT( onAddInputCombineTransform() ) );
  connect( d->removeInputCombineTransformButton, SIGNAL( clicked() ), this, SLOT( onRemoveInputCombineTransform() ) );
  
  connect( d->processingModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onProcessingModeChanged( int ) ) );

  connect( d->advancedRotationModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onRotationModeChanged( int ) ) );
  connect( d->advancedRotationPrimaryAxisComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onPrimaryAxisChanged( int ) ) );
  connect( d->advancedRotationDependentAxesModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onDependentAxesModeChanged( int ) ) );
  connect( d->advancedRotationSecondaryAxisComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSecondaryAxisChanged( int ) ) );

  connect( d->advancedTranslationCopyXCheckbox, SIGNAL( clicked() ), this, SLOT( onCopyTranslationChanged( ) ) );
  connect( d->advancedTranslationCopyYCheckbox, SIGNAL( clicked() ), this, SLOT( onCopyTranslationChanged( ) ) );
  connect( d->advancedTranslationCopyZCheckbox, SIGNAL( clicked() ), this, SLOT( onCopyTranslationChanged( ) ) );

  connect( d->updateButton, SIGNAL( clicked() ), this, SLOT( onUpdateButtonPressed() ) );
  connect( d->updateButton, SIGNAL( checkBoxToggled( bool ) ), this, SLOT( onUpdateButtonCheckboxToggled( bool ) ) );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::enter()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  if ( this->mrmlScene() == NULL )
  {
    qCritical( "Invalid scene!" );
    return;
  }

  // Create a module MRML node if there is none in the scene.
  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLTransformProcessorNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLTransformProcessorNode > newNode = vtkSmartPointer< vtkMRMLTransformProcessorNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLTransformProcessorNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node vtkMRMLTransformProcessorNode" );
    return;
  }

  // For convenience, select a default module.
  if ( d->parameterNodeComboBox->currentNode() == NULL )
  {
    d->parameterNodeComboBox->setCurrentNode( node );
  }
  else
  {
    this->updateGUIFromMRML();
  }

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::setSignalsBlocked( bool newBlock )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  d->parameterNodeComboBox->blockSignals( newBlock );
  d->processingModeComboBox->blockSignals( newBlock );
  d->inputFromTransformComboBox->blockSignals( newBlock );
  d->inputToTransformComboBox->blockSignals( newBlock );
  d->inputInitialTransformComboBox->blockSignals( newBlock );
  d->inputChangedTransformComboBox->blockSignals( newBlock );
  d->inputAnchorTransformComboBox->blockSignals( newBlock );
  d->inputForwardTransformComboBox->blockSignals( newBlock );
  d->outputTransformComboBox->blockSignals( newBlock );
  d->advancedRotationModeComboBox->blockSignals( newBlock );
  d->advancedRotationPrimaryAxisComboBox->blockSignals( newBlock );
  d->advancedRotationDependentAxesModeComboBox->blockSignals( newBlock );
  d->advancedRotationSecondaryAxisComboBox->blockSignals( newBlock );
  d->advancedTranslationCopyXCheckbox->blockSignals( newBlock );
  d->advancedTranslationCopyYCheckbox->blockSignals( newBlock );
  d->advancedTranslationCopyZCheckbox->blockSignals( newBlock );
  d->updateButton->blockSignals( newBlock );
}

//-----------------------------------------------------------------------------
bool qSlicerTransformProcessorModuleWidget::getSignalsBlocked()
{
  Q_D( qSlicerTransformProcessorModuleWidget );

  bool parameterNodeBlocked = d->parameterNodeComboBox->signalsBlocked();

  // all signal blocks should be the same as parameterNodeBlocked. If not, then output a warning message.
  if ( parameterNodeBlocked == d->processingModeComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->inputFromTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->inputToTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->inputInitialTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->inputChangedTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->inputAnchorTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->inputForwardTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->outputTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedRotationModeComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedRotationPrimaryAxisComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedRotationDependentAxesModeComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedRotationSecondaryAxisComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedTranslationCopyXCheckbox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedTranslationCopyYCheckbox->signalsBlocked() &&
       parameterNodeBlocked == d->advancedTranslationCopyZCheckbox->signalsBlocked() &&
       parameterNodeBlocked == d->updateButton->signalsBlocked() )
  {
    return parameterNodeBlocked;
  }
  else
  {
    qWarning( "getAllSignalsBlocked: Inconsistent blocking detected. This is indicative of a coding error. Assuming that signals are NOT blocked." );
    return false;
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::updateGUIFromMRML()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical("Error: Failed to update widget, no parameter node/scene found.");
    return;
  }

  bool wasBlocked = this->getSignalsBlocked();
  this->setSignalsBlocked( true );

  // == Populate node and parameter selections ==

  d->inputCombineTransformList->clear();
  for ( int i = 0; i < pNode->GetNumberOfInputCombineTransformNodes(); i++ )
  {
    new QListWidgetItem( tr( pNode->GetNthInputCombineTransformNode( i )->GetName() ), d->inputCombineTransformList );
  }
  d->inputFromTransformComboBox->setCurrentNode( pNode->GetInputFromTransformNode() );
  d->inputToTransformComboBox->setCurrentNode( pNode->GetInputToTransformNode() );
  d->inputInitialTransformComboBox->setCurrentNode( pNode->GetInputInitialTransformNode() );
  d->inputChangedTransformComboBox->setCurrentNode( pNode->GetInputChangedTransformNode() );
  d->inputAnchorTransformComboBox->setCurrentNode( pNode->GetInputAnchorTransformNode() );
  d->inputForwardTransformComboBox->setCurrentNode( pNode->GetInputForwardTransformNode() );
  d->outputTransformComboBox->setCurrentNode( pNode->GetOutputTransformNode() );

  d->advancedTranslationCopyXCheckbox->setChecked( pNode->GetCopyTranslationX() );
  d->advancedTranslationCopyYCheckbox->setChecked( pNode->GetCopyTranslationY() );
  d->advancedTranslationCopyZCheckbox->setChecked( pNode->GetCopyTranslationZ() );

  int processingComboBoxIndex = d->processingModeComboBox->findText( QString( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( pNode->GetProcessingMode() ).c_str() ) );
  if ( processingComboBoxIndex < 0 )
  {
    processingComboBoxIndex = 0;
  }
  d->processingModeComboBox->setCurrentIndex( processingComboBoxIndex );

  int rotationComboBoxIndex = d->advancedRotationModeComboBox->findText( QString( vtkMRMLTransformProcessorNode::GetRotationModeAsString( pNode->GetRotationMode() ).c_str() ) );
  if ( rotationComboBoxIndex < 0 )
  {
    rotationComboBoxIndex = 0;
  }
  d->advancedRotationModeComboBox->setCurrentIndex( rotationComboBoxIndex );
  
  int primaryAxisComboBoxIndex = d->advancedRotationPrimaryAxisComboBox->findText( QString( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( pNode->GetPrimaryAxisLabel() ).c_str() ) );
  if ( primaryAxisComboBoxIndex < 0 )
  {
    primaryAxisComboBoxIndex = 0;
  }
  d->advancedRotationPrimaryAxisComboBox->setCurrentIndex( primaryAxisComboBoxIndex );

  int dependentAxesModeIndex = d->advancedRotationDependentAxesModeComboBox->findText( QString( vtkMRMLTransformProcessorNode::GetDependentAxesModeAsString( pNode->GetDependentAxesMode() ).c_str() ) );
  if ( dependentAxesModeIndex < 0 )
  {
    dependentAxesModeIndex = 0;
  }
  d->advancedRotationDependentAxesModeComboBox->setCurrentIndex( dependentAxesModeIndex );
  
  int secondaryAxisComboBoxIndex = d->advancedRotationSecondaryAxisComboBox->findText( QString( vtkMRMLTransformProcessorNode::GetAxisLabelAsString( pNode->GetSecondaryAxisLabel() ).c_str() ) );
  if ( secondaryAxisComboBoxIndex < 0 )
  {
    secondaryAxisComboBoxIndex = 0;
  }
  d->advancedRotationSecondaryAxisComboBox->setCurrentIndex( secondaryAxisComboBoxIndex );

  // == update visibility of widgets ==

  bool showCombineTransformList = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_QUATERNION_AVERAGE );
  d->inputCombineTransformListGroupBox->setVisible( showCombineTransformList );

  bool showFromToTransform = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_ROTATION ||
                               pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_TRANSLATION ||
                               pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM );
  d->inputFromTransformLabel->setVisible( showFromToTransform );
  d->inputFromTransformComboBox->setVisible( showFromToTransform );
  d->inputToTransformLabel->setVisible( showFromToTransform );
  d->inputToTransformComboBox->setVisible( showFromToTransform );

  bool showTranslationGroupBox = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_TRANSLATION ||
                                   pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM );
  d->advancedTranslationGroupBox->setVisible( showTranslationGroupBox );

  bool showRotationGroupBox = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_ROTATION ||
                                pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM );
  d->advancedRotationGroupBox->setVisible( showRotationGroupBox );

  bool showRotationPrimaryAxis = ( showRotationGroupBox &&
                                   pNode->GetRotationMode() == vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_SINGLE_AXIS );
  d->advancedRotationPrimaryAxisLabel->setVisible( showRotationPrimaryAxis );
  d->advancedRotationPrimaryAxisComboBox->setVisible( showRotationPrimaryAxis );
  d->advancedRotationDependentAxesModeLabel->setVisible( showRotationPrimaryAxis );
  d->advancedRotationDependentAxesModeComboBox->setVisible( showRotationPrimaryAxis );

  bool showRotationSecondaryAxis = ( showRotationPrimaryAxis &&
                                     pNode->GetDependentAxesMode() == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS );
  d->advancedRotationSecondaryAxisLabel->setVisible( showRotationSecondaryAxis );
  d->advancedRotationSecondaryAxisComboBox->setVisible( showRotationSecondaryAxis );

  bool showInitialChangedTransform = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT );
  d->inputInitialTransformLabel->setVisible( showInitialChangedTransform );
  d->inputInitialTransformComboBox->setVisible( showInitialChangedTransform );
  d->inputChangedTransformLabel->setVisible( showInitialChangedTransform );
  d->inputChangedTransformComboBox->setVisible( showInitialChangedTransform );

  bool showAnchorTransform = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT );
  d->inputAnchorTransformLabel->setVisible( showAnchorTransform );
  d->inputAnchorTransformComboBox->setVisible( showAnchorTransform );

  bool showForwardTransform = ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_INVERSE );
  d->inputForwardTransformLabel->setVisible( showForwardTransform );
  d->inputForwardTransformComboBox->setVisible( showForwardTransform );

  d->outputTransformLabel->setVisible( true ); // always visible
  d->outputTransformComboBox->setVisible( true );

  // == update the main button ==

  bool conditionsForCurrentProcessingMode = d->logic()->IsTransformProcessingPossible( pNode );
  if ( conditionsForCurrentProcessingMode )
  {
    d->updateButton->setEnabled( true );
  }
  else
  {
    d->updateButton->setEnabled( false );
  }
  
  if ( pNode->GetUpdateMode() == vtkMRMLTransformProcessorNode::UPDATE_MODE_MANUAL )
  {
    bool wasBlocked = d->updateButton->blockSignals( true );
    d->updateButton->setCheckable( false );
    d->updateButton->setChecked( false );
    d->updateButton->setText( tr( "Update" ) );
    d->updateButton->blockSignals( wasBlocked );
  }
  else if ( pNode->GetUpdateMode() == vtkMRMLTransformProcessorNode::UPDATE_MODE_AUTO )
  {
    bool wasBlocked = d->updateButton->blockSignals( true );
    d->updateButton->setCheckable( true );
    d->updateButton->setChecked( true );
    d->updateButton->setText( tr( "Auto-update" ) );
    d->updateButton->blockSignals( wasBlocked );
  }
  else
  {
    qWarning( "Error: Unrecognized update mode. Putting button in manual update mode" );
    bool wasBlocked = d->updateButton->blockSignals( true );
    d->updateButton->setCheckable( false );
    d->updateButton->setChecked( false );
    d->updateButton->setText( tr( "Update" ) );
    d->updateButton->blockSignals( wasBlocked );
  }

  this->setSignalsBlocked( wasBlocked );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onUpdateButtonPressed()
{
  this->singleUpdate();
}

//------------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onUpdateButtonCheckboxToggled( bool checked )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: No parameter node." );
    return;
  }

  if ( checked )
  {
    bool verboseConditionChecking = true;
    bool conditionsMetForProcessing = d->logic()->IsTransformProcessingPossible( pNode, verboseConditionChecking );
    if ( conditionsMetForProcessing == true )
    {
      pNode->SetUpdateModeToAuto();
    }
    else
    {
      qWarning( "Cannot set to automatic mode - fix afore-mentioned problems first." );
      pNode->SetUpdateModeToManual();
    }
  }
  else
  {
    pNode->SetUpdateModeToManual();
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::singleUpdate()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update, no parameter node/scene found." );
    return;
  }

  bool verboseConditionChecking = true;
  bool conditionsMetForProcessing = d->logic()->IsTransformProcessingPossible( pNode, verboseConditionChecking );
  if (conditionsMetForProcessing == true)
  {
    d->logic()->UpdateOutputTransform( pNode );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::handleEventAutoUpdate()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to auto-update, no parameter node/scene found." );
    return;
  }

  if ( pNode->GetUpdateMode() == vtkMRMLTransformProcessorNode::UPDATE_MODE_AUTO )
  {
    bool verboseConditionChecking = true;
    if (d->logic()->IsTransformProcessingPossible(pNode,verboseConditionChecking) == true)
    {
      this->singleUpdate();
    }
    else
    {
      qWarning( "Stopping automatic update - fix afore-mentioned problems before turning it back on." );
      pNode->SetUpdateModeToManual();
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputFromTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_FROM );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputToTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_TO );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputInitialTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_INITIAL );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputChangedTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_CHANGED );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputAnchorTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_ANCHOR );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputForwardTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_FORWARD );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onOutputTransformNodeSelected( vtkMRMLNode* node )
{
  this->SetTransformAccordingToRole( node, TRANSFORM_ROLE_OUTPUT );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onAddInputCombineTransform()
{
  Q_D( qSlicerTransformProcessorModuleWidget );

  if ( d->addInputCombineTransformComboBox->currentNode() == NULL )
  {
    return; // don't do anything if no input node is selected
  }
  
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to add transform to input list, no parameter node/scene found." );
    return;
  }

  vtkMRMLLinearTransformNode* sourceTransform = vtkMRMLLinearTransformNode::SafeDownCast( d->addInputCombineTransformComboBox->currentNode() );
  if ( sourceTransform == NULL )
  {
    qCritical( "Error: Failed to add transform to input list, input transform node is not a linear transform node." );
    return;
  }

  pNode->AddAndObserveInputCombineTransformNode( sourceTransform );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onRemoveInputCombineTransform()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to remove transform from input list, no parameter node/scene found." );
    return;
  }

  int selectedSourceListIndex = d->inputCombineTransformList->currentRow();

  if ( selectedSourceListIndex >= 0 && selectedSourceListIndex < pNode->GetNumberOfInputCombineTransformNodes() )
  {
    pNode->RemoveNthInputCombineTransformNode( selectedSourceListIndex );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::SetTransformAccordingToRole( vtkMRMLNode* node, TransformRole role )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: No parameter node/scene found. Cannot set transform to paramter node." );
    return;
  }

  vtkMRMLLinearTransformNode* linearTransformNode = NULL;
  if ( node )
  {
    linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
    if ( linearTransformNode == NULL )
    {
      qCritical( "Error: MRML node is not a linear transform. Cannot set transform to paramter node." );
      return;
    }
  }

  switch ( role )
  {
    case TRANSFORM_ROLE_INPUT_FROM:
    {
      pNode->SetAndObserveInputFromTransformNode( linearTransformNode );
      break;
    }
    case TRANSFORM_ROLE_INPUT_TO:
    {
      pNode->SetAndObserveInputToTransformNode( linearTransformNode );
      break;
    }
    case TRANSFORM_ROLE_INPUT_INITIAL:
    {
      pNode->SetAndObserveInputInitialTransformNode( linearTransformNode );
      break;
    }
    case TRANSFORM_ROLE_INPUT_CHANGED:
    {
      pNode->SetAndObserveInputChangedTransformNode( linearTransformNode );
      break;
    }
    case TRANSFORM_ROLE_INPUT_ANCHOR:
    {
      pNode->SetAndObserveInputAnchorTransformNode( linearTransformNode );
      break;
    }
    case TRANSFORM_ROLE_INPUT_FORWARD:
    {
      pNode->SetAndObserveInputForwardTransformNode( linearTransformNode );
      break;
    }
    case TRANSFORM_ROLE_OUTPUT:
    {
      pNode->SetAndObserveOutputTransformNode( linearTransformNode );
      break;
    }
    default:
    {
      qCritical( "Error: Transform role is not recognized. Cannot set transform to paramter node." );
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onProcessingModeChanged( int )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to change processing mode, no parameter node/scene found." );
    return;
  }

  std::string processingModeAsString = d->processingModeComboBox->currentText().toStdString();
  int processingModeAsEnum = vtkMRMLTransformProcessorNode::GetProcessingModeFromString( processingModeAsString );
  pNode->SetProcessingMode( processingModeAsEnum );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onRotationModeChanged( int )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to change rotation mode, no parameter node/scene found." );
    return;
  }

  std::string rotationModeAsString = d->advancedRotationModeComboBox->currentText().toStdString();
  int rotationModeAsEnum = vtkMRMLTransformProcessorNode::GetRotationModeFromString( rotationModeAsString );
  pNode->SetRotationMode( rotationModeAsEnum );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onPrimaryAxisChanged( int )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to change primary axis, no parameter node/scene found." );
    return;
  }

  std::string primaryAxisAsString = d->advancedRotationPrimaryAxisComboBox->currentText().toStdString();
  int primaryAxisAsEnum = vtkMRMLTransformProcessorNode::GetAxisLabelFromString( primaryAxisAsString );
  pNode->SetPrimaryAxisLabel( primaryAxisAsEnum );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onDependentAxesModeChanged( int )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to change dependent axes mode, no parameter node/scene found." );
    return;
  }

  std::string dependentAxesModeAsString = d->advancedRotationDependentAxesModeComboBox->currentText().toStdString();
  int dependentAxesModeAsEnum = vtkMRMLTransformProcessorNode::GetDependentAxesModeFromString( dependentAxesModeAsString );
  pNode->SetDependentAxesMode( dependentAxesModeAsEnum );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onSecondaryAxisChanged( int )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to change secondary axis, no parameter node/scene found." );
    return;
  }
  
  std::string secondaryAxisAsString = d->advancedRotationSecondaryAxisComboBox->currentText().toStdString();
  int secondaryAxisAsEnum = vtkMRMLTransformProcessorNode::GetAxisLabelFromString( secondaryAxisAsString );
  pNode->SetSecondaryAxisLabel( secondaryAxisAsEnum );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onCopyTranslationChanged()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to change copy translation settings, no parameter node/scene found." );
    return;
  }

  bool copyX = d->advancedTranslationCopyXCheckbox->isChecked();
  pNode->SetCopyTranslationX( copyX );
  bool copyY = d->advancedTranslationCopyYCheckbox->isChecked();
  pNode->SetCopyTranslationY( copyY );
  bool copyZ = d->advancedTranslationCopyZCheckbox->isChecked();
  pNode->SetCopyTranslationZ( copyZ );
}

//-----------------------------------------------------------------------------
bool qSlicerTransformProcessorModuleWidget::eventFilter( QObject * obj, QEvent *event )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  if ( event->type() == QEvent::Show && d->updateButton != NULL && obj == d->updateButton->menu() )
  {
    // Show UpdateButton's menu aligned to the right side of the button
    QMenu* menu = d->updateButton->menu();
    QPoint menuPos = menu->pos();
    menu->move( menuPos.x() + d->updateButton->geometry().width() - menu->geometry().width(), menuPos.y() );
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onParameterNodeChanged()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  qvtkReconnect( d->TransformProcessorNode, pNode, vtkCommand::ModifiedEvent, this, SLOT( updateGUIFromMRML() ) );
  d->TransformProcessorNode = pNode;
  this->updateGUIFromMRML();
}
