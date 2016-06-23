/*==============================================================================

  Program: 3D Slicer

  Portions ( c ) Copyright Brigham and Women's Hospital ( BWH ) All Rights Reserved.

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

#include "qSlicerApplication.h"

// SlicerQt includes
#include "qSlicerMarkupsToModelModuleWidget.h"
#include "ui_qSlicerMarkupsToModelModuleWidget.h"
#include "qSlicerSimpleMarkupsWidget.h"


#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkSlicerMarkupsToModelLogic.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLInteractionNode.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMarkupsToModelModuleWidgetPrivate: public Ui_qSlicerMarkupsToModelModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerMarkupsToModelModuleWidget ); 

protected:
  qSlicerMarkupsToModelModuleWidget* const q_ptr;
public:
  qSlicerMarkupsToModelModuleWidgetPrivate( qSlicerMarkupsToModelModuleWidget& object );
  vtkSlicerMarkupsToModelLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsToModelModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModuleWidgetPrivate::qSlicerMarkupsToModelModuleWidgetPrivate(  qSlicerMarkupsToModelModuleWidget& object ) : q_ptr( &object )
{
}


//-----------------------------------------------------------------------------
vtkSlicerMarkupsToModelLogic* qSlicerMarkupsToModelModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerMarkupsToModelModuleWidget );
  return vtkSlicerMarkupsToModelLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerMarkupsToModelModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModuleWidget::qSlicerMarkupsToModelModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerMarkupsToModelModuleWidgetPrivate ( *this ) )
{
}


//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModuleWidget::~qSlicerMarkupsToModelModuleWidget()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  disconnect( d->ParameterNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeChanged() ) );
  disconnect( d->ParameterNodeSelector, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onParameterNodeAddedByUser( vtkMRMLNode* ) ) );
  //disconnect( d->ModuleNodeComboBox, SIGNAL( nodeAboutToBeRemoved( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeAboutToBeRemoved( vtkMRMLNode* ) ) );

  disconnect( d->ModelNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );
  //connect( d->ModelNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode* )), this, SLOT(onModelNodeAddedByUser(vtkMRMLNode* ) ) );
  //connect( d->ModelNodeComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode* )), this, SLOT(onMarkupsToModelModelNodeAboutToBeRemoved(vtkMRMLNode* ) ) );

  disconnect( d->MarkupsSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onCurrentMarkupsNodeChanged() ) );
  disconnect( d->MarkupsSelector, SIGNAL( nodeAboutToBeEdited( vtkMRMLNode* ) ), this, SLOT( onNodeAboutToBeEdited( vtkMRMLNode* ) ) );

  //disconnect( d->UpdateOutputModelPushButton, SIGNAL( clicked() ) , this, SLOT( onUpdateOutputModelPushButton() ) );
  //disconnect( d->AutoUpdateOutputCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onAutoUpdateOutputToggled( bool ) ) );
  //disconnect( d->DeleteAllPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteAllPushButton() ) );
  //disconnect( d->DeleteLastPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteLastModelPushButton() ) );
  //disconnect( d->PlacePushButton, SIGNAL( clicked() ), this, SLOT( onPlacePushButtonClicked() ) );

  disconnect( d->ButterflySubdivisionCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onButterflySubdivisionToggled( bool ) ) );

  disconnect( d->CleanMarkupsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onCleanMarkupsToggled( bool ) ) );
  disconnect( d->ModeClosedSurfaceRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );
  disconnect( d->ModeCurveRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );
  disconnect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onDelaunayAlphaDoubleChanged( double ) ) );
  disconnect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onTubeRadiusDoubleChanged( double ) ) );

  disconnect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekBiasDoubleChanged( double ) ) );
  disconnect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekContinuityDoubleChanged( double ) ) );
  disconnect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekTensionDoubleChanged( double ) ) );

  disconnect( d->ModelOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( onOutputOpacityValueChanged( double ) ) );
  disconnect( d->ModelColorSelector, SIGNAL( colorChanged( QColor ) ), this, SLOT( onOutputColorChanged( QColor ) ) );
  disconnect( d->ModelVisiblityButton, SIGNAL( toggled( bool ) ), this, SLOT( onOutputVisibilityToggled( bool ) ) );
  disconnect( d->ModelSliceIntersectionCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( onOutputIntersectionVisibilityToggled( bool ) ) );
  disconnect( d->MarkupsTextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( onTextScaleChanged( double ) ) );

  disconnect( d->LinearInterpolationButton, SIGNAL( toggled( bool ) ), this, SLOT( onInterpolationBoxClicked( bool ) ) );
  disconnect( d->CardinalInterpolationRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onInterpolationBoxClicked( bool ) ) );
  disconnect( d->KochanekInterpolationRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onInterpolationBoxClicked( bool ) ) );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onNodeAboutToBeEdited( vtkMRMLNode* node )
{
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( node );
  this->qvtkDisconnect(markupsToModelModuleNode, vtkMRMLMarkupsToModelNode::InputDataModifiedEvent, this, SLOT(updateFromMRMLNode()));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onCurrentMarkupsNodeChanged()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }

  this->qvtkConnect( markupsToModelModuleNode, vtkMRMLMarkupsToModelNode::InputDataModifiedEvent, this, SLOT( updateFromMRMLNode() ) );

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsSelector->currentNode() );
  d->logic()->SetMarkupsNode( markupsNode, markupsToModelModuleNode );
  if( markupsToModelModuleNode->GetAutoUpdateOutput() )
  {
    d->logic()->UpdateOutputModel( markupsToModelModuleNode );
  }
  d->logic()->UpdateSelectionNode( markupsToModelModuleNode );

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::setup()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  d->setupUi( this );
  this->Superclass::setup();

  // Setup Update button menu
  // Install event filter to override menu position to show it on the right side of the button.
  // This is necessary because the update button is very wide and the
  // menu arrow is on the right side. With the default QMenu the menu would appear
  // on the left side, which would be very inconvenient because the mouse would need
  // to be moved a lot to click the manual/auto option.
  QMenu* updateMenu = new QMenu(tr("Update options"), this);
  updateMenu->installEventFilter(this);
  updateMenu->setObjectName("UpdateOptions");
  QActionGroup* updateActions = new QActionGroup(d->UpdateButton);
  updateActions->setExclusive(true);
  updateMenu->addAction( d->ActionUpdateAuto );
  updateActions->addAction( d->ActionUpdateAuto );
  QObject::connect( d->ActionUpdateAuto, SIGNAL( triggered() ), this, SLOT(onAutoUpdateSelected() ) );
  updateMenu->addAction( d->ActionUpdateManual );
  updateActions->addAction( d->ActionUpdateManual );
  QObject::connect( d->ActionUpdateManual, SIGNAL( triggered() ), this, SLOT(onManualUpdateSelected() ) );
  d->UpdateButton->setMenu(updateMenu);

  connect( d->ParameterNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeChanged() ) );
  connect( d->ParameterNodeSelector, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onParameterNodeAddedByUser( vtkMRMLNode* ) ) );
  //connect( d->ModuleNodeComboBox, SIGNAL( nodeAboutToBeRemoved( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeAboutToBeRemoved( vtkMRMLNode* ) ) );

  connect( d->ModelNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );
  //connect( d->ModelNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode* )), this, SLOT(onModelNodeAddedByUser(vtkMRMLNode* ) ) );
  //connect( d->ModelNodeComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode* )), this, SLOT(onMarkupsToModelModelNodeAboutToBeRemoved(vtkMRMLNode* ) ) );

  connect( d->MarkupsSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onCurrentMarkupsNodeChanged() ) );
  connect( d->MarkupsSelector, SIGNAL( nodeAboutToBeEdited( vtkMRMLNode* ) ), this, SLOT( onNodeAboutToBeEdited( vtkMRMLNode* ) ) );

  //connect( d->UpdateOutputModelPushButton, SIGNAL( clicked() ) , this, SLOT( onUpdateOutputModelPushButton() ) );
  //connect( d->DeleteAllPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteAllPushButton() ) );
  //d->DeleteAllPushButton->setIcon( QIcon( ":/Icons/MarkupsDeleteAllRows.png" ) );
  //connect( d->DeleteLastPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteLastModelPushButton() ) );
  //d->DeleteLastPushButton->setIcon( QIcon( ":/Icons/MarkupsDeleteLast.png" ) );
  //connect( d->PlacePushButton, SIGNAL( clicked() ), this, SLOT( onPlacePushButtonClicked() ) ); 
  //d->PlacePushButton->setIcon( QIcon( ":/Icons/MarkupsMouseModePlace.png" ) );
  //connect( d->AutoUpdateOutputCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onAutoUpdateOutputToggled( bool ) ) );
  connect( d->UpdateButton, SIGNAL(clicked()), this, SLOT(onUpdateOutputModelPushButton()) );
  connect( d->UpdateButton, SIGNAL(toggled(bool)), this, SLOT(onAutoUpdateOutputToggled(bool)) );

  connect( d->ButterflySubdivisionCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onButterflySubdivisionToggled( bool ) ) );

  connect( d->CleanMarkupsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onCleanMarkupsToggled( bool ) ) );

  d->CleanMarkupsCheckBox->setToolTip(QString("It will merge duplicate  points.  Duplicate points are the ones closer than tolerance distance. The tolerance distance is 1% diagonal size bounding box of total points."));
  
  connect( d->ModeClosedSurfaceRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );
  connect( d->ModeCurveRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );
  connect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onDelaunayAlphaDoubleChanged( double ) ) );
  connect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onTubeRadiusDoubleChanged( double ) ) );
  
  connect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekBiasDoubleChanged( double ) ) );
  connect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekContinuityDoubleChanged( double ) ) );
  connect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekTensionDoubleChanged( double ) ) );

  connect( d->ModelOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( onOutputOpacityValueChanged( double ) ) );
  connect( d->ModelColorSelector, SIGNAL( colorChanged( QColor ) ), this, SLOT( onOutputColorChanged( QColor ) ) );
  connect( d->ModelVisiblityButton, SIGNAL( toggled( bool ) ), this, SLOT( onOutputVisibilityToggled( bool ) ) );
  connect( d->ModelSliceIntersectionCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( onOutputIntersectionVisibilityToggled( bool ) ) );
  connect( d->MarkupsTextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( onTextScaleChanged( double ) ) );

  connect( d->LinearInterpolationButton, SIGNAL( toggled( bool ) ), this, SLOT( onInterpolationBoxClicked( bool ) ) );
  connect( d->CardinalInterpolationRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onInterpolationBoxClicked( bool ) ) );
  connect( d->KochanekInterpolationRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onInterpolationBoxClicked( bool ) ) );
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::enter()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // Create a module MRML node if there is none in the scene.

  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLMarkupsToModelNode" );
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLMarkupsToModelNode > newNode = vtkSmartPointer< vtkMRMLMarkupsToModelNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLMarkupsToModelNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node" );
    return;
  }

  //// For convenience, select a default module.
  if ( d->ParameterNodeSelector->currentNode() == NULL )
  {
    d->ParameterNodeSelector->setCurrentNodeID( node->GetID() );
  }
  onModeGroupBoxClicked( true );

  this->previouslyPersistent = qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->GetPlaceModePersistence();
  qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->SetPlaceModePersistence(1);

  //TODO UPDATE selectionNode according to markups toolbox when markups module enter selected
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  this->qvtkConnect( markupsToModelModuleNode, vtkMRMLMarkupsToModelNode::InputDataModifiedEvent, this, SLOT( updateFromMRMLNode() ) );

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsSelector->currentNode() );
  d->logic()->SetMarkupsNode( markupsNode, markupsToModelModuleNode );
  if( markupsToModelModuleNode->GetAutoUpdateOutput() )
  {
    d->logic()->UpdateOutputModel( markupsToModelModuleNode );
  }

  d->logic()->UpdateSelectionNode( markupsToModelModuleNode );

  this->Superclass::enter();
  this->updateFromMRMLNode();
}

void qSlicerMarkupsToModelModuleWidget::exit()
{
  qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->SetPlaceModePersistence(this->previouslyPersistent);
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  qCritical( "PERRAS" );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()) );
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onSceneImportedEvent()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  qCritical( "PERRAS" );
  this->enter();
  this->updateFromMRMLNode();
  //this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateWidget()
{
  //qCritical( "HOLAS" );
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLNode* currentModuleNode = d->ParameterNodeSelector->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->MarkupsSelector->setCurrentNode( NULL );
    d->MarkupsSelector->setEnabled( false );
    return;
  }
  //vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentModuleNode );
  //if ( markupsToModelModuleNode == NULL )
  //{
  //  qCritical( "Selected node not a valid module node" );
  //  return;
  //}
  //vtkMRMLDisplayableNode* currentMarkupsNode = vtkMRMLDisplayableNode::SafeDownCast( d->MarkupsSelector->currentNode() );
  //d->DeleteAllPushButton->blockSignals( true );
  //d->DeleteLastPushButton->blockSignals( true );
  //d->UpdateOutputModelPushButton->blockSignals( true );
  //if ( currentMarkupsNode == NULL )
  //{
  //  d->DeleteAllPushButton->setChecked( Qt::Unchecked );
  //  d->DeleteLastPushButton->setChecked( Qt::Unchecked );
  //  d->UpdateOutputModelPushButton->setChecked( Qt::Unchecked );
  //  // This will ensure that we refresh the widget next time we move to a non-null widget ( since there is guaranteed to be a modified status of larger than zero )
  //  //return;
  //  d->DeleteAllPushButton->setEnabled( false );
  //  d->DeleteLastPushButton->setEnabled( false );
  //  d->UpdateOutputModelPushButton->setEnabled( false );
  //}
  //else
  //{
  //  // Set the button indicating if this list is active
  //  if ( markupsToModelModuleNode->GetMarkupsNode()!=NULL && markupsToModelModuleNode->GetMarkupsNode()->GetNumberOfFiducials() > 0 )
  //  {
  //    d->DeleteAllPushButton->setEnabled( true );
  //    d->DeleteLastPushButton->setEnabled( true );
  //    if( markupsToModelModuleNode->GetMarkupsNode()->GetNumberOfFiducials() >= MINIMUM_MARKUPS_NUMBER )
  //    {
  //      d->UpdateOutputModelPushButton->setEnabled( true );
  //    }
  //    else
  //    {
  //      d->UpdateOutputModelPushButton->setEnabled( false );
  //    }
  //  }
  //  else
  //  {
  //    d->DeleteAllPushButton->setEnabled( false );
  //    d->DeleteLastPushButton->setEnabled( false );
  //    d->UpdateOutputModelPushButton->setEnabled( false );
  //  }
  //}
  //d->DeleteAllPushButton->blockSignals( false );
  //d->DeleteLastPushButton->blockSignals( false );
  //d->UpdateOutputModelPushButton->blockSignals( false );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateFromMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLNode* currentNode = d->ParameterNodeSelector->currentNode();
  if ( currentNode == NULL )
  {
    d->MarkupsSelector->setCurrentNode( NULL );
    d->MarkupsSelector->setEnabled( false );
    return;
  }
  vtkMRMLMarkupsToModelNode* MarkupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentNode );
  if ( MarkupsToModelNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  d->MarkupsSelector->setEnabled( true );

  if ( /*d->ModelNodeComboBox->currentNode() == 0 &&*/ MarkupsToModelNode->GetMarkupsNode() != NULL )
  {
    d->MarkupsSelector->setCurrentNode( MarkupsToModelNode->GetMarkupsNode() );
  }
  else
  {
    d->MarkupsSelector->setCurrentNode( NULL );
  }

  if ( /*d->ModelNodeComboBox->currentNode() == 0 && */MarkupsToModelNode->GetModelNode() != NULL )
  {
    d->ModelNodeSelector->setCurrentNodeID( MarkupsToModelNode->GetModelNode()->GetID() );
    //qCritical( "Model not null" );
  }
  else
  {
    d->ModelNodeSelector->setCurrentNodeID( "None" );
  }
  //d->AutoUpdateOutputCheckBox->setChecked( MarkupsToModelNode->GetAutoUpdateOutput() );

  d->ModelOpacitySlider->setValue( MarkupsToModelNode->GetOutputOpacity() );
  double outputColor[3];
  MarkupsToModelNode->GetOutputColor( outputColor );
  QColor nodeOutputColor;
  nodeOutputColor.setRgbF( outputColor[0],outputColor[1],outputColor[2] );
  d->ModelColorSelector->setColor( nodeOutputColor );
  d->ModelVisiblityButton->setChecked( MarkupsToModelNode->GetOutputVisibility() );
  d->ModelSliceIntersectionCheckbox->setChecked( MarkupsToModelNode->GetOutputIntersectionVisibility() );

  d->CleanMarkupsCheckBox->setChecked( MarkupsToModelNode->GetCleanMarkups() );

  switch( MarkupsToModelNode->GetModelType() )
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: 
    d->ModeClosedSurfaceRadioButton->setChecked( 1 );
    d->ButterflySubdivisionCheckBox->setChecked( MarkupsToModelNode->GetButterflySubdivision() );
    d->DelaunayAlphaDoubleSpinBox->setValue( MarkupsToModelNode->GetDelaunayAlpha() );
    break;

  case vtkMRMLMarkupsToModelNode::Curve:
    d->ModeCurveRadioButton->setChecked( 1 );
    d->TubeRadiusDoubleSpinBox->setValue(MarkupsToModelNode->GetTubeRadius());
    switch( MarkupsToModelNode->GetInterpolationType() )
    {
    case vtkMRMLMarkupsToModelNode::Linear: d->LinearInterpolationButton->setChecked( 1 ); break;
    case vtkMRMLMarkupsToModelNode::CardinalSpline: d->CardinalInterpolationRadioButton->setChecked( 1 ); break;
    case vtkMRMLMarkupsToModelNode::KochanekSpline: 
      d->KochanekInterpolationRadioButton->setChecked( 1 ); 
      d->KochanekBiasDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekBias());
      d->KochanekContinuityDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekContinuity());
      d->KochanekTensionDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekTension());
      break;
    }
    break;
  }

  this->updateWidget();
}




//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onModelNodeChanged()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  
  vtkMRMLNode* currentNode = d->ParameterNodeSelector->currentNode();
  if ( currentNode == NULL )
  {
    return;
  }
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentNode );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( d->ModelNodeSelector->currentNode() );
  if ( modelNode == NULL )
  {
    qCritical( "Selected node not a valid model node" );
    markupsToModelModuleNode->SetModelNode( NULL );
    markupsToModelModuleNode->SetModelNodeID( "" );
    return;
  }

  markupsToModelModuleNode->SetModelNode( modelNode );
  markupsToModelModuleNode->SetModelNodeID( modelNode->GetID() );

  UpdateOutputModel();
}

////-----------------------------------------------------------------------------
//void qSlicerMarkupsToModelModuleWidget::onModelNodeAddedByUser( vtkMRMLNode* nodeAdded )
//{
//  Q_D( qSlicerMarkupsToModelModuleWidget );
//  if ( this->mrmlScene() == NULL )
//  {
//    qCritical() << "Invalid scene!";
//    return;
//  }
//  // For convenience, select a default module.
//  if( nodeAdded==NULL )
//  {
//    return;
//  }
//  vtkMRMLModelNode* ModelNodeAdded = vtkMRMLModelNode::SafeDownCast( nodeAdded );
//  if( ModelNodeAdded==NULL )
//  {
//    return;
//  }
//}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onParameterNodeChanged()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  d->logic()->UpdateSelectionNode( markupsToModelModuleNode );
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onParameterNodeAddedByUser( vtkMRMLNode* nodeAdded )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default module.
  if( nodeAdded==NULL )
  {
    return;
  }
  vtkMRMLMarkupsToModelNode* MarkupsToModelNodeAdded = vtkMRMLMarkupsToModelNode::SafeDownCast( nodeAdded );
  if( MarkupsToModelNodeAdded==NULL )
  {
    return;
  }
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onUpdateOutputModelPushButton()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  d->logic()->UpdateOutputModel( markupsToModelModuleNode );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::UpdateOutputModel()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  if( markupsToModelModuleNode->GetAutoUpdateOutput() )
  {
    d->logic()->UpdateOutputModel( markupsToModelModuleNode );
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onDeleteAllPushButton()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->RemoveAllMarkups();
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onInterpolationBoxClicked( bool nana )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Conversion Mode changed with no module node selection" );
    return;
  }

  if( d->LinearInterpolationButton->isChecked() )
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::Linear );
  }
  else if(d->CardinalInterpolationRadioButton->isChecked())
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::CardinalSpline );
  }
  else if(d->KochanekInterpolationRadioButton->isChecked())
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::KochanekSpline );
  }
  else
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::HermiteSpline );
  }

  if(d->KochanekInterpolationRadioButton->isChecked())
  {
    d->KochanekBiasLabel->setVisible( true );
    d->KochanekBiasDoubleSpinBox->setVisible( true );
    d->KochanekTensionLabel->setVisible( true );
    d->KochanekTensionDoubleSpinBox->setVisible( true );
    d->KochanekContinuityLabel->setVisible( true );
    d->KochanekContinuityDoubleSpinBox->setVisible( true );
  }
  else
  {
    d->KochanekBiasLabel->setVisible( false );
    d->KochanekBiasDoubleSpinBox->setVisible( false );
    d->KochanekTensionLabel->setVisible( false );
    d->KochanekTensionDoubleSpinBox->setVisible( false );
    d->KochanekContinuityLabel->setVisible( false );
    d->KochanekContinuityDoubleSpinBox->setVisible( false );
  }

  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onModeGroupBoxClicked( bool nana )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  //qCritical( "HOLAS" );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Conversion Mode changed with no module node selection" );
    return;
  }

  if( d->ModeClosedSurfaceRadioButton->isChecked() )
  {
    d->ButterflySubdivisionCheckBox->setVisible( true );
    d->DelaunayAlphaLabel->setVisible( true );
    d->DelaunayAlphaDoubleSpinBox->setVisible( true );

    d->SplineInterpolationLayout->setEnabled(false);
    d->TubeRadiusLabel->setVisible( false );
    d->TubeRadiusDoubleSpinBox->setVisible( false );
    d->KochanekBiasLabel->setVisible( false );
    d->KochanekBiasDoubleSpinBox->setVisible( false );
    d->KochanekTensionLabel->setVisible( false );
    d->KochanekTensionDoubleSpinBox->setVisible( false );
    d->KochanekContinuityLabel->setVisible( false );
    d->KochanekContinuityDoubleSpinBox->setVisible( false );
    markupsToModelModuleNode->SetModelType( vtkMRMLMarkupsToModelNode::ClosedSurface );
  }
  else
  {
    d->ButterflySubdivisionCheckBox->setVisible( false );
    d->DelaunayAlphaLabel->setVisible( false );
    d->DelaunayAlphaDoubleSpinBox->setVisible( false );

    d->SplineInterpolationLayout->setEnabled(true);
    d->TubeRadiusLabel->setVisible( true );
    d->TubeRadiusDoubleSpinBox->setVisible( true );
    markupsToModelModuleNode->SetModelType( vtkMRMLMarkupsToModelNode::Curve );

    if(d->KochanekInterpolationRadioButton->isChecked())
    {
      d->KochanekBiasLabel->setVisible( true );
      d->KochanekBiasDoubleSpinBox->setVisible( true );
      d->KochanekTensionLabel->setVisible( true );
      d->KochanekTensionDoubleSpinBox->setVisible( true );
      d->KochanekContinuityLabel->setVisible( true );
      d->KochanekContinuityDoubleSpinBox->setVisible( true );
    }
    else
    {
      d->KochanekBiasLabel->setVisible( false );
      d->KochanekBiasDoubleSpinBox->setVisible( false );
      d->KochanekTensionLabel->setVisible( false );
      d->KochanekTensionDoubleSpinBox->setVisible( false );
      d->KochanekContinuityLabel->setVisible( false );
      d->KochanekContinuityDoubleSpinBox->setVisible( false );
    }
  }

  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onTubeRadiusDoubleChanged( double tubeRadius )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetTubeRadius( tubeRadius );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onKochanekContinuityDoubleChanged( double kochanekContinuity )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetKochanekContinuity( kochanekContinuity );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onKochanekBiasDoubleChanged( double kochanekBias )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetKochanekBias( kochanekBias );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onKochanekTensionDoubleChanged( double kochanekTension )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetKochanekTension( kochanekTension );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onDelaunayAlphaDoubleChanged( double delaunayAlpha )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetDelaunayAlpha( delaunayAlpha );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onOutputOpacityValueChanged( double outputOpacity )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetOutputOpacity( outputOpacity );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onOutputColorChanged( QColor newColor )
{
   Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetOutputColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onOutputVisibilityToggled( bool outputVisibility )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetOutputVisibility( outputVisibility );
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onTextScaleChanged( double textScale )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  if (d->logic()->MarkupsLogic == 0)
  {
    return;
  }

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsSelector->currentNode() );
  if ( markupsNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  vtkMRMLMarkupsDisplayNode * markupsDisplay = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  markupsDisplay->SetTextScale(textScale);
  //d->logic()->MarkupsLogic->SetDefaultMarkupsDisplayNodeTextScale( textScale );
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onOutputIntersectionVisibilityToggled( bool outputIntersectionVisibility )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetOutputIntersectionVisibility( outputIntersectionVisibility );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onCleanMarkupsToggled( bool cleanMarkups )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetCleanMarkups( cleanMarkups );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onButterflySubdivisionToggled( bool butterflySubdivision )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetButterflySubdivision( butterflySubdivision );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onAutoUpdateOutputToggled( bool autoUpdateOutput )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetAutoUpdateOutput( autoUpdateOutput );
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onDeleteLastModelPushButton()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->RemoveLastMarkup();

}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onPlacePushButtonClicked()
{

  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsSelector->currentNode() );

  // Depending to the current state, change the activeness and placeness for the current markups node
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast( this->mrmlScene()->GetNodeByID( "vtkMRMLInteractionNodeSingleton" ) );

  bool isActive = currentMarkupsNode != NULL && d->logic()->MarkupsLogic->GetActiveListID().compare( currentMarkupsNode->GetID() ) == 0;
  bool isPlace = interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place;

  if ( isPlace && isActive )
  {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::ViewTransform );
  }
  else
  {
    interactionNode->SetCurrentInteractionMode( vtkMRMLInteractionNode::Place );
    // interactionNode->SetPlaceModePersistence( true ); // Use whatever persistence the user has already set
  }

  if ( currentMarkupsNode != NULL )
  {
    d->logic()->MarkupsLogic->SetActiveListID( currentMarkupsNode ); // If there are other widgets, they are responsible for updating themselves
  }

  this->updateWidget();
}


