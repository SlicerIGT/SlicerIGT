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
#include <QDebug>
#include <QTabWidget>

#include "qSlicerApplication.h"

// SlicerQt includes
#include "qSlicerMarkupsToModelModuleWidget.h"
#include "ui_qSlicerMarkupsToModelModuleWidget.h"

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
  disconnect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeChanged() ) );
  disconnect( d->ModuleNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeAddedByUser( vtkMRMLNode* ) ) );

  disconnect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );

  disconnect( d->MarkupsNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onCurrentMarkupsNodeChanged() ) );
  disconnect( d->MarkupsNodeComboBox, SIGNAL( nodeAboutToBeEdited( vtkMRMLNode* ) ), this, SLOT( onNodeAboutToBeEdited( vtkMRMLNode* ) ) );

  disconnect( d->UpdateOutputModelPushButton, SIGNAL( clicked() ) , this, SLOT( onUpdateOutputModelPushButton() ) );
  disconnect( d->DeleteAllPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteAllPushButton() ) );
  disconnect( d->DeleteLastPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteLastModelPushButton() ) );
  disconnect( d->PlacePushButton, SIGNAL( clicked() ), this, SLOT( onPlacePushButtonClicked() ) ); 

  disconnect( d->AutoUpdateOutputCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onAutoUpdateOutputToggled( bool ) ) );

  disconnect( d->CleanMarkupsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onCleanMarkupsToggled( bool ) ) );
  disconnect( d->ClosedSurfaceRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );
  disconnect( d->CurveRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );

  disconnect( d->NoFilterRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onSmoothingFilterGroupBoxClicked( bool ) ) );
  disconnect( d->NormalsFilterRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onSmoothingFilterGroupBoxClicked( bool ) ) );
  disconnect( d->ButterflyFilterRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onSmoothingFilterGroupBoxClicked( bool ) ) );

  disconnect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onDelaunayAlphaDoubleChanged( double ) ) );
  disconnect( d->ConvexHullCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onConvexHullToggled( bool ) ) );

  disconnect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onTubeRadiusDoubleChanged( double ) ) );
  disconnect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekBiasDoubleChanged( double ) ) );
  disconnect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekContinuityDoubleChanged( double ) ) );
  disconnect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekTensionDoubleChanged( double ) ) );

  disconnect( d->OutputOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( onOutputOpacityValueChanged( double ) ) );
  disconnect( d->OutputColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( onOutputColorChanged( QColor ) ) );
  disconnect( d->OutputVisiblityButton, SIGNAL( toggled( bool ) ), this, SLOT( onOutputVisibilityToggled( bool ) ) );
  disconnect( d->OutputIntersectionVisibilityCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onOutputIntersectionVisibilityToggled( bool ) ) );

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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }

  this->qvtkConnect( markupsToModelModuleNode, vtkMRMLMarkupsToModelNode::InputDataModifiedEvent, this, SLOT( updateFromMRMLNode() ) );

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );
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

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeChanged() ) );
  connect( d->ModuleNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeAddedByUser( vtkMRMLNode* ) ) );

  connect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );

  connect( d->MarkupsNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onCurrentMarkupsNodeChanged() ) );
  connect( d->MarkupsNodeComboBox, SIGNAL( nodeAboutToBeEdited( vtkMRMLNode* ) ), this, SLOT( onNodeAboutToBeEdited( vtkMRMLNode* ) ) );

  connect( d->UpdateOutputModelPushButton, SIGNAL( clicked() ) , this, SLOT( onUpdateOutputModelPushButton() ) );
  connect( d->DeleteAllPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteAllPushButton() ) );
  d->DeleteAllPushButton->setIcon( QIcon( ":/Icons/MarkupsDeleteAllRows.png" ) );
  connect( d->DeleteLastPushButton, SIGNAL( clicked() ) , this, SLOT( onDeleteLastModelPushButton() ) );
  d->DeleteLastPushButton->setIcon( QIcon( ":/Icons/MarkupsDeleteLast.png" ) );
  connect( d->PlacePushButton, SIGNAL( clicked() ), this, SLOT( onPlacePushButtonClicked() ) ); 
  d->PlacePushButton->setIcon( QIcon( ":/Icons/MarkupsMouseModePlace.png" ) );

  connect( d->AutoUpdateOutputCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onAutoUpdateOutputToggled( bool ) ) );
  connect( d->ConvexHullCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onConvexHullToggled( bool ) ) );

  connect( d->CleanMarkupsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onCleanMarkupsToggled( bool ) ) );

  d->CleanMarkupsCheckBox->setToolTip(QString("It will merge duplicate  points.  Duplicate points are the ones closer than tolerance distance. The tolerance distance is 1% diagonal size bounding box of total points."));
  
  connect( d->ClosedSurfaceRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );
  connect( d->CurveRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onModeGroupBoxClicked( bool ) ) );

  connect( d->NoFilterRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onSmoothingFilterGroupBoxClicked( bool ) ) );
  connect( d->NormalsFilterRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onSmoothingFilterGroupBoxClicked( bool ) ) );
  connect( d->ButterflyFilterRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( onSmoothingFilterGroupBoxClicked( bool ) ) );

  connect( d->DelaunayAlphaDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onDelaunayAlphaDoubleChanged( double ) ) );
  connect( d->TubeRadiusDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onTubeRadiusDoubleChanged( double ) ) );
  
  connect( d->KochanekBiasDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekBiasDoubleChanged( double ) ) );
  connect( d->KochanekContinuityDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekContinuityDoubleChanged( double ) ) );
  connect( d->KochanekTensionDoubleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( onKochanekTensionDoubleChanged( double ) ) );

  connect( d->OutputOpacitySlider, SIGNAL( valueChanged( double ) ), this, SLOT( onOutputOpacityValueChanged( double ) ) );
  connect( d->OutputColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( onOutputColorChanged( QColor ) ) );
  connect( d->OutputVisiblityButton, SIGNAL( toggled( bool ) ), this, SLOT( onOutputVisibilityToggled( bool ) ) );
  connect( d->OutputIntersectionVisibilityCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onOutputIntersectionVisibilityToggled( bool ) ) );
  connect( d->TextScaleSlider, SIGNAL( valueChanged( double ) ), this, SLOT( onTextScaleChanged( double ) ) );

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
  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    d->ModuleNodeComboBox->setCurrentNodeID( node->GetID() );
  }
  onModeGroupBoxClicked( true );
  onSmoothingFilterGroupBoxClicked( true );

  this->previouslyPersistent = qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->GetPlaceModePersistence();
  qSlicerApplication::application()->applicationLogic()->GetInteractionNode()->SetPlaceModePersistence(1);

  //TODO UPDATE selectionNode according to markups toolbox when markups module enter selected
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  this->qvtkConnect( markupsToModelModuleNode, vtkMRMLMarkupsToModelNode::InputDataModifiedEvent, this, SLOT( updateFromMRMLNode() ) );

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );
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
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateWidget()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLNode* currentModuleNode = d->ModuleNodeComboBox->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->MarkupsNodeComboBox->setCurrentNodeID( "" );
    d->MarkupsNodeComboBox->setEnabled( false );
    return;
  }
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentModuleNode );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  vtkMRMLDisplayableNode* currentMarkupsNode = vtkMRMLDisplayableNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );
  d->DeleteAllPushButton->blockSignals( true );
  d->DeleteLastPushButton->blockSignals( true );
  d->UpdateOutputModelPushButton->blockSignals( true );
  if ( currentMarkupsNode == NULL )
  {
    d->DeleteAllPushButton->setChecked( Qt::Unchecked );
    d->DeleteLastPushButton->setChecked( Qt::Unchecked );
    d->UpdateOutputModelPushButton->setChecked( Qt::Unchecked );
    // This will ensure that we refresh the widget next time we move to a non-null widget ( since there is guaranteed to be a modified status of larger than zero )
    d->DeleteAllPushButton->setEnabled( false );
    d->DeleteLastPushButton->setEnabled( false );
    d->UpdateOutputModelPushButton->setEnabled( false );
  }
  else
  {
    // Set the button indicating if this list is active
    if ( markupsToModelModuleNode->GetMarkupsNode()!=NULL && markupsToModelModuleNode->GetMarkupsNode()->GetNumberOfFiducials() > 0 )
    {
      d->DeleteAllPushButton->setEnabled( true );
      d->DeleteLastPushButton->setEnabled( true );
      if( markupsToModelModuleNode->GetMarkupsNode()->GetNumberOfFiducials() >= MINIMUM_MARKUPS_NUMBER )
      {
        d->UpdateOutputModelPushButton->setEnabled( true );
      }
      else
      {
        d->UpdateOutputModelPushButton->setEnabled( false );
      }
    }
    else
    {
      d->DeleteAllPushButton->setEnabled( false );
      d->DeleteLastPushButton->setEnabled( false );
      d->UpdateOutputModelPushButton->setEnabled( false );
    }
  }
  d->DeleteAllPushButton->blockSignals( false );
  d->DeleteLastPushButton->blockSignals( false );
  d->UpdateOutputModelPushButton->blockSignals( false );
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateFromMRMLNode()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->MarkupsNodeComboBox->setCurrentNodeID( "" );
    d->MarkupsNodeComboBox->setEnabled( false );
    return;
  }
  vtkMRMLMarkupsToModelNode* MarkupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentNode );
  if ( MarkupsToModelNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  d->MarkupsNodeComboBox->setEnabled( true );

  if ( MarkupsToModelNode->GetMarkupsNode() != NULL )
  {
    d->MarkupsNodeComboBox->setCurrentNodeID( MarkupsToModelNode->GetMarkupsNode()->GetID() );
  }
  else
  {
    d->MarkupsNodeComboBox->setCurrentNodeIndex( 0 );
  }

  if ( MarkupsToModelNode->GetModelNode() != NULL )
  {
    d->ModelNodeComboBox->setCurrentNodeID( MarkupsToModelNode->GetModelNode()->GetID() );
  }
  else
  {
    d->ModelNodeComboBox->setCurrentNodeID( "None" );
  }
  d->AutoUpdateOutputCheckBox->setChecked( MarkupsToModelNode->GetAutoUpdateOutput() );

  d->OutputOpacitySlider->setValue( MarkupsToModelNode->GetOutputOpacity() );
  double outputColor[3];
  MarkupsToModelNode->GetOutputColor( outputColor );
  QColor nodeOutputColor;
  nodeOutputColor.setRgbF( outputColor[0],outputColor[1],outputColor[2] );
  d->OutputColorPickerButton->setColor( nodeOutputColor );
  d->OutputVisiblityButton->setChecked( MarkupsToModelNode->GetOutputVisibility() );
  d->OutputIntersectionVisibilityCheckBox->setChecked( MarkupsToModelNode->GetOutputIntersectionVisibility() );

  d->CleanMarkupsCheckBox->setChecked( MarkupsToModelNode->GetCleanMarkups() );

  switch( MarkupsToModelNode->GetModelType() )
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: 
    d->ClosedSurfaceRadioButton->setChecked( 1 );
    switch( MarkupsToModelNode->GetSmoothingType() )
    {
    case vtkMRMLMarkupsToModelNode::NoFilter:
      d->NoFilterRadioButton->setChecked( 1 );
      d->NormalsFilterRadioButton->setChecked( 0 );
      d->ButterflyFilterRadioButton->setChecked( 0 );
      d->ConvexHullCheckBox->setChecked( 0 );
      break;
    case vtkMRMLMarkupsToModelNode::NormalsFilter:
      d->NoFilterRadioButton->setChecked( 0 );
      d->NormalsFilterRadioButton->setChecked( 1 );
      d->ButterflyFilterRadioButton->setChecked( 0 );
      d->ConvexHullCheckBox->setChecked( 0 );
      break;
    case vtkMRMLMarkupsToModelNode::ButterflyFilter:
      d->NoFilterRadioButton->setChecked( 0 );
      d->NormalsFilterRadioButton->setChecked( 0 );
      d->ButterflyFilterRadioButton->setChecked( 1 );
      d->ConvexHullCheckBox->setChecked( 1 );
      d->DelaunayAlphaDoubleSpinBox->setValue( MarkupsToModelNode->GetDelaunayAlpha() );
      break;
    }
    break;

  case vtkMRMLMarkupsToModelNode::Curve: d->CurveRadioButton->setChecked( 1 );
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
  
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
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

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( d->ModelNodeComboBox->currentNode() );
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

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onMarkupsToModelModuleNodeChanged()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  d->logic()->UpdateSelectionNode( markupsToModelModuleNode );
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onMarkupsToModelModuleNodeAddedByUser( vtkMRMLNode* nodeAdded )
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Conversion Mode changed with no module node selection" );
    return;
  }

  if( d->ClosedSurfaceRadioButton->isChecked() )
  {
    d->ModeTab->setCurrentIndex(0);
    markupsToModelModuleNode->SetModelType( vtkMRMLMarkupsToModelNode::ClosedSurface );
  }
  else
  {
    d->ModeTab->setCurrentIndex(1);
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
void qSlicerMarkupsToModelModuleWidget::onSmoothingFilterGroupBoxClicked( bool nana )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Conversion Mode changed with no module node selection" );
    return;
  }

  if ( d->ButterflyFilterRadioButton->isChecked() )
  {
    d->ConvexHullCheckBox->setVisible( true );
    d->DelaunayAlphaLabel->setVisible( true );
    d->DelaunayAlphaDoubleSpinBox->setVisible( true );
    markupsToModelModuleNode->SetSmoothingType( vtkMRMLMarkupsToModelNode::ButterflyFilter );
  }
  else
  {
    d->ConvexHullCheckBox->setVisible( false );
    d->DelaunayAlphaLabel->setVisible( false );
    d->DelaunayAlphaDoubleSpinBox->setVisible( false );
    if (d->NormalsFilterRadioButton->isChecked() )
    {
      markupsToModelModuleNode->SetSmoothingType( vtkMRMLMarkupsToModelNode::NormalsFilter );
    }
    else
    {
      markupsToModelModuleNode->SetSmoothingType( vtkMRMLMarkupsToModelNode::NoFilter );
    }
  }

  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onTubeRadiusDoubleChanged( double tubeRadius )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );
  if ( markupsNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  vtkMRMLMarkupsDisplayNode * markupsDisplay = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  markupsDisplay->SetTextScale(textScale);
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onOutputIntersectionVisibilityToggled( bool outputIntersectionVisibility )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetCleanMarkups( cleanMarkups );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onConvexHullToggled( bool convexHull )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  markupsToModelModuleNode->SetConvexHull( convexHull );
  UpdateOutputModel();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onAutoUpdateOutputToggled( bool autoUpdateOutput )
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( d->MarkupsNodeComboBox->currentNode() );

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
  }

  if ( currentMarkupsNode != NULL )
  {
    d->logic()->MarkupsLogic->SetActiveListID( currentMarkupsNode ); // If there are other widgets, they are responsible for updating themselves
  }

  this->updateWidget();
}


