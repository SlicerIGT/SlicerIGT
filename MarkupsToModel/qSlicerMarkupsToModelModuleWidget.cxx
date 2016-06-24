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
  disconnect( d->ParameterNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateFromMRMLNode() ) );

  disconnect( d->ModelNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->MarkupsSelector, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->UpdateButton, SIGNAL( clicked() ), this, SLOT( UpdateOutputModel() ) );
  disconnect( d->UpdateButton, SIGNAL( toggled( bool ) ), this, SLOT( updateFromMRMLNode() ) );

  disconnect( d->ButterflySubdivisionCheckBox, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->ConvexHullCheckBox, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->CleanMarkupsCheckBox, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );

  disconnect( d->ModeClosedSurfaceRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->ModeCurveRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->TubeSegmentsSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->TubeSidesSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  
  disconnect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );

  disconnect( d->ModelOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->ModelColorSelector, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->ModelVisiblityButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->ModelSliceIntersectionCheckbox, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->MarkupsTextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );

  disconnect( d->LinearInterpolationButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->CardinalInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  disconnect( d->KochanekInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
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
  QMenu* updateMenu = new QMenu( tr( "Update options" ), this );
  updateMenu->installEventFilter( this );
  updateMenu->setObjectName( "UpdateOptions" );
  QActionGroup* updateActions = new QActionGroup( d->UpdateButton );
  updateActions->setExclusive( true );
  updateMenu->addAction( d->ActionUpdateAuto );
  updateActions->addAction( d->ActionUpdateAuto );
  QObject::connect( d->ActionUpdateAuto, SIGNAL( triggered() ), this, SLOT(onActionUpdateAuto() ) );
  updateMenu->addAction( d->ActionUpdateManual );
  updateActions->addAction( d->ActionUpdateManual );
  QObject::connect( d->ActionUpdateManual, SIGNAL( triggered() ), this, SLOT(onActionUpdateManual() ) );
  d->UpdateButton->setMenu(updateMenu);

  d->MarkupsSelector->tableWidget()->setHidden( true ); // we don't need to see the table of fiducials
  
  d->MarkupsSelector->setNodeBaseName( "ModelMarkup" );
  QColor markupDefaultColor;
  markupDefaultColor.setRgbF(1, 0.5, 0.5);
  d->MarkupsSelector->setDefaultNodeColor( markupDefaultColor );

  connect( d->ParameterNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateFromMRMLNode() ) );

  connect( d->ModelNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->MarkupsSelector, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( updateToMRMLNode() ) );
  connect( d->UpdateButton, SIGNAL( clicked() ), this, SLOT( UpdateOutputModel() ) );
  connect( d->UpdateButton, SIGNAL( toggled( bool ) ), this, SLOT( updateFromMRMLNode() ) );

  connect( d->ButterflySubdivisionCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->ConvexHullCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->CleanMarkupsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateToMRMLNode() ) );

  connect( d->ModeClosedSurfaceRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  connect( d->ModeCurveRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  connect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->TubeSegmentsSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->TubeSidesSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  
  connect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );

  connect( d->ModelOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->ModelColorSelector, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  connect( d->ModelVisiblityButton, SIGNAL( toggled( bool ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->ModelSliceIntersectionCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( updateToMRMLNode() ) );
  connect( d->MarkupsTextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToMRMLNode() ) );

  connect( d->LinearInterpolationButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  connect( d->CardinalInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
  connect( d->KochanekInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateToMRMLNode() ) );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::enter()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  this->blockAllSignals();

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
    d->ParameterNodeSelector->setCurrentNode( node );
  }

  this->previouslyPersistent = qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->GetPlaceModePersistence();
  qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->SetPlaceModePersistence(1);

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  this->qvtkConnect( markupsToModelModuleNode, vtkMRMLMarkupsToModelNode::InputDataModifiedEvent, this, SLOT( updateFromMRMLNode() ) );

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsSelector->currentNode() );
  d->logic()->SetMarkupsNode( markupsNode, markupsToModelModuleNode );
  d->logic()->UpdateSelectionNode( markupsToModelModuleNode );

  if( markupsToModelModuleNode->GetAutoUpdateOutput() )
  {
    UpdateOutputModel();
  }

  this->unblockAllSignals();
  this->Superclass::enter();
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::exit()
{
  qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->SetPlaceModePersistence(this->previouslyPersistent);
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()) );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onSceneImportedEvent()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  this->enter();
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateToMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLNode* currentMRMLNode = d->ParameterNodeSelector->currentNode();
  if ( currentMRMLNode == NULL )
  {
    return;
  }
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentMRMLNode );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  if ( d->ModeClosedSurfaceRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetModelType( vtkMRMLMarkupsToModelNode::ClosedSurface );
  }
  else if ( d->ModeCurveRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetModelType( vtkMRMLMarkupsToModelNode::Curve );
  }
  else
  {
    qCritical("Invalid markups to model mode selected.");
  }
  markupsToModelModuleNode->SetAutoUpdateOutput( d->UpdateButton->isChecked() );

  markupsToModelModuleNode->SetCleanMarkups( d->CleanMarkupsCheckBox->isChecked() );
  markupsToModelModuleNode->SetDelaunayAlpha( d->DelaunayAlphaDoubleSpinBox->value() );
  markupsToModelModuleNode->SetConvexHull( d->ConvexHullCheckBox->isChecked() );
  markupsToModelModuleNode->SetButterflySubdivision( d->ButterflySubdivisionCheckBox->isChecked() );
  
  markupsToModelModuleNode->SetTubeRadius( d->TubeRadiusDoubleSpinBox->value() );
  markupsToModelModuleNode->SetTubeResolutionLength( d->TubeSegmentsSpinBox->value() );
  markupsToModelModuleNode->SetTubeResolutionAround( d->TubeSidesSpinBox->value() );
  if ( d->LinearInterpolationButton->isChecked() )
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::Linear );
  }
  else if ( d->CardinalInterpolationRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::CardinalSpline );
  }
  else if ( d->KochanekInterpolationRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::KochanekSpline );
  }
  markupsToModelModuleNode->SetKochanekBias( d->KochanekBiasDoubleSpinBox->value() );
  markupsToModelModuleNode->SetKochanekContinuity( d->KochanekContinuityDoubleSpinBox->value() );
  markupsToModelModuleNode->SetKochanekTension( d->KochanekTensionDoubleSpinBox->value() );

  markupsToModelModuleNode->SetOutputVisibility( d->ModelVisiblityButton->isChecked() );
  markupsToModelModuleNode->SetOutputOpacity( d->ModelOpacitySlider->value() );
  markupsToModelModuleNode->SetOutputColor( d->ModelColorSelector->color().redF(), d->ModelColorSelector->color().greenF(), d->ModelColorSelector->color().blueF() );
  markupsToModelModuleNode->SetOutputIntersectionVisibility( d->ModelSliceIntersectionCheckbox->isChecked() );
  markupsToModelModuleNode->SetMarkupsTextScale( d->MarkupsTextScaleSlider->value() );
  
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( d->ModelNodeSelector->currentNode() );
  if ( modelNode == NULL )
  {
    qWarning( "Selected node not a valid model node" );
    markupsToModelModuleNode->SetAndObserveModelNodeID( NULL ); // TODO: Verify this is correct
  }
  else
  {
    std::string modelNodeID = modelNode->GetID();
    markupsToModelModuleNode->SetAndObserveModelNodeID( modelNodeID.c_str() );
  }

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsSelector->currentNode() );
  if ( markupsNode == NULL )
  {
    qWarning( "Selected node not a valid markups node" );
    markupsToModelModuleNode->SetAndObserveMarkupsNodeID( NULL ); // TODO: Verify this is correct
  }
  else
  {
    std::string markupsNodeID = markupsNode->GetID();
    markupsToModelModuleNode->SetAndObserveMarkupsNodeID( markupsNodeID.c_str() );
  }

  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateFromMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLNode* currentNode = d->ParameterNodeSelector->currentNode();
  if ( currentNode == NULL )
  {
    this->disableAllWidgets();
    return;
  }
  vtkMRMLMarkupsToModelNode* MarkupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentNode );
  if ( MarkupsToModelNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    this->disableAllWidgets();
    return;
  }
  this->enableAllWidgets();

  // block ALL signals until the function returns
  // if a return is called after this line, then unblockAllSignals should also be called.
  this->blockAllSignals();

  // Set the button indicating if this list is active
  if ( MarkupsToModelNode->GetMarkupsNode() != NULL &&
       MarkupsToModelNode->GetMarkupsNode()->GetNumberOfFiducials() >= MINIMUM_MARKUPS_NUMBER &&
       MarkupsToModelNode->GetModelNode() != NULL )
  {
    d->UpdateButton->setEnabled( true );
  }
  else
  {
    d->UpdateButton->setEnabled( false );
  }

  // make all widget inputs match the MRML node
  switch( MarkupsToModelNode->GetModelType() )
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: d->ModeClosedSurfaceRadioButton->setChecked( 1 ); break;
  case vtkMRMLMarkupsToModelNode::Curve: d->ModeCurveRadioButton->setChecked( 1 ); break;
  }
  d->MarkupsSelector->setCurrentNode( MarkupsToModelNode->GetMarkupsNode() );
  d->logic()->UpdateSelectionNode( MarkupsToModelNode );
  d->ModelNodeSelector->setCurrentNode( MarkupsToModelNode->GetModelNode() );
  if ( MarkupsToModelNode->GetAutoUpdateOutput() )
  {
    d->UpdateButton->setCheckable( true );
    d->UpdateButton->setChecked( true );
    d->UpdateButton->setText( "Automatic Update" );
    d->ActionUpdateAuto->setChecked( Qt::Checked );
    d->ActionUpdateManual->setChecked( Qt::Unchecked );
  }
  else
  {
    d->UpdateButton->setChecked( false );
    d->UpdateButton->setCheckable( false );
    d->UpdateButton->setText( "Update" );
    d->ActionUpdateAuto->setChecked( Qt::Unchecked );
    d->ActionUpdateManual->setChecked( Qt::Checked );
  }

  // advanced options
  d->CleanMarkupsCheckBox->setChecked( MarkupsToModelNode->GetCleanMarkups() );
  // closed surface
  d->ButterflySubdivisionCheckBox->setChecked( MarkupsToModelNode->GetButterflySubdivision() );
  d->DelaunayAlphaDoubleSpinBox->setValue( MarkupsToModelNode->GetDelaunayAlpha() );
  d->ConvexHullCheckBox->setChecked( MarkupsToModelNode->GetConvexHull() );
  // curve
  d->TubeRadiusDoubleSpinBox->setValue( MarkupsToModelNode->GetTubeRadius() );
  d->TubeSidesSpinBox->setValue( MarkupsToModelNode->GetTubeResolutionAround() );
  d->TubeSegmentsSpinBox->setValue( MarkupsToModelNode->GetTubeResolutionLength() );
  switch( MarkupsToModelNode->GetInterpolationType() )
  {
  case vtkMRMLMarkupsToModelNode::Linear: d->LinearInterpolationButton->setChecked( 1 ); break;
  case vtkMRMLMarkupsToModelNode::CardinalSpline: d->CardinalInterpolationRadioButton->setChecked( 1 ); break;
  case vtkMRMLMarkupsToModelNode::KochanekSpline: d->KochanekInterpolationRadioButton->setChecked( 1 ); break;
  }
  d->KochanekBiasDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekBias());
  d->KochanekContinuityDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekContinuity());
  d->KochanekTensionDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekTension());

  // display options
  d->ModelVisiblityButton->setChecked( MarkupsToModelNode->GetOutputVisibility() );
  d->ModelOpacitySlider->setValue( MarkupsToModelNode->GetOutputOpacity() );
  double outputColor[3];
  MarkupsToModelNode->GetOutputColor( outputColor );
  QColor nodeOutputColor;
  nodeOutputColor.setRgbF( outputColor[0],outputColor[1],outputColor[2] );
  d->ModelColorSelector->setColor( nodeOutputColor );
  d->ModelSliceIntersectionCheckbox->setChecked( MarkupsToModelNode->GetOutputIntersectionVisibility() );
  d->MarkupsTextScaleSlider->setValue(MarkupsToModelNode->GetMarkupsTextScale());

  // determine visibility of widgets
  if( d->ModeClosedSurfaceRadioButton->isChecked() )
  {
    d->ButterflySubdivisionLabel->setVisible( true );
    d->ButterflySubdivisionCheckBox->setVisible( true );
    d->DelaunayAlphaLabel->setVisible( true );
    d->DelaunayAlphaDoubleSpinBox->setVisible( true );
    d->ConvexHullLabel->setVisible( true );
    d->ConvexHullCheckBox->setVisible( true );

    d->SplineInterpolationLayout->setEnabled(false);
    d->SplineInterpolationLabel->setVisible(false);
    d->LinearInterpolationButton->setVisible(false);
    d->KochanekInterpolationRadioButton->setVisible(false);
    d->CardinalInterpolationRadioButton->setVisible(false);
    d->TubeRadiusLabel->setVisible( false );
    d->TubeRadiusDoubleSpinBox->setVisible( false );
    d->TubeSidesLabel->setVisible( false );
    d->TubeSidesSpinBox->setVisible( false );
    d->TubeSegmentsLabel->setVisible( false );
    d->TubeSegmentsSpinBox->setVisible( false );
    d->KochanekBiasLabel->setVisible( false );
    d->KochanekBiasDoubleSpinBox->setVisible( false );
    d->KochanekTensionLabel->setVisible( false );
    d->KochanekTensionDoubleSpinBox->setVisible( false );
    d->KochanekContinuityLabel->setVisible( false );
    d->KochanekContinuityDoubleSpinBox->setVisible( false );
  }
  else
  {
    d->ButterflySubdivisionLabel->setVisible( false );
    d->ButterflySubdivisionCheckBox->setVisible( false );
    d->DelaunayAlphaLabel->setVisible( false );
    d->DelaunayAlphaDoubleSpinBox->setVisible( false );
    d->ConvexHullLabel->setVisible( false );
    d->ConvexHullCheckBox->setVisible( false );

    d->SplineInterpolationLayout->setEnabled(true);
    d->SplineInterpolationLabel->setVisible(true);
    d->LinearInterpolationButton->setVisible(true);
    d->KochanekInterpolationRadioButton->setVisible(true);
    d->CardinalInterpolationRadioButton->setVisible(true);
    d->TubeRadiusLabel->setVisible( true );
    d->TubeRadiusDoubleSpinBox->setVisible( true );
    d->TubeSidesLabel->setVisible( true );
    d->TubeSidesSpinBox->setVisible( true );
    d->TubeSegmentsLabel->setVisible( true );
    d->TubeSegmentsSpinBox->setVisible( true );

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

  this->unblockAllSignals();
  
  // the parameters have possibly changed, so also update the model (if automatic update is on)
  if( MarkupsToModelNode->GetAutoUpdateOutput() )
  {
    UpdateOutputModel();
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onActionUpdateAuto()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelNode == NULL )
  {
    return;
  }
  markupsToModelNode->SetAutoUpdateOutput( true );
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onActionUpdateManual()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelNode == NULL )
  {
    return;
  }
  markupsToModelNode->SetAutoUpdateOutput( false );
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::blockAllSignals()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  d->ParameterNodeSelector->blockSignals( true );
  d->ModeClosedSurfaceRadioButton->blockSignals( true );
  d->ModeCurveRadioButton->blockSignals( true );
  d->MarkupsSelector->blockSignals( true );  
  d->ModelNodeSelector->blockSignals( true );
  d->UpdateButton->blockSignals( true );
  d->ActionUpdateAuto->blockSignals( true );
  d->ActionUpdateManual->blockSignals( true );

  // advanced options
  d->CleanMarkupsCheckBox->blockSignals( true );
  // closed surface options
  d->ButterflySubdivisionCheckBox->blockSignals( true );
  d->DelaunayAlphaDoubleSpinBox->blockSignals( true );
  d->ConvexHullCheckBox->blockSignals( true );
  // curve options
  d->TubeSidesSpinBox->blockSignals( true );
  d->TubeRadiusDoubleSpinBox->blockSignals( true );
  d->TubeSegmentsSpinBox->blockSignals( true );
  d->LinearInterpolationButton->blockSignals( true );
  d->CardinalInterpolationRadioButton->blockSignals( true );
  d->KochanekInterpolationRadioButton->blockSignals( true );
  d->KochanekBiasDoubleSpinBox->blockSignals( true );
  d->KochanekContinuityDoubleSpinBox->blockSignals( true );
  d->KochanekTensionDoubleSpinBox->blockSignals( true );

  // display options
  d->ModelVisiblityButton->blockSignals( true );
  d->ModelOpacitySlider->blockSignals( true );
  d->ModelColorSelector->blockSignals( true );
  d->ModelSliceIntersectionCheckbox->blockSignals( true );
  d->MarkupsTextScaleSlider->blockSignals( true );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::unblockAllSignals()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  d->ParameterNodeSelector->blockSignals( false );
  d->ModeClosedSurfaceRadioButton->blockSignals( false );
  d->ModeCurveRadioButton->blockSignals( false );
  d->MarkupsSelector->blockSignals( false );  
  d->ModelNodeSelector->blockSignals( false );
  d->UpdateButton->blockSignals( false );
  d->ActionUpdateAuto->blockSignals( false );
  d->ActionUpdateManual->blockSignals( false );

  // advanced options
  d->CleanMarkupsCheckBox->blockSignals( false );
  // closed surface options
  d->ButterflySubdivisionCheckBox->blockSignals( false );
  d->DelaunayAlphaDoubleSpinBox->blockSignals( false );
  d->ConvexHullCheckBox->blockSignals( false );
  // curve options
  d->TubeSidesSpinBox->blockSignals( false );
  d->TubeRadiusDoubleSpinBox->blockSignals( false );
  d->TubeSegmentsSpinBox->blockSignals( false );
  d->LinearInterpolationButton->blockSignals( false );
  d->CardinalInterpolationRadioButton->blockSignals( false );
  d->KochanekInterpolationRadioButton->blockSignals( false );
  d->KochanekBiasDoubleSpinBox->blockSignals( false );
  d->KochanekContinuityDoubleSpinBox->blockSignals( false );
  d->KochanekTensionDoubleSpinBox->blockSignals( false );

  // display options
  d->ModelVisiblityButton->blockSignals( false );
  d->ModelOpacitySlider->blockSignals( false );
  d->ModelColorSelector->blockSignals( false );
  d->ModelSliceIntersectionCheckbox->blockSignals( false );
  d->MarkupsTextScaleSlider->blockSignals( false );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::enableAllWidgets()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  d->ModeClosedSurfaceRadioButton->setEnabled( true );
  d->ModeCurveRadioButton->setEnabled( true );
  d->MarkupsSelector->setEnabled( true );
  d->ModelNodeSelector->setEnabled( true );
  d->UpdateButton->setEnabled( true );
  d->AdvancedGroupBox->setEnabled( true );
  d->DisplayGroupBox->setEnabled( true );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::disableAllWidgets()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  d->ModeClosedSurfaceRadioButton->setEnabled( false );
  d->ModeCurveRadioButton->setEnabled( false );
  d->MarkupsSelector->setEnabled( false );
  d->ModelNodeSelector->setEnabled( false );
  d->UpdateButton->setEnabled( false );
  d->AdvancedGroupBox->setEnabled( false );
  d->DisplayGroupBox->setEnabled( false );
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
  d->logic()->UpdateOutputModel( markupsToModelModuleNode );
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsToModelModuleWidget::eventFilter(QObject * obj, QEvent *event)
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  if (event->type() == QEvent::Show && d->UpdateButton != NULL && obj == d->UpdateButton->menu())
  {
    // Show UpdateButton's menu aligned to the right side of the button
    QMenu* menu = d->UpdateButton->menu();
    QPoint menuPos = menu->pos();
    menu->move(menuPos.x()+d->UpdateButton->geometry().width()-menu->geometry().width(), menuPos.y());
    return true;
  }
  return false;
}
