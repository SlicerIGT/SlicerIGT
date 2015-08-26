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
  disconnect(d->DistanceCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayDistance(bool)));
  disconnect( d->DistanceColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateDistanceColor( QColor ) ) );
  disconnect(d->TrajectoryCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayTrajectory(bool)));
  disconnect( d->TrajectoryColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateTrajectoryColor( QColor ) ) );
  disconnect(d->CrosshairCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayCrossHair(bool)));
  disconnect( d->CrossHairColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateCrossHairColor( QColor ) ) );
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
  connect(d->DistanceCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayDistance(bool)));
  connect( d->DistanceColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateDistanceColor( QColor ) ) );
  connect(d->TrajectoryCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayTrajectory(bool)));
  connect( d->TrajectoryColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateTrajectoryColor( QColor ) ) );
  connect(d->CrosshairCheckBox, SIGNAL(toggled(bool)), this, SLOT(DisplayCrossHair(bool)));
  connect( d->CrossHairColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateCrossHairColor( QColor ) ) );

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
    qCritical( "Transform node should not be changed when no module node selected" );
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
    qCritical( "Transform node should not be changed when no module node selected" );
    return;
  }
  parameterNode->SetDisplayWarningColor(displayWarningColor);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::DisplayTrajectory(bool displayTrajectory)
{
  Q_D(qSlicerBreachWarningModuleWidget);
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Transform node should not be changed when no module node selected" );
    return;
  }
  parameterNode->SetDisplayTrajectory(displayTrajectory);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::DisplayCrossHair(bool displayCrossHair)
{
  Q_D(qSlicerBreachWarningModuleWidget);
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Transform node should not be changed when no module node selected" );
    return;
  }
  parameterNode->SetDisplayCrossHair(displayCrossHair);
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::DisplayDistance(bool displayDistance)
{
  Q_D(qSlicerBreachWarningModuleWidget);
  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Transform node should not be changed when no module node selected" );
    return;
  }
  parameterNode->SetDisplayDistance(displayDistance);
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
void qSlicerBreachWarningModuleWidget::UpdateTrajectoryColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Color selected without module node" );
    return;
  }

  parameterNode->SetTrajectoryColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::UpdateDistanceColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Color selected without module node" );
    return;
  }

  parameterNode->SetDistanceColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModuleWidget::UpdateCrossHairColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* parameterNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( parameterNode == NULL )
  {
    qCritical( "Color selected without module node" );
    return;
  }

  parameterNode->SetCrossHairColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
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
    d->CrosshairCheckBox->setEnabled( false );
    d->CrossHairColorPickerButton->setEnabled( false );
    d->DistanceCheckBox->setEnabled( false );
    d->DistanceColorPickerButton->setEnabled( false );
    d->TrajectoryCheckBox->setEnabled( false );
    d->TrajectoryColorPickerButton->setEnabled( false );
    return;
  }
    
  d->ModelNodeComboBox->setEnabled( true );
  d->ToolComboBox->setEnabled( true );
  d->WarningCheckBox->setEnabled( true );
  d->WarningColorPickerButton->setEnabled( true );
  d->SoundCheckBox->setEnabled( true );
  d->CrosshairCheckBox->setEnabled( true );
  d->CrossHairColorPickerButton->setEnabled( true );
  d->DistanceCheckBox->setEnabled( true );
  d->DistanceColorPickerButton->setEnabled( true );
  d->TrajectoryCheckBox->setEnabled( true );
  d->TrajectoryColorPickerButton->setEnabled( true );

  d->ToolComboBox->setCurrentNode( bwNode->GetToolTransformNode() );
  d->ModelNodeComboBox->setCurrentNode( bwNode->GetWatchedModelNode() );
  
  double* warningColor = bwNode->GetWarningColor();
  QColor nodeWarningColor;
  nodeWarningColor.setRgbF(warningColor[0],warningColor[1],warningColor[2]);
  d->WarningColorPickerButton->setColor(nodeWarningColor);

  double* crossHairColor = bwNode->GetCrossHairColor();
  QColor nodeCrossHairColor;
  nodeCrossHairColor.setRgbF(crossHairColor[0],crossHairColor[1],crossHairColor[2]);
  d->CrossHairColorPickerButton->setColor(nodeCrossHairColor);

  double* trajectoryColor = bwNode->GetTrajectoryColor();
  QColor nodeTrajectoryColor;
  nodeTrajectoryColor.setRgbF(trajectoryColor[0],trajectoryColor[1],trajectoryColor[2]);
  d->TrajectoryColorPickerButton->setColor(nodeTrajectoryColor);

  double* distanceColor = bwNode->GetDistanceColor();
  QColor nodeDistanceColor;
  nodeDistanceColor.setRgbF(distanceColor[0],distanceColor[1],distanceColor[2]);
  d->DistanceColorPickerButton->setColor(nodeDistanceColor);

  d->WarningCheckBox->setChecked( bwNode->GetDisplayWarningColor() );
  d->SoundCheckBox->setChecked( bwNode->GetPlayWarningSound() );
  d->CrosshairCheckBox->setChecked( bwNode->GetDisplayCrossHair() );
  d->DistanceCheckBox->setChecked( bwNode->GetDisplayDistance() );
  d->TrajectoryCheckBox->setChecked( bwNode->GetDisplayTrajectory() );
}
