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
#include "qSlicerBreachWarningModuleWidget.h"
#include "ui_qSlicerBreachWarningModule.h"

#include "vtkSlicerBreachWarningLogic.h"

#include "vtkMRMLBreachWarningNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_BreachWarning
class qSlicerBreachWarningModuleWidgetPrivate: public Ui_qSlicerBreachWarningModule
{
  Q_DECLARE_PUBLIC( qSlicerBreachWarningModuleWidget ); 
  
protected:
  qSlicerBreachWarningModuleWidget* const q_ptr;
public:
  qSlicerBreachWarningModuleWidgetPrivate( qSlicerBreachWarningModuleWidget& object );
  vtkSlicerBreachWarningLogic* logic() const;

  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerBreachWarningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerBreachWarningModuleWidgetPrivate::qSlicerBreachWarningModuleWidgetPrivate( qSlicerBreachWarningModuleWidget& object ) 
  : q_ptr( &object )
  , ModuleWindowInitialized( false )
{
}

//-----------------------------------------------------------------------------
vtkSlicerBreachWarningLogic* qSlicerBreachWarningModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerBreachWarningModuleWidget );
  return vtkSlicerBreachWarningLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerBreachWarningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerBreachWarningModuleWidget::qSlicerBreachWarningModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerBreachWarningModuleWidgetPrivate( *this ) )
{

}

//-----------------------------------------------------------------------------
qSlicerBreachWarningModuleWidget::~qSlicerBreachWarningModuleWidget()
{
 Q_D(qSlicerBreachWarningModuleWidget);
  disconnect( d->ParameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeChanged() ) );

  // Make connections to update the mrml from the widget
  disconnect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );
  disconnect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolTransformChanged() ) );
  disconnect( d->WarningColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateWarningColor( QColor ) ) );
  disconnect(d->SoundCheckBox, SIGNAL(toggled(bool)), this, SLOT(PlayWarningSound(bool)));
  disconnect(d->WarningCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayWarningColor(bool)));
  disconnect(d->RulerCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayRuler(bool)));
  disconnect( d->RulerColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateRulerColor( QColor ) ) );  
  disconnect( d->RulerTextSizeSlider, SIGNAL( valueChanged (double ) ), this, SLOT( RulerTextSizeChanged( double ) ) );  
  disconnect( d->RulerSizeSlider, SIGNAL( valueChanged (double ) ), this, SLOT( RulerSizeChanged( double ) ) );  
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::setup()
{
  Q_D(qSlicerBreachWarningModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ParameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeChanged() ) );

  // Make connections to update the mrml from the widget
  connect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );
  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolTransformChanged() ) );
  connect( d->WarningColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateWarningColor( QColor ) ) );
  connect(d->SoundCheckBox, SIGNAL(toggled(bool)), this, SLOT(PlayWarningSound(bool)));
  connect(d->WarningCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayWarningColor(bool)));
  connect(d->RulerCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayRuler(bool)));
  connect( d->RulerColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateRulerColor( QColor ) ) );
  connect( d->RulerTextSizeSlider, SIGNAL( valueChanged (double ) ), this, SLOT( RulerTextSizeChanged( double ) ) );  
  connect( d->RulerSizeSlider, SIGNAL( valueChanged (double ) ), this, SLOT( RulerSizeChanged( double ) ) );  

  this->UpdateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::enter()
{
  Q_D(qSlicerBreachWarningModuleWidget);

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }
  
  // Create a module MRML node if there is none in the scene.

  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLBreachWarningNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLBreachWarningNode > newNode = vtkSmartPointer< vtkMRMLBreachWarningNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLBreachWarningNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node" );
    return;
  }

  // For convenience, select a default module.

  if ( d->ParameterNodeComboBox->currentNode() == NULL )
  {
    d->ParameterNodeComboBox->setCurrentNodeID( node->GetID() );
  }

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerBreachWarningModuleWidget );
  this->Superclass::setMRMLScene( scene );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::onSceneImportedEvent()
{
  this->enter();
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::onParameterNodeChanged()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  this->UpdateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::onModelNodeChanged()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( bwNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  
  vtkMRMLModelNode* currentNode = vtkMRMLModelNode::SafeDownCast(d->ModelNodeComboBox->currentNode());
  d->logic()->SetWatchedModelNode( currentNode, bwNode );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::onToolTransformChanged()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Transform node should not be changed when no parameter node selected" );
    return;
  }
  
  vtkMRMLTransformNode* currentNode = vtkMRMLTransformNode::SafeDownCast(d->ToolComboBox->currentNode());
  parameterNode->SetAndObserveToolTransformNodeId( (currentNode!=NULL) ? currentNode->GetID() : NULL);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::PlayWarningSound(bool playWarningSound)
{
  Q_D(qSlicerBreachWarningModuleWidget);
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "No module node selected" );
    return;
  }
  parameterNode->SetPlayWarningSound(playWarningSound);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::DisplayWarningColor(bool displayWarningColor)
{
  Q_D(qSlicerBreachWarningModuleWidget);
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "No module node selected" );
    return;
  }
  parameterNode->SetDisplayWarningColor(displayWarningColor);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::DisplayRuler(bool displayRuler)
{
  Q_D(qSlicerBreachWarningModuleWidget);
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "No module node selected" );
    return;
  }
  parameterNode->SetDisplayRuler(displayRuler);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::UpdateWarningColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Color selected without module node" );
    return;
  }

  parameterNode->SetWarningColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::UpdateRulerColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Color selected without module node" );
    return;
  }

  parameterNode->SetRulerColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::RulerTextSizeChanged( double size )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "No module node selected" );
    return;
  }

  parameterNode->SetRulerTextSize( size );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::RulerSizeChanged( double thickness )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "No module node selected" );
    return;
  }

  parameterNode->SetRulerSize( thickness );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::UpdateFromMRMLNode()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( bwNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ModelNodeComboBox->setCurrentNodeID( "" );
    d->ModelNodeComboBox->setEnabled( false );
    d->ToolComboBox->setEnabled( false );
    d->WarningCheckBox->setEnabled( false );
    d->WarningColorPickerButton->setEnabled( false );
    d->SoundCheckBox->setEnabled( false );        
    d->RulerCheckBox->setEnabled( false );
    d->RulerColorPickerButton->setEnabled( false );
    d->RulerTextSizeSlider->setEnabled( false );
    d->RulerSizeSlider->setEnabled( false );
    return;
  }
    
  d->ModelNodeComboBox->setEnabled( true );
  d->ToolComboBox->setEnabled( true );
  d->WarningCheckBox->setEnabled( true );
  d->WarningColorPickerButton->setEnabled( true );
  d->SoundCheckBox->setEnabled( true );
  d->RulerCheckBox->setEnabled( true );
  d->RulerColorPickerButton->setEnabled( true );
  d->RulerTextSizeSlider->setEnabled( true );
  d->RulerSizeSlider->setEnabled( true );

  d->ToolComboBox->setCurrentNode( bwNode->GetToolTransformNode() );
  d->ModelNodeComboBox->setCurrentNode( bwNode->GetWatchedModelNode() );
  
  double* warningColor = bwNode->GetWarningColor();
  QColor nodeWarningColor;
  nodeWarningColor.setRgbF(warningColor[0],warningColor[1],warningColor[2]);
  d->WarningColorPickerButton->setColor(nodeWarningColor);

  double* rulerColor = bwNode->GetRulerColor();
  QColor nodeRulerColor;
  nodeRulerColor.setRgbF(rulerColor[0],rulerColor[1],rulerColor[2]);
  d->RulerColorPickerButton->setColor(nodeRulerColor);

  d->WarningCheckBox->setChecked( bwNode->GetDisplayWarningColor() );
  d->SoundCheckBox->setChecked( bwNode->GetPlayWarningSound() );  
  d->RulerCheckBox->setChecked( bwNode->GetDisplayRuler() );
  d->RulerTextSizeSlider->setValue( bwNode->GetRulerTextSize() );
  d->RulerSizeSlider->setValue( bwNode->GetRulerSize() );
}
