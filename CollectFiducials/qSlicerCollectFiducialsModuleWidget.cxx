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
#include <QDebug>

// SlicerQt includes
#include "qSlicerCollectFiducialsModuleWidget.h"
#include "ui_qSlicerCollectFiducialsModule.h"

// module includes
#include "vtkSlicerCollectFiducialsLogic.h"
#include "vtkMRMLCollectFiducialsNode.h"

// slicer includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CollectFiducials
class qSlicerCollectFiducialsModuleWidgetPrivate: public Ui_qSlicerCollectFiducialsModule
{
  Q_DECLARE_PUBLIC( qSlicerCollectFiducialsModuleWidget ); 
  
protected:
  qSlicerCollectFiducialsModuleWidget* const q_ptr;
public:
  qSlicerCollectFiducialsModuleWidgetPrivate( qSlicerCollectFiducialsModuleWidget& object );
  vtkSlicerCollectFiducialsLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerCollectFiducialsModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerCollectFiducialsModuleWidgetPrivate::qSlicerCollectFiducialsModuleWidgetPrivate( qSlicerCollectFiducialsModuleWidget& object )
: q_ptr( &object )
{
}

//-----------------------------------------------------------------------------
vtkSlicerCollectFiducialsLogic* qSlicerCollectFiducialsModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerCollectFiducialsModuleWidget );
  return vtkSlicerCollectFiducialsLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerCollectFiducialsModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerCollectFiducialsModuleWidget::qSlicerCollectFiducialsModuleWidget( QWidget* _parent )
: Superclass( _parent ), d_ptr( new qSlicerCollectFiducialsModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerCollectFiducialsModuleWidget::~qSlicerCollectFiducialsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::setup()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  d->setupUi( this );
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ParameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeSelected() ) );
  connect( d->ProbeTransformNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onProbeTransformNodeSelected() ) );
  connect( d->OutputNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onOutputNodeSelected() ) );
  connect( d->LabelBaseLineEdit, SIGNAL( editingFinished() ), this, SLOT( onLabelBaseChanged() ) );
  connect( d->LabelCounterSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( onLabelCounterChanged() ) );
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordClicked() ) );
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::enter()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  this->Superclass::enter();

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default parameter node.
  if ( d->ParameterNodeComboBox->currentNode() == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLCollectFiducialsNode" );
    // Create a new parameter node if there is none in the scene.
    if ( node == NULL )
    {
      node = this->mrmlScene()->AddNewNodeByClass( "vtkMRMLCollectFiducialsNode" );
    }
    if ( node == NULL )
    {
      qCritical( "Failed to create module node" );
      return;
    }
    d->ParameterNodeComboBox->setCurrentNode( node );
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::exit()
{
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::blockAllSignals( bool block )
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  d->ParameterNodeComboBox->blockSignals( block );
  d->ProbeTransformNodeComboBox->blockSignals( block );
  d->OutputNodeComboBox->blockSignals( block );
  d->LabelBaseLineEdit->blockSignals( block );
  d->LabelCounterSpinBox->blockSignals( block );
  d->RecordButton->blockSignals( block );
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::enableAllWidgets( bool enable )
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  // don't ever disable parameter node, need to be able to select between them
  d->ProbeTransformNodeComboBox->setEnabled( enable );
  d->OutputNodeComboBox->setEnabled( enable );
  d->LabelBaseLineEdit->setEnabled( enable );
  d->LabelCounterSpinBox->setEnabled( enable );
  d->RecordButton->setEnabled( enable );
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportedEvent() ) );
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onSceneImportedEvent()
{
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onParameterNodeSelected()
{
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onProbeTransformNodeSelected()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );

  vtkMRMLCollectFiducialsNode* parameterNode = vtkMRMLCollectFiducialsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  vtkMRMLLinearTransformNode* probeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->ProbeTransformNodeComboBox->currentNode() );
  if( probeTransformNode != NULL )
  {
    parameterNode->SetAndObserveProbeTransformNodeId( probeTransformNode->GetID() );
  }
  else
  {
    parameterNode->SetAndObserveProbeTransformNodeId( NULL );
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onOutputNodeSelected()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );

  vtkMRMLCollectFiducialsNode* parameterNode = vtkMRMLCollectFiducialsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  vtkMRMLMarkupsFiducialNode* outputNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->OutputNodeComboBox->currentNode() );
  
  if( outputNode != NULL )
  {
    parameterNode->SetOutputNodeId( outputNode->GetID() );
  }
  else
  {
    parameterNode->SetAndObserveProbeTransformNodeId( NULL );
  }
  
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onLabelBaseChanged()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );

  vtkMRMLCollectFiducialsNode* parameterNode = vtkMRMLCollectFiducialsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  parameterNode->SetLabelBase( d->LabelBaseLineEdit->text().toStdString() );
  
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onLabelCounterChanged()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );

  vtkMRMLCollectFiducialsNode* parameterNode = vtkMRMLCollectFiducialsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  parameterNode->SetLabelCounter( d->LabelCounterSpinBox->value() );
  
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::onRecordClicked()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  
  vtkMRMLCollectFiducialsNode* parameterNode = vtkMRMLCollectFiducialsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  d->logic()->AddPoint( parameterNode );

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModuleWidget::updateGUIFromMRML()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );

  vtkMRMLCollectFiducialsNode* parameterNode = vtkMRMLCollectFiducialsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    // should not be able to change parameters in a non-existant node...
    this->enableAllWidgets( false );
    return;
  }

  // if a valid node is selected, then the various parameter widgets should be enabled
  this->enableAllWidgets( true );

  // temporarily block signals so nothing gets triggered when updating the GUI
  this->blockAllSignals( true );

  d->ProbeTransformNodeComboBox->setCurrentNode( parameterNode->GetProbeTransformNode() );
  d->OutputNodeComboBox->setCurrentNode( parameterNode->GetOutputNode() );
  d->LabelCounterSpinBox->setValue( parameterNode->GetLabelCounter() );
  d->LabelBaseLineEdit->setText( parameterNode->GetLabelBase().c_str() );

  this->blockAllSignals( false );
}
