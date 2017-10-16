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
#include "qSlicerTransformFusionModuleWidget.h"
#include "ui_qSlicerTransformFusionModule.h"

// Transform Fusion includes
#include "vtkSlicerTransformFusionLogic.h"
#include "vtkMRMLTransformFusionNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLLinearTransformNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformFusion
class qSlicerTransformFusionModuleWidgetPrivate: public Ui_qSlicerTransformFusionModule
{
  Q_DECLARE_PUBLIC( qSlicerTransformFusionModuleWidget ); 
  
protected:
  qSlicerTransformFusionModuleWidget* const q_ptr;
public:
  qSlicerTransformFusionModuleWidgetPrivate( qSlicerTransformFusionModuleWidget& object );
  vtkSlicerTransformFusionLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerTransformFusionModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerTransformFusionModuleWidgetPrivate::qSlicerTransformFusionModuleWidgetPrivate( qSlicerTransformFusionModuleWidget& object ) : q_ptr( &object )
{
}

vtkSlicerTransformFusionLogic* qSlicerTransformFusionModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTransformFusionModuleWidget );
  return vtkSlicerTransformFusionLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerTransformFusionModuleWidget methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
qSlicerTransformFusionModuleWidget::qSlicerTransformFusionModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerTransformFusionModuleWidgetPrivate( *this ) )
{
  this->updateTimer = new QTimer();
  updateTimer->setSingleShot( false );
  updateTimer->setInterval( 17 ); // default is once every sixtieth of a second, = 17 milliseconds
}

//-----------------------------------------------------------------------------
qSlicerTransformFusionModuleWidget::~qSlicerTransformFusionModuleWidget()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  disconnect( d->parameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( setTransformFusionParametersNode( vtkMRMLNode* ) ) );
  
  disconnect( d->inputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onSingleInputTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->outputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onOutputTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->restingTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onRestingTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->referenceTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onReferenceTransformNodeSelected( vtkMRMLNode* ) ) );
  disconnect( d->addInputTransformButton, SIGNAL( clicked() ), this, SLOT( onAddInputTransform() ) );
  disconnect( d->removeInputTransformButton, SIGNAL( clicked() ), this, SLOT( onRemoveInputTransform() ) );
  
  disconnect( d->fusionModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onFusionModeChanged( int ) ) );

  disconnect( d->updateButton, SIGNAL( clicked() ), this, SLOT( onUpdateButtonPressed() ) );
  disconnect( d->updateRateSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( setUpdatesPerSecond( double ) ) );
  disconnect( updateTimer, SIGNAL( timeout() ), this, SLOT( handleEventTimedUpdate() ) );
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setup()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  d->setupUi( this );
  this->Superclass::setup();

  // Setup Update button menu
  // Install event filter to override menu position to show it on the right side of the button.
  // This is necessary because the update button is very wide and the
  // menu arrow is on the right side. With the default QMenu the menu would appear
  // on the left side, which would be very inconvenient because the mouse would need
  // to be moved a lot to click the manual/auto option.
  QMenu* updateMenu = new QMenu( tr( "Update options" ), this );
  updateMenu->installEventFilter( this );
  updateMenu->setObjectName( "UpdateOptions" );
  QActionGroup* updateActions = new QActionGroup( d->updateButton );
  updateActions->setExclusive( true );
  updateMenu->addAction( d->actionUpdateManual );
  updateActions->addAction( d->actionUpdateManual );
  QObject::connect( d->actionUpdateManual, SIGNAL( triggered() ), this, SLOT( onActionUpdateManual() ) );
  updateMenu->addAction( d->actionUpdateAuto );
  updateActions->addAction( d->actionUpdateAuto );
  QObject::connect( d->actionUpdateAuto, SIGNAL( triggered() ), this, SLOT( onActionUpdateAuto() ) );
  updateMenu->addAction( d->actionUpdateTimed );
  updateActions->addAction( d->actionUpdateTimed );
  QObject::connect( d->actionUpdateTimed, SIGNAL( triggered() ), this, SLOT( onActionUpdateTimed() ) );
  d->updateButton->setMenu(updateMenu);

  // fusion modes
  d->fusionModeComboBox->addItem( vtkMRMLTransformFusionNode::GetFusionModeAsString( vtkMRMLTransformFusionNode::FUSION_MODE_QUATERNION_AVERAGE ).c_str() );
  d->fusionModeComboBox->setItemData( 0, "Compute the quaternion average of all Input transforms provided.", Qt::ToolTipRole );
  d->fusionModeComboBox->addItem( vtkMRMLTransformFusionNode::GetFusionModeAsString( vtkMRMLTransformFusionNode::FUSION_MODE_CONSTRAIN_SHAFT_ROTATION ).c_str() );
  d->fusionModeComboBox->setItemData( 1, "Compute a constrained version of an Input transform, the shaft direction is preserved but the other axes resemble the Resting coordinate system.", Qt::ToolTipRole );

  // set up connections
  connect( d->parameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( setTransformFusionParametersNode( vtkMRMLNode* ) ) );
  
  connect( d->inputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onSingleInputTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->outputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onOutputTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->restingTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onRestingTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->referenceTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onReferenceTransformNodeSelected( vtkMRMLNode* ) ) );
  connect( d->addInputTransformButton, SIGNAL( clicked() ), this, SLOT( onAddInputTransform() ) );
  connect( d->removeInputTransformButton, SIGNAL( clicked() ), this, SLOT( onRemoveInputTransform() ) );
  
  connect( d->fusionModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onFusionModeChanged( int ) ) );

  connect( d->updateButton, SIGNAL( clicked() ), this, SLOT( onUpdateButtonPressed() ) );
  connect( d->updateRateSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( setUpdatesPerSecond( double ) ) );
  connect( updateTimer, SIGNAL( timeout() ), this, SLOT( handleEventTimedUpdate() ) );

  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( onLogicModified() ) );
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::enter()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  bool wasBlocked = getSignalsBlocked();
  this->setSignalsBlocked( true );
  this->onEnter();
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL )
  {
    qCritical( "Selected parameter node not a valid parameter node" );
    return;
  }
  this->qvtkConnect( pNode, vtkMRMLTransformFusionNode::InputDataModifiedEvent, this, SLOT( handleEventAutoUpdate() ) );
  this->setSignalsBlocked( wasBlocked );
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  bool wasBlocked = getSignalsBlocked();
  this->setSignalsBlocked( true );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportedEvent() ) );
  if ( scene && vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() ) == NULL )
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLTransformFusionNode" );
    if ( node )
    {
      this->setTransformFusionParametersNode( vtkMRMLTransformFusionNode::SafeDownCast( node ) );
    }
  }
  this->setSignalsBlocked( wasBlocked );
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onLogicModified()
{
  this->updateWidget();
  this->updateInputList();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onEnter()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  
  if ( this->mrmlScene() == NULL || d->logic() == NULL )
  {
    qCritical( "Error: Unable to initialize module, no parameter node/scene found." );
    return;
  }

  //Check for existing parameter node
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLTransformFusionNode" );
    if ( node == NULL )
    {
      vtkSmartPointer< vtkMRMLTransformFusionNode > newNode = vtkSmartPointer< vtkMRMLTransformFusionNode >::New();
      this->mrmlScene()->AddNode( newNode );
      d->parameterNodeComboBox->setCurrentNode( newNode );
    }
  }
  
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setSignalsBlocked( bool newBlock )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  d->parameterNodeComboBox->blockSignals( newBlock );
  d->inputTransformComboBox->blockSignals( newBlock );
  d->restingTransformComboBox->blockSignals( newBlock );
  d->referenceTransformComboBox->blockSignals( newBlock );
  d->outputTransformComboBox->blockSignals( newBlock );
  d->fusionModeComboBox->blockSignals( newBlock );
  d->updateRateSpinBox->blockSignals( newBlock );
}

//-----------------------------------------------------------------------------
bool qSlicerTransformFusionModuleWidget::getSignalsBlocked()
{
  Q_D( qSlicerTransformFusionModuleWidget );

  bool parameterNodeBlocked = d->parameterNodeComboBox->signalsBlocked();

  // all signal blocks should be the same as parameterNodeBlocked. If not, then output a warning message.
  if ( parameterNodeBlocked == d->inputTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->restingTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->referenceTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->outputTransformComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->fusionModeComboBox->signalsBlocked() &&
       parameterNodeBlocked == d->updateRateSpinBox->signalsBlocked() )
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
void qSlicerTransformFusionModuleWidget::setTransformFusionParametersNode( vtkMRMLNode *node )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( node );
  qvtkReconnect( pNode, vtkCommand::ModifiedEvent, this, SLOT( update() ) );
  d->parameterNodeComboBox->setCurrentNode( pNode );

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateWidget()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical("Error: Failed to update widget, no parameter node/scene found.");
    return;
  }

  // Populate node selections
  d->inputTransformComboBox->setCurrentNode( pNode->GetSingleInputTransformNode() );
  d->referenceTransformComboBox->setCurrentNode( pNode->GetReferenceTransformNode() );
  d->restingTransformComboBox->setCurrentNode( pNode->GetRestingTransformNode() );
  d->outputTransformComboBox->setCurrentNode( pNode->GetOutputTransformNode() );

  // Parameters
  int currentTechniqueIndex = d->fusionModeComboBox->findText( QString( vtkMRMLTransformFusionNode::GetFusionModeAsString( pNode->GetFusionMode() ).c_str() ) );
  if ( currentTechniqueIndex < 0 )
  {
    currentTechniqueIndex = 0;
  }
  d->fusionModeComboBox->setCurrentIndex( currentTechniqueIndex );

  d->updateRateSpinBox->setValue( pNode->GetUpdatesPerSecond() );

  this->updateInputList();
  this->updateInputFieldVisibility();
  this->updateButtons();
  this->updateRateSpinBoxVisibility();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateInputFieldVisibility()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Unable to update input field visibility, no parameter node/scene found." );
    return;
  }

  if ( pNode->GetFusionMode() == vtkMRMLTransformFusionNode::FUSION_MODE_QUATERNION_AVERAGE )
  {
    d->inputTransformListGroupBox->setVisible( true );
    d->inputTransformLabel->setVisible( false );
    d->inputTransformComboBox->setVisible( false );
    d->restingTransformLabel->setVisible( false );
    d->restingTransformComboBox->setVisible( false );
    d->referenceTransformLabel->setVisible( false );
    d->referenceTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
  }
  else if ( pNode->GetFusionMode() == vtkMRMLTransformFusionNode::FUSION_MODE_CONSTRAIN_SHAFT_ROTATION )
  {
    d->inputTransformListGroupBox->setVisible( false );
    d->inputTransformLabel->setVisible( true );
    d->inputTransformComboBox->setVisible( true );
    d->restingTransformLabel->setVisible( true );
    d->restingTransformComboBox->setVisible( true );
    d->referenceTransformLabel->setVisible( true );
    d->referenceTransformComboBox->setVisible( true );
    d->outputTransformLabel->setVisible( true );
    d->outputTransformComboBox->setVisible( true );
  }
  else
  {
    d->inputTransformListGroupBox->setVisible( false );
    d->inputTransformLabel->setVisible( false );
    d->inputTransformComboBox->setVisible( false );
    d->restingTransformLabel->setVisible( false );
    d->restingTransformComboBox->setVisible( false );
    d->referenceTransformLabel->setVisible( false );
    d->referenceTransformComboBox->setVisible( false );
    d->outputTransformLabel->setVisible( false );
    d->outputTransformComboBox->setVisible( false );
    qWarning( "Warning: Unrecognized fusion mode, setting all fields invisible." );
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateRateSpinBoxVisibility()
{
  Q_D( qSlicerTransformFusionModuleWidget );

  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update timer spinbox, no parameter node/scene found." );
    return;
  }

  if ( pNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_TIMED )
  {
    d->updateRateSpinBox->setVisible( true );
    d->updateRateLabel->setVisible( true );
  }
  else
  {
    d->updateRateSpinBox->setVisible( false );
    d->updateRateLabel->setVisible( false );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateButtons()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update buttons, no parameter node/scene found." );
    return;
  }

  bool conditionsForCurrentFusionMode = d->logic()->IsTransformFusionPossible( pNode );
  if ( conditionsForCurrentFusionMode )
  {
    d->updateButton->setEnabled( true );
  }
  else
  {
    d->updateButton->setEnabled( false );
  }
  
  if ( pNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_MANUAL )
  {
    d->updateButton->setCheckable( false );
    d->updateButton->setChecked( false );
    d->updateButton->setText( vtkMRMLTransformFusionNode::GetUpdateModeAsString( vtkMRMLTransformFusionNode::UPDATE_MODE_MANUAL ).c_str() );
    d->actionUpdateManual->setChecked( Qt::Checked );
    d->actionUpdateAuto->setChecked( Qt::Unchecked );
    d->actionUpdateTimed->setChecked( Qt::Unchecked );
  }
  else if ( pNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_AUTO )
  {
    d->updateButton->setCheckable( true );
    d->updateButton->setChecked( true );
    d->updateButton->setText( vtkMRMLTransformFusionNode::GetUpdateModeAsString( vtkMRMLTransformFusionNode::UPDATE_MODE_AUTO ).c_str() );
    d->actionUpdateManual->setChecked( Qt::Unchecked );
    d->actionUpdateAuto->setChecked( Qt::Checked );
    d->actionUpdateTimed->setChecked( Qt::Unchecked );
  }
  else if ( pNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_TIMED )
  {
    d->updateButton->setCheckable( true );
    d->updateButton->setChecked( true );
    d->updateButton->setText( vtkMRMLTransformFusionNode::GetUpdateModeAsString( vtkMRMLTransformFusionNode::UPDATE_MODE_TIMED ).c_str() );
    d->actionUpdateManual->setChecked( Qt::Unchecked );
    d->actionUpdateAuto->setChecked( Qt::Unchecked );
    d->actionUpdateTimed->setChecked( Qt::Checked );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateInputList()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update Input List on GUI, no parameter node/scene found." );
    return;
  }

  d->inputTransformList->clear();

  for ( int i = 0; i < pNode->GetNumberOfInputTransformNodes(); i++ )
  {
      new QListWidgetItem( tr( pNode->GetNthInputTransformNode( i )->GetName() ), d->inputTransformList );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onActionUpdateManual()
{
  stopExistingUpdates();
  singleUpdate();
  updateButtons();
  updateRateSpinBoxVisibility();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onActionUpdateAuto()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  stopExistingUpdates();

  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to start auto update, no parameter node/scene found." );
    return;
  }
  
  bool verboseConditionChecking = true;
  bool conditionsMetForFusion = d->logic()->IsTransformFusionPossible( pNode, verboseConditionChecking );
  if ( conditionsMetForFusion == true )
  {
    pNode->SetUpdateMode( vtkMRMLTransformFusionNode::UPDATE_MODE_AUTO );
  }
  updateButtons();
  updateRateSpinBoxVisibility();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onActionUpdateTimed()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  stopExistingUpdates();

  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to start timed update, no parameter node/scene found." );
    return;
  }
  
  // if the timer is for some reason already going, stop it before restarting
  if ( updateTimer->isActive() )
  {
    updateTimer->stop();
  }

  // don't start timed updates unless the conditions for updating are met
  bool verboseConditionChecking = true;
  bool conditionsMetForFusion = d->logic()->IsTransformFusionPossible( pNode, verboseConditionChecking );
  if ( conditionsMetForFusion == true )
  {
    updateTimer->start();
    pNode->SetUpdateMode( vtkMRMLTransformFusionNode::UPDATE_MODE_TIMED );
  }
  updateButtons();
  updateRateSpinBoxVisibility();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::stopExistingUpdates()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to stop updates, no parameter node/scene found." );
    return;
  }

  // set to manual updating by default, avoids automatic updates
  pNode->SetUpdateMode( vtkMRMLTransformFusionNode::UPDATE_MODE_MANUAL );

  if ( updateTimer->isActive() )
  {
    updateTimer->stop();
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onUpdateButtonPressed()
{
  singleUpdate();
  updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::singleUpdate()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update, no parameter node/scene found." );
    return;
  }

  bool verboseConditionChecking = true;
  bool conditionsMetForFusion = d->logic()->IsTransformFusionPossible( pNode, verboseConditionChecking );
  if (conditionsMetForFusion == true)
  {
    d->logic()->UpdateOutputTransform( pNode );
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::handleEventAutoUpdate()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to auto-update, no parameter node/scene found." );
    return;
  }

  if ( pNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_AUTO )
  {
    bool verboseConditionChecking = true;
    if (d->logic()->IsTransformFusionPossible(pNode,verboseConditionChecking) == true)
    {
      singleUpdate();
    }
    else
    {
      // if conditions are not met, then auto updating should be disabled
      pNode->SetUpdateMode( vtkMRMLTransformFusionNode::UPDATE_MODE_MANUAL );
      updateButtons();
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::handleEventTimedUpdate()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to timed update, no parameter node/scene found." );
    return;
  }
  
  bool verboseWarnings = false;
  bool conditionsMetForFusion = d->logic()->IsTransformFusionPossible( pNode, verboseWarnings );
  if ( pNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_TIMED &&
       conditionsMetForFusion == true )
  {
    singleUpdate();
  }
  else
  {
    // if the update mode is not timed, or
    // if conditions are not met, then timed updating should be disabled
    pNode->SetUpdateMode( vtkMRMLTransformFusionNode::UPDATE_MODE_MANUAL );
    updateButtons();
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setUpdatesPerSecond( double updatesPerSecond )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to update timer for timed update, no parameter node/scene found." );
    return;
  }

  pNode->SetUpdatesPerSecond( updatesPerSecond );
  updateTimer->setInterval( ( 1.0 / updatesPerSecond ) * 1000 );
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onOutputTransformNodeSelected( vtkMRMLNode* node )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to set output transform node, no parameter node/scene found." );
    return;
  }

  vtkMRMLLinearTransformNode* outputTransform = NULL;
  if ( node )
  {
    outputTransform = vtkMRMLLinearTransformNode::SafeDownCast( node );
    if ( outputTransform == NULL )
    {
      qCritical( "Error: Failed to set output transform node, output transform node is not a linear transform node" );
      return;    
    }
  }

  pNode->SetAndObserveOutputTransformNode( outputTransform );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onSingleInputTransformNodeSelected( vtkMRMLNode* node )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to set single input transform node, no parameter node/scene found." );
    return;
  }

  vtkMRMLLinearTransformNode* inputTransform = NULL;
  if ( node )
  {
    inputTransform = vtkMRMLLinearTransformNode::SafeDownCast( node );
    if ( inputTransform == NULL )
    {
      qCritical( "Error: Failed to set single input transform node, input transform node is not a linear transform node." );
      return;    
    }
  }

  pNode->SetAndObserveSingleInputTransformNode( inputTransform );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onReferenceTransformNodeSelected( vtkMRMLNode* node )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to set reference transform node, no parameter node/scene found." );
    return;
  }

  vtkMRMLLinearTransformNode* referenceTransform = NULL;
  if ( node )
  {
    referenceTransform = vtkMRMLLinearTransformNode::SafeDownCast( node );
    if ( referenceTransform == NULL )
    {
      qCritical( "Error: Failed to set reference transform node, reference transform node is not a linear transform node" );
      return;
    }
  }

  pNode->SetAndObserveReferenceTransformNode( referenceTransform );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onRestingTransformNodeSelected( vtkMRMLNode* node )
{
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to set resting transform node, no parameter node/scene found." );
    return;
  }

  vtkMRMLLinearTransformNode* restingTransform = NULL;
  if ( node )
  {
    restingTransform = vtkMRMLLinearTransformNode::SafeDownCast( node );
    if ( restingTransform == NULL )
    {
      qCritical( "Error: Failed to set resting transform node, resting transform node is not a linear transform node" );
      return;
    }
  }

  pNode->SetAndObserveRestingTransformNode( restingTransform );
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onAddInputTransform()
{
  Q_D( qSlicerTransformFusionModuleWidget );

  if ( d->addInputTransformComboBox->currentNode() == NULL )
  {
    return; // don't do anything if no input node is selected
  }
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to add transform to input list, no parameter node/scene found." );
    return;
  }

  vtkMRMLLinearTransformNode* inputTransform = vtkMRMLLinearTransformNode::SafeDownCast( d->addInputTransformComboBox->currentNode() );
  if ( inputTransform == NULL )
  {
    qCritical( "Error: Failed to add transform to input list, input transform node is not a linear transform node." );
    return;
  }

  pNode->AddAndObserveInputTransformNode( inputTransform );
  this->updateInputList();
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onFusionModeChanged( int unused )
{
  Q_UNUSED(unused);
  Q_D( qSlicerTransformFusionModuleWidget );
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    return;
  }

  std::string fusionModeAsString = d->fusionModeComboBox->currentText().toStdString();
  int fusionModeAsEnum = vtkMRMLTransformFusionNode::GetFusionModeFromString( fusionModeAsString );
  pNode->SetFusionMode( fusionModeAsEnum );

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onRemoveInputTransform()
{
  Q_D( qSlicerTransformFusionModuleWidget );
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( d->parameterNodeComboBox->currentNode() );
  if ( pNode == NULL || this->mrmlScene() == NULL )
  {
    qCritical( "Error: Failed to remove transform from input list, no parameter node/scene found." );
    return;
  }

  int selectedInputListIndex = d->inputTransformList->currentRow();

  if ( selectedInputListIndex >= 0 && selectedInputListIndex < pNode->GetNumberOfInputTransformNodes() )
  {
    pNode->RemoveInputTransformNode( selectedInputListIndex );
    this->updateInputList();
    this->updateButtons();
  }
}

//-----------------------------------------------------------------------------
bool qSlicerTransformFusionModuleWidget::eventFilter( QObject * obj, QEvent *event )
{
  Q_D( qSlicerTransformFusionModuleWidget );
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
