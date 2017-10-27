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
  Q_D( qSlicerTransformProcessorModuleWidget );
  disconnect( d->parameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( setTransformProcessorParametersNode( vtkMRMLNode* ) ) );
  
  disconnect( d->addInputCombineTransformButton, SIGNAL( clicked() ), this, SLOT( onAddInputCombineTransform() ) );
  disconnect( d->removeInputCombineTransformButton, SIGNAL( clicked() ), this, SLOT( onRemoveInputCombineTransform() ) );
  disconnect( d->inputFromTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputFromTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->inputToTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputToTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->inputInitialTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputInitialTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->inputChangedTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputChangedTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->inputAnchorTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputAnchorTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->outputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onResultTransformNodeSelected( vtkMRMLNode* ) ) );
  
  disconnect( d->processingModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onProcessingModeChanged( int ) ) );

  disconnect( d->advancedRotationModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onRotationModeChanged( int ) ) );
  disconnect( d->advancedRotationPrimaryAxisComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onPrimaryAxisChanged( int ) ) );
  disconnect( d->advancedRotationDependentAxesModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onDependentAxesModeChanged( int ) ) );
  disconnect( d->advancedRotationSecondaryAxisComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onSecondaryAxisChanged( int ) ) );

  disconnect( d->updateButton, SIGNAL( clicked() ), this, SLOT( onUpdateButtonPressed() ) );
  disconnect( d->updateButton, SIGNAL( checkBoxToggled( bool ) ), this, SLOT( onUpdateButtonCheckboxToggled( bool ) ) );
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
  d->processingModeComboBox->addItem( vtkMRMLTransformProcessorNode::GetProcessingModeAsString( vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT ).c_str() );
  d->processingModeComboBox->setItemData( 4, "Compute a constrained version of an Source transform, the translation and z direction are preserved but the other axes resemble the Target coordinate system.", Qt::ToolTipRole );

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

  // set up connections
  connect( d->parameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( setTransformProcessorParametersNode( vtkMRMLNode* ) ) );
  
  connect( d->inputFromTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputFromTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputToTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputToTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputInitialTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputInitialTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputChangedTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputChangedTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->inputAnchorTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onInputAnchorTransformNodeSelected( vtkMRMLNode* ) ) );
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

  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( onLogicModified() ) );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::enter()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  bool wasBlocked = getSignalsBlocked();
  this->setSignalsBlocked( true );
  this->onEnter();
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL )
  {
    qCritical( "Selected parameter node not a valid parameter node" );
    return;
  }
  this->qvtkConnect( pNode, vtkMRMLTransformProcessorNode::InputDataModifiedEvent, this, SLOT( handleEventAutoUpdate() ) );
  this->setSignalsBlocked( wasBlocked );
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  bool wasBlocked = getSignalsBlocked();
  this->setSignalsBlocked( true );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportedEvent() ) );
  if ( scene && vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() ) == NULL )
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLTransformProcessorNode" );
    if ( node )
    {
      this->setTransformProcessorParametersNode( vtkMRMLTransformProcessorNode::SafeDownCast( node ) );
    }
  }
  this->setSignalsBlocked( wasBlocked );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onLogicModified()
{
  this->updateWidget();
  this->updateInputCombineList();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onEnter()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  
  if ( this->mrmlScene() == NULL || d->logic() == NULL )
  {
    qCritical( "Error: Unable to initialize module, no parameter node/scene found." );
    return;
  }

  //Check for existing parameter node
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLTransformProcessorNode" );
    if ( node == NULL )
    {
      vtkSmartPointer< vtkMRMLTransformProcessorNode > newNode = vtkSmartPointer< vtkMRMLTransformProcessorNode >::New();
      this->mrmlScene()->AddNode( newNode );
      d->parameterNodeComboBox->setCurrentNode( newNode );
    }
  }
  
  this->updateWidget();
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
void qSlicerTransformProcessorModuleWidget::setTransformProcessorParametersNode( vtkMRMLNode* node )
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( node );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical("Error: Failed to set transform processor node, input node not a transform processor node.");
    return;
  }
  qvtkReconnect( pNode, vtkCommand::ModifiedEvent, this, SLOT( update() ) );
  d->parameterNodeComboBox->setCurrentNode( pNode );

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::updateWidget()
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

  // Populate node selections
  d->inputFromTransformComboBox->setCurrentNode( pNode->GetInputFromTransformNode() );
  d->inputToTransformComboBox->setCurrentNode( pNode->GetInputToTransformNode() );
  d->inputInitialTransformComboBox->setCurrentNode( pNode->GetInputInitialTransformNode() );
  d->inputChangedTransformComboBox->setCurrentNode( pNode->GetInputChangedTransformNode() );
  d->inputAnchorTransformComboBox->setCurrentNode( pNode->GetInputAnchorTransformNode() );
  d->outputTransformComboBox->setCurrentNode( pNode->GetOutputTransformNode() );

  // Parameters
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

  this->updateInputCombineList();
  this->updateInputFieldVisibility();
  this->updateButtons();

  this->setSignalsBlocked( wasBlocked );
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::updateInputFieldVisibility()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Unable to update input field visibility, no parameter node/scene found." );
    return;
  }

  if ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_QUATERNION_AVERAGE )
  {
    d->inputCombineTransformListGroupBox->setVisible( true );
    d->inputFromTransformLabel->setVisible( false );
    d->inputFromTransformComboBox->setVisible( false );
    d->inputToTransformLabel->setVisible( false );
    d->inputToTransformComboBox->setVisible( false );
    d->inputInitialTransformLabel->setVisible( false );
    d->inputInitialTransformComboBox->setVisible( false );
    d->inputChangedTransformLabel->setVisible( false );
    d->inputChangedTransformComboBox->setVisible( false );
    d->inputAnchorTransformLabel->setVisible( false );
    d->inputAnchorTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
    d->advancedRotationGroupBox->setVisible( false );
    d->advancedTranslationGroupBox->setVisible( false );
  }
  else if ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT )
  {
    d->inputCombineTransformListGroupBox->setVisible( false );
    d->inputFromTransformLabel->setVisible( false );
    d->inputFromTransformComboBox->setVisible( false );
    d->inputToTransformLabel->setVisible( false );
    d->inputToTransformComboBox->setVisible( false );
    d->inputInitialTransformLabel->setVisible( true );
    d->inputInitialTransformComboBox->setVisible( true );
    d->inputChangedTransformLabel->setVisible( true );
    d->inputChangedTransformComboBox->setVisible( true );
    d->inputAnchorTransformLabel->setVisible( true );
    d->inputAnchorTransformComboBox->setVisible( true );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
    d->advancedRotationGroupBox->setVisible( false );
    d->advancedTranslationGroupBox->setVisible( false );
  }
  else if ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_ROTATION )
  {
    d->inputCombineTransformListGroupBox->setVisible( false );
    d->inputFromTransformLabel->setVisible( true );
    d->inputFromTransformComboBox->setVisible( true );
    d->inputToTransformLabel->setVisible( true );
    d->inputToTransformComboBox->setVisible( true );
    d->inputInitialTransformLabel->setVisible( false );
    d->inputInitialTransformComboBox->setVisible( false );
    d->inputChangedTransformLabel->setVisible( false );
    d->inputChangedTransformComboBox->setVisible( false );
    d->inputAnchorTransformLabel->setVisible( false );
    d->inputAnchorTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
    d->advancedRotationGroupBox->setVisible( true );
    d->advancedTranslationGroupBox->setVisible( false );
    if ( pNode->GetRotationMode() == vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_SINGLE_AXIS )
    {
      d->advancedRotationPrimaryAxisLabel->setVisible( true );
      d->advancedRotationPrimaryAxisComboBox->setVisible( true );
      d->advancedRotationDependentAxesModeLabel->setVisible( true );
      d->advancedRotationDependentAxesModeComboBox->setVisible( true );
      if ( pNode->GetDependentAxesMode() == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS )
      {
        d->advancedRotationSecondaryAxisLabel->setVisible( true );
        d->advancedRotationSecondaryAxisComboBox->setVisible( true );
      }
      else
      {
        d->advancedRotationSecondaryAxisLabel->setVisible( false );
        d->advancedRotationSecondaryAxisComboBox->setVisible( false );
      }
    }
    else
    {
      d->advancedRotationPrimaryAxisLabel->setVisible( false );
      d->advancedRotationPrimaryAxisComboBox->setVisible( false );
      d->advancedRotationDependentAxesModeLabel->setVisible( false );
      d->advancedRotationDependentAxesModeComboBox->setVisible( false );
      d->advancedRotationSecondaryAxisLabel->setVisible( false );
      d->advancedRotationSecondaryAxisComboBox->setVisible( false );
    }
  }
  else if ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_TRANSLATION )
  {
    d->inputCombineTransformListGroupBox->setVisible( false );
    d->inputFromTransformLabel->setVisible( true );
    d->inputFromTransformComboBox->setVisible( true );
    d->inputToTransformLabel->setVisible( true );
    d->inputToTransformComboBox->setVisible( true );
    d->inputInitialTransformLabel->setVisible( false );
    d->inputInitialTransformComboBox->setVisible( false );
    d->inputChangedTransformLabel->setVisible( false );
    d->inputChangedTransformComboBox->setVisible( false );
    d->inputAnchorTransformLabel->setVisible( false );
    d->inputAnchorTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
    d->advancedRotationGroupBox->setVisible( false );
    d->advancedTranslationGroupBox->setVisible( true );
  }
  else if ( pNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM )
  {
    d->inputCombineTransformListGroupBox->setVisible( false );
    d->inputFromTransformLabel->setVisible( true );
    d->inputFromTransformComboBox->setVisible( true );
    d->inputToTransformLabel->setVisible( true );
    d->inputToTransformComboBox->setVisible( true );
    d->inputInitialTransformLabel->setVisible( false );
    d->inputInitialTransformComboBox->setVisible( false );
    d->inputChangedTransformLabel->setVisible( false );
    d->inputChangedTransformComboBox->setVisible( false );
    d->inputAnchorTransformLabel->setVisible( false );
    d->inputAnchorTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
    d->advancedRotationGroupBox->setVisible( false );
    d->advancedTranslationGroupBox->setVisible( false );
  }
  else
  {
    d->inputCombineTransformListGroupBox->setVisible( false );
    d->inputFromTransformLabel->setVisible( false );
    d->inputFromTransformComboBox->setVisible( false );
    d->inputToTransformLabel->setVisible( false );
    d->inputToTransformComboBox->setVisible( false );
    d->inputInitialTransformLabel->setVisible( false );
    d->inputInitialTransformComboBox->setVisible( false );
    d->inputChangedTransformLabel->setVisible( false );
    d->inputChangedTransformComboBox->setVisible( false );
    d->inputAnchorTransformLabel->setVisible( false );
    d->inputAnchorTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
    d->advancedRotationGroupBox->setVisible( false );
    d->advancedTranslationGroupBox->setVisible( false );
    qWarning( "Warning: Unrecognized processing mode, setting all fields invisible." );
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::updateButtons()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update buttons, no parameter node/scene found." );
    return;
  }

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
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::updateInputCombineList()
{
  Q_D( qSlicerTransformProcessorModuleWidget );
  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update Input List on GUI, no parameter node/scene found." );
    return;
  }

  d->inputCombineTransformList->clear();
  for ( int i = 0; i < pNode->GetNumberOfInputCombineTransformNodes(); i++ )
  {
    new QListWidgetItem( tr( pNode->GetNthInputCombineTransformNode( i )->GetName() ), d->inputCombineTransformList );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onUpdateButtonPressed()
{
  singleUpdate();
  updateButtons();
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

  updateButtons();
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
      singleUpdate();
    }
    else
    {
      qWarning( "Stopping automatic update - fix afore-mentioned problems before turning it back on." );
      pNode->SetUpdateModeToManual();
      updateButtons();
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputFromTransformNodeSelected( vtkMRMLNode* node )
{
  SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_FROM );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputToTransformNodeSelected( vtkMRMLNode* node )
{
  SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_TO );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputInitialTransformNodeSelected( vtkMRMLNode* node )
{
  SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_INITIAL );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputChangedTransformNodeSelected( vtkMRMLNode* node )
{
  SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_CHANGED );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onInputAnchorTransformNodeSelected( vtkMRMLNode* node )
{
  SetTransformAccordingToRole( node, TRANSFORM_ROLE_INPUT_ANCHOR );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModuleWidget::onOutputTransformNodeSelected( vtkMRMLNode* node )
{
  SetTransformAccordingToRole( node, TRANSFORM_ROLE_OUTPUT );
  this->updateButtons();
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
  this->updateInputCombineList();
  this->updateButtons();
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
    this->updateInputCombineList();
    this->updateButtons();
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

  this->updateWidget();
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

  this->updateWidget();
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

  this->updateWidget();
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

  this->updateWidget();
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

  this->updateWidget();
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

  this->updateWidget();
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
