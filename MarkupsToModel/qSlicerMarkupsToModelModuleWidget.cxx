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

// Slicer MRML includes
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLInteractionNode.h"

// logic
#include "vtkSlicerMarkupsToModelLogic.h"


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
  disconnect( d->ModelNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateModelToMRMLNode() ) );
  disconnect( d->MarkupsSelector, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( updateMarkupsToMRMLNode() ) );
  disconnect( d->UpdateButton, SIGNAL( clicked() ), this, SLOT( UpdateOutputModel() ) );
  disconnect( d->UpdateButton, SIGNAL( toggled( bool ) ), this, SLOT( updateFromMRMLNode() ) );

  disconnect( d->ButterflySubdivisionCheckBox, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->ConvexHullCheckBox, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->CleanMarkupsCheckBox, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );

  disconnect( d->ModeClosedSurfaceRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->ModeCurveRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->TubeSegmentsSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->TubeSidesSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  
  disconnect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->PointParameterRawIndicesRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->PointParameterMinimumSpanningTreeRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->PolynomialOrderSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );

  disconnect( d->ModelOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToRenderedNodes() ) );
  disconnect( d->ModelColorSelector, SIGNAL( clicked() ), this, SLOT( updateToRenderedNodes() ) );
  disconnect( d->ModelVisiblityButton, SIGNAL( clicked() ), this, SLOT( updateToRenderedNodes() ) );
  disconnect( d->ModelSliceIntersectionCheckbox, SIGNAL( clicked() ), this, SLOT( updateToRenderedNodes() ) );
  disconnect( d->MarkupsTextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToRenderedNodes() ) );

  disconnect( d->LinearInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->CardinalInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->KochanekInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  disconnect( d->PolynomialInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
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
  connect( d->ModelNodeSelector, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateModelToMRMLNode() ) );
  connect( d->MarkupsSelector, SIGNAL( markupsFiducialNodeChanged() ), this, SLOT( updateMarkupsToMRMLNode() ) );
  connect( d->UpdateButton, SIGNAL( clicked() ), this, SLOT( UpdateOutputModel() ) );
  connect( d->UpdateButton, SIGNAL( toggled( bool ) ), this, SLOT( updateFromMRMLNode() ) );

  connect( d->ButterflySubdivisionCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->ConvexHullCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->CleanMarkupsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( updateParametersToMRMLNode() ) );

  connect( d->ModeClosedSurfaceRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->ModeCurveRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->TubeSegmentsSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->TubeSidesSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  
  connect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->PointParameterRawIndicesRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->PointParameterMinimumSpanningTreeRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->PolynomialOrderSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateParametersToMRMLNode() ) );

  connect( d->ModelOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToRenderedNodes() ) );
  connect( d->ModelColorSelector, SIGNAL( clicked() ), this, SLOT( updateToRenderedNodes() ) );
  connect( d->ModelVisiblityButton, SIGNAL( toggled( bool ) ), this, SLOT( updateToRenderedNodes() ) );
  connect( d->ModelSliceIntersectionCheckbox, SIGNAL( toggled( bool ) ), this, SLOT( updateToRenderedNodes() ) );
  connect( d->MarkupsTextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( updateToRenderedNodes() ) );

  connect( d->LinearInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->CardinalInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->KochanekInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
  connect( d->PolynomialInterpolationRadioButton, SIGNAL( clicked() ), this, SLOT( updateParametersToMRMLNode() ) );
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
  if (markupsNode)
  {
    d->logic()->UpdateSelectionNode( markupsToModelModuleNode );
  }

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
void qSlicerMarkupsToModelModuleWidget::updateModelToMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  
  vtkMRMLNode* currentMRMLNode = d->ModelNodeSelector->currentNode();
  if ( currentMRMLNode == NULL )
  {
    return; // if "None" is selected, don't output any errors
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( currentMRMLNode );
  if ( modelNode == NULL )
  {
    qWarning( "Selected node not a valid model node" );
    markupsToModelModuleNode->SetAndObserveModelNodeID( NULL );
    return;
  }

  std::string modelNodeID = modelNode->GetID();
  markupsToModelModuleNode->SetAndObserveModelNodeID( modelNodeID.c_str() );
  
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateMarkupsToMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  
  vtkMRMLNode* currentMRMLNode = d->MarkupsSelector->currentNode();
  if ( currentMRMLNode == NULL )
  {
    return; // if "None" is selected, don't output any errors
  }

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( currentMRMLNode );
  if ( markupsNode == NULL )
  {
    qWarning( "Selected node not a valid markups node" );
    markupsToModelModuleNode->SetAndObserveMarkupsNodeID( NULL );
    return;
  }

  std::string markupsNodeID = markupsNode->GetID();
  markupsToModelModuleNode->SetAndObserveMarkupsNodeID( markupsNodeID.c_str() );
  
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateParametersToMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
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
  markupsToModelModuleNode->SetTubeSamplePointsBetweenControlPoints( d->TubeSegmentsSpinBox->value() );
  markupsToModelModuleNode->SetTubeNumberOfSides( d->TubeSidesSpinBox->value() );
  if ( d->LinearInterpolationRadioButton->isChecked() )
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
  else if ( d->PolynomialInterpolationRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::Polynomial );
  }
  markupsToModelModuleNode->SetKochanekBias( d->KochanekBiasDoubleSpinBox->value() );
  markupsToModelModuleNode->SetKochanekContinuity( d->KochanekContinuityDoubleSpinBox->value() );
  markupsToModelModuleNode->SetKochanekTension( d->KochanekTensionDoubleSpinBox->value() );
  if ( d->PointParameterRawIndicesRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetPointParameterType( vtkMRMLMarkupsToModelNode::RawIndices );
  }
  else if ( d->PointParameterMinimumSpanningTreeRadioButton->isChecked() )
  {
    markupsToModelModuleNode->SetPointParameterType( vtkMRMLMarkupsToModelNode::MinimumSpanningTree );
  }
  markupsToModelModuleNode->SetPolynomialOrder( d->PolynomialOrderSpinBox->value() );
  
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

  // match the high-level widgets to the MRML node
  switch( MarkupsToModelNode->GetModelType() )
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: d->ModeClosedSurfaceRadioButton->setChecked( 1 ); break;
  case vtkMRMLMarkupsToModelNode::Curve: d->ModeCurveRadioButton->setChecked( 1 ); break;
  }
  d->MarkupsSelector->setCurrentNode( MarkupsToModelNode->GetMarkupsNode() );
  if ( MarkupsToModelNode->GetMarkupsNode() )
  {
    d->logic()->UpdateSelectionNode( MarkupsToModelNode );
  }
  d->ModelNodeSelector->setCurrentNode( MarkupsToModelNode->GetModelNode() );

  // Set the button indicating if this list is active
  if ( MarkupsToModelNode->GetMarkupsNode() != NULL &&
       //MarkupsToModelNode->GetMarkupsNode()->GetNumberOfFiducials() >= MINIMUM_MARKUPS_NUMBER && // TODO: Evaluate if this call is necessary
       MarkupsToModelNode->GetModelNode() != NULL )
  {
    d->UpdateButton->setEnabled( true );
    d->AdvancedGroupBox->setEnabled( true );
    d->DisplayGroupBox->setEnabled( true );
    
    // match the low-level widgets to the MRML node
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
    d->TubeSidesSpinBox->setValue( MarkupsToModelNode->GetTubeNumberOfSides() );
    d->TubeSegmentsSpinBox->setValue( MarkupsToModelNode->GetTubeSamplePointsBetweenControlPoints() );
    switch( MarkupsToModelNode->GetInterpolationType() )
    {
    case vtkMRMLMarkupsToModelNode::Linear: d->LinearInterpolationRadioButton->setChecked( 1 ); break;
    case vtkMRMLMarkupsToModelNode::CardinalSpline: d->CardinalInterpolationRadioButton->setChecked( 1 ); break;
    case vtkMRMLMarkupsToModelNode::KochanekSpline: d->KochanekInterpolationRadioButton->setChecked( 1 ); break;
    case vtkMRMLMarkupsToModelNode::Polynomial: d->PolynomialInterpolationRadioButton->setChecked( 1 ); break;
    }
    d->KochanekBiasDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekBias());
    d->KochanekContinuityDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekContinuity());
    d->KochanekTensionDoubleSpinBox->setValue(MarkupsToModelNode->GetKochanekTension());
    switch( MarkupsToModelNode->GetPointParameterType() )
    {
    case vtkMRMLMarkupsToModelNode::RawIndices: d->PointParameterRawIndicesRadioButton->setChecked( 1 ); break;
    case vtkMRMLMarkupsToModelNode::MinimumSpanningTree: d->PointParameterMinimumSpanningTreeRadioButton->setChecked( 1 ); break;
    }
    d->PolynomialOrderSpinBox->setValue(MarkupsToModelNode->GetPolynomialOrder());

    updateFromRenderedNodes();
  }
  else
  {
    d->UpdateButton->setEnabled( false );
    d->AdvancedGroupBox->setEnabled( false );
    d->DisplayGroupBox->setEnabled( false );
  }  

  // determine visibility of widgets
  if( d->ModeClosedSurfaceRadioButton->isChecked() )
  {
    d->ButterflySubdivisionLabel->setVisible( true );
    d->ButterflySubdivisionCheckBox->setVisible( true );
    d->DelaunayAlphaLabel->setVisible( true );
    d->DelaunayAlphaDoubleSpinBox->setVisible( true );
    d->ConvexHullLabel->setVisible( true );
    d->ConvexHullCheckBox->setVisible( true );

    d->InterpolationGroupBox->setVisible(false);
    d->InterpolationLabel->setVisible(false);
    d->LinearInterpolationRadioButton->setVisible(false);
    d->CardinalInterpolationRadioButton->setVisible(false);
    d->KochanekInterpolationRadioButton->setVisible(false);
    d->PolynomialInterpolationRadioButton->setVisible(false);
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
    d->PointParameterLabel->setVisible( false );
    d->PointParameterGroupBox->setVisible( false );
    d->PointParameterRawIndicesRadioButton->setVisible( false );
    d->PointParameterMinimumSpanningTreeRadioButton->setVisible( false );
    d->PolynomialOrderLabel->setVisible( false );
    d->PolynomialOrderSpinBox->setVisible( false );
  }
  else
  {
    d->ButterflySubdivisionLabel->setVisible( false );
    d->ButterflySubdivisionCheckBox->setVisible( false );
    d->DelaunayAlphaLabel->setVisible( false );
    d->DelaunayAlphaDoubleSpinBox->setVisible( false );
    d->ConvexHullLabel->setVisible( false );
    d->ConvexHullCheckBox->setVisible( false );

    d->InterpolationGroupBox->setVisible(true);
    d->InterpolationLabel->setVisible(true);
    d->LinearInterpolationRadioButton->setVisible(true);
    d->CardinalInterpolationRadioButton->setVisible(true);
    d->KochanekInterpolationRadioButton->setVisible(true);
    d->PolynomialInterpolationRadioButton->setVisible(true);
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
    
    if(d->PolynomialInterpolationRadioButton->isChecked())
    {
      d->PointParameterLabel->setVisible( true );
      d->PointParameterGroupBox->setVisible( true );
      d->PointParameterRawIndicesRadioButton->setVisible( true );
      d->PointParameterMinimumSpanningTreeRadioButton->setVisible( true );
      d->PolynomialOrderLabel->setVisible( true );
      d->PolynomialOrderSpinBox->setVisible( true );
    }
    else
    {
      d->PointParameterLabel->setVisible( false );
      d->PointParameterGroupBox->setVisible( false );
      d->PointParameterRawIndicesRadioButton->setVisible( false );
      d->PointParameterMinimumSpanningTreeRadioButton->setVisible( false );
      d->PolynomialOrderLabel->setVisible( false );
      d->PolynomialOrderSpinBox->setVisible( false );
    }
  }

  this->unblockAllSignals();
  
  // the parameters have possibly changed, so also update the model (if automatic update is on)
  if( MarkupsToModelNode->GetAutoUpdateOutput() )
  {
    UpdateOutputModel();
  }
}

void qSlicerMarkupsToModelModuleWidget::updateFromRenderedNodes()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  // display options
  d->ModelVisiblityButton->setChecked( this->GetOutputVisibility() );
  d->ModelOpacitySlider->setValue( this->GetOutputOpacity() );
  double outputColor[3];
  this->GetOutputColor( outputColor );
  QColor nodeOutputColor;
  nodeOutputColor.setRgbF( outputColor[0],outputColor[1],outputColor[2] );
  d->ModelColorSelector->setColor( nodeOutputColor );
  d->ModelSliceIntersectionCheckbox->setChecked( this->GetOutputIntersectionVisibility() );
  d->MarkupsTextScaleSlider->setValue( this->GetMarkupsTextScale() );
}

void qSlicerMarkupsToModelModuleWidget::updateToRenderedNodes()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  this->blockAllSignals();
  this->SetOutputVisibility( d->ModelVisiblityButton->isChecked() );
  this->SetOutputOpacity( d->ModelOpacitySlider->value() );
  this->SetOutputColor( d->ModelColorSelector->color().redF(), d->ModelColorSelector->color().greenF(), d->ModelColorSelector->color().blueF() );
  this->SetOutputIntersectionVisibility( d->ModelSliceIntersectionCheckbox->isChecked() );
  this->SetMarkupsTextScale( d->MarkupsTextScaleSlider->value() );
  this->unblockAllSignals();
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
  d->LinearInterpolationRadioButton->blockSignals( true );
  d->CardinalInterpolationRadioButton->blockSignals( true );
  d->KochanekInterpolationRadioButton->blockSignals( true );
  d->PolynomialInterpolationRadioButton->blockSignals( true );
  d->KochanekBiasDoubleSpinBox->blockSignals( true );
  d->KochanekContinuityDoubleSpinBox->blockSignals( true );
  d->KochanekTensionDoubleSpinBox->blockSignals( true );
  d->PointParameterRawIndicesRadioButton->blockSignals( true );
  d->PointParameterMinimumSpanningTreeRadioButton->blockSignals( true );
  d->PolynomialOrderSpinBox->blockSignals( true );

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
  d->LinearInterpolationRadioButton->blockSignals( false );
  d->CardinalInterpolationRadioButton->blockSignals( false );
  d->KochanekInterpolationRadioButton->blockSignals( false );
  d->PolynomialInterpolationRadioButton->blockSignals( false );
  d->KochanekBiasDoubleSpinBox->blockSignals( false );
  d->KochanekContinuityDoubleSpinBox->blockSignals( false );
  d->KochanekTensionDoubleSpinBox->blockSignals( false );
  d->PointParameterRawIndicesRadioButton->blockSignals( false );
  d->PointParameterMinimumSpanningTreeRadioButton->blockSignals( false );
  d->PolynomialOrderSpinBox->blockSignals( false );

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

//-----------------------------------------------------------------------------

vtkMRMLModelNode* qSlicerMarkupsToModelModuleWidget::GetModelNode( )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return NULL;
  }
  return markupsToModelNode->GetModelNode();
}

vtkMRMLModelDisplayNode* qSlicerMarkupsToModelModuleWidget::GetModelDisplayNode( )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLModelNode* modelNode = GetModelNode();
  if (modelNode == NULL)
  {
    qCritical( "Cannot get Model Node" );
    return NULL;
  }
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if (modelDisplayNode == NULL) // if there is no display node, create one
  {
    modelNode->CreateDefaultDisplayNodes();
    vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
    modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
    modelDisplayNode->SetName( vtkSlicerMarkupsToModelLogic::GetModelDisplayNodeName(markupsToModelNode).c_str() );
    modelDisplayNode->SetColor( 0.5, 0.5, 0.5 );
  }
  return modelDisplayNode;
}

vtkMRMLMarkupsFiducialNode* qSlicerMarkupsToModelModuleWidget::GetMarkupsNode( )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
  if ( markupsToModelNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return NULL;
  }
  return markupsToModelNode->GetMarkupsNode();
}

vtkMRMLMarkupsDisplayNode* qSlicerMarkupsToModelModuleWidget::GetMarkupsDisplayNode( )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsFiducialNode* markupsNode = GetMarkupsNode();
  if (markupsNode == NULL)
  {
    qCritical( "Cannot get Markups Node" );
    return NULL;
  }
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  if (markupsDisplayNode == NULL) // if there is no display node, create one
  {
    markupsNode->CreateDefaultDisplayNodes();
    vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ParameterNodeSelector->currentNode() );
    markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
    markupsDisplayNode->SetName( vtkSlicerMarkupsToModelLogic::GetModelDisplayNodeName(markupsToModelNode).c_str() );
    markupsDisplayNode->SetColor( 1.0, 0.5, 0.5 );
  }
  return markupsDisplayNode;
}

//-----------------------------------------------------------------------------

void qSlicerMarkupsToModelModuleWidget::SetOutputOpacity( double outputOpacity )
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return;
  displayNode->SetOpacity( outputOpacity );
}

void qSlicerMarkupsToModelModuleWidget::SetOutputVisibility( bool outputVisibility )
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return;
  if ( outputVisibility )
    displayNode->VisibilityOn();
  else
    displayNode->VisibilityOff();
}

void qSlicerMarkupsToModelModuleWidget::SetOutputIntersectionVisibility( bool outputIntersectionVisibility )
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return;
  if ( outputIntersectionVisibility )
    displayNode->SliceIntersectionVisibilityOn();
  else
    displayNode->SliceIntersectionVisibilityOff();
}

void qSlicerMarkupsToModelModuleWidget::SetOutputColor( double redComponent, double greenComponent, double blueComponent )
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return;
  double outputColor[3];
  outputColor[0] = redComponent;
  outputColor[1] = greenComponent;
  outputColor[2] = blueComponent;
  displayNode->SetColor( outputColor );
}

void qSlicerMarkupsToModelModuleWidget::SetMarkupsTextScale( double scale )
{
  vtkMRMLMarkupsDisplayNode * displayNode = GetMarkupsDisplayNode();
  if (displayNode == NULL)
    return;
  displayNode->SetTextScale( scale );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double qSlicerMarkupsToModelModuleWidget::GetOutputOpacity()
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return 0.0;
  return displayNode->GetOpacity();
}

bool qSlicerMarkupsToModelModuleWidget::GetOutputVisibility()
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return false;
  return displayNode->GetVisibility();  
}

bool qSlicerMarkupsToModelModuleWidget::GetOutputIntersectionVisibility()
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return false;
  return displayNode->GetSliceIntersectionVisibility();
}

void qSlicerMarkupsToModelModuleWidget::GetOutputColor( double outputColor[3] )
{
  vtkMRMLModelDisplayNode* displayNode = GetModelDisplayNode();
  if (displayNode == NULL)
    return;
  displayNode->GetColor( outputColor[0], outputColor[1], outputColor[2] );
}

double qSlicerMarkupsToModelModuleWidget::GetMarkupsTextScale()
{
  vtkMRMLMarkupsDisplayNode* displayNode = GetMarkupsDisplayNode();
  if (displayNode == NULL)
    return 0.0;
  return displayNode->GetTextScale();
}


