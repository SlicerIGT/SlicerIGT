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
#include "qSlicerCollectPointsModuleWidget.h"
#include "ui_qSlicerCollectPointsModule.h"

// module includes
#include "vtkSlicerCollectPointsLogic.h"
#include "vtkMRMLCollectPointsNode.h"

// slicer includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelDisplayNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CollectPoints
class qSlicerCollectPointsModuleWidgetPrivate: public Ui_qSlicerCollectPointsModule
{
  Q_DECLARE_PUBLIC( qSlicerCollectPointsModuleWidget ); 
  
protected:
  qSlicerCollectPointsModuleWidget* const q_ptr;
public:
  qSlicerCollectPointsModuleWidgetPrivate( qSlicerCollectPointsModuleWidget& object );
  vtkSlicerCollectPointsLogic* logic() const;
  QMenu* OutputDeleteMenu;
};

//-----------------------------------------------------------------------------
// qSlicerCollectPointsModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerCollectPointsModuleWidgetPrivate::qSlicerCollectPointsModuleWidgetPrivate( qSlicerCollectPointsModuleWidget& object )
: q_ptr( &object )
{
}

//-----------------------------------------------------------------------------
vtkSlicerCollectPointsLogic* qSlicerCollectPointsModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerCollectPointsModuleWidget );
  return vtkSlicerCollectPointsLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerCollectPointsModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerCollectPointsModuleWidget::qSlicerCollectPointsModuleWidget( QWidget* _parent )
: Superclass( _parent ), d_ptr( new qSlicerCollectPointsModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerCollectPointsModuleWidget::~qSlicerCollectPointsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::setup()
{
  Q_D( qSlicerCollectPointsModuleWidget );
  d->setupUi( this );
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ParameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeSelected() ) );
  connect( d->ProbeTransformNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onProbeTransformNodeSelected() ) );
  connect( d->OutputNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onOutputNodeSelected( vtkMRMLNode* ) ) );
  connect( d->OutputNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onOutputNodeAdded( vtkMRMLNode* ) ) );
  connect( d->OutputColorButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( onColorButtonChanged( QColor ) ) );
  connect( d->OutputDeleteButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked() ) );
  d->OutputDeleteMenu = new QMenu(tr( "Delete options" ), d->OutputDeleteButton );
  d->OutputDeleteMenu->setObjectName( "DeleteMenu" );
  d->OutputDeleteMenu->addAction( d->ActionDeleteAll );
  QObject::connect( d->ActionDeleteAll, SIGNAL( triggered() ), this, SLOT( onDeleteAllClicked() ) );
  d->OutputDeleteButton->setMenu( d->OutputDeleteMenu );
  connect( d->OutputVisibilityButton, SIGNAL( clicked() ), this, SLOT( onVisibilityButtonClicked() ) );

  connect( d->LabelBaseLineEdit, SIGNAL( editingFinished() ), this, SLOT( onLabelBaseChanged() ) );
  connect( d->LabelCounterSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( onLabelCounterChanged() ) );
  connect( d->MinimumDistanceSlider, SIGNAL( valueChanged( double ) ), this, SLOT( onMinimumDistanceChanged() ) );
  connect( d->CollectButton, SIGNAL( clicked() ), this, SLOT( onCollectClicked() ) );
  connect( d->CollectButton, SIGNAL( checkBoxToggled( bool ) ), this, SLOT( onCollectCheckboxToggled() ) );
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::enter()
{
  Q_D( qSlicerCollectPointsModuleWidget );
  this->Superclass::enter();

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default parameter node.
  if ( d->ParameterNodeComboBox->currentNode() == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLCollectPointsNode" );
    // Create a new parameter node if there is none in the scene.
    if ( node == NULL )
    {
      node = this->mrmlScene()->AddNewNodeByClass( "vtkMRMLCollectPointsNode" );
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
void qSlicerCollectPointsModuleWidget::exit()
{
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::blockAllSignals( bool block )
{
  Q_D( qSlicerCollectPointsModuleWidget );
  d->ParameterNodeComboBox->blockSignals( block );
  d->ProbeTransformNodeComboBox->blockSignals( block );
  d->OutputNodeComboBox->blockSignals( block );
  d->OutputDeleteButton->blockSignals( block );
  d->OutputColorButton->blockSignals( block );
  d->OutputVisibilityButton->blockSignals( block );
  d->LabelBaseLineEdit->blockSignals( block );
  d->LabelCounterSpinBox->blockSignals( block );
  d->MinimumDistanceSlider->blockSignals( block );
  d->CollectButton->blockSignals( block );
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::enableAllWidgets( bool enable )
{
  Q_D( qSlicerCollectPointsModuleWidget );
  // don't ever disable parameter node, need to be able to select between them
  d->ProbeTransformNodeComboBox->setEnabled( enable );
  d->OutputNodeComboBox->setEnabled( enable );
  d->OutputDeleteButton->setEnabled( enable );
  d->OutputColorButton->setEnabled( enable );
  d->OutputVisibilityButton->setEnabled( enable );
  d->LabelBaseLineEdit->setEnabled( enable );
  d->LabelCounterSpinBox->setEnabled( enable );
  d->MinimumDistanceSlider->setEnabled( enable );
  d->CollectButton->setEnabled( enable );
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerCollectPointsModuleWidget );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportedEvent() ) );
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onSceneImportedEvent()
{
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onParameterNodeSelected()
{
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onProbeTransformNodeSelected()
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  vtkMRMLLinearTransformNode* probeTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->ProbeTransformNodeComboBox->currentNode() );
  if( probeTransformNode != NULL )
  {
    collectPointsNode->SetAndObserveProbeTransformNodeId( probeTransformNode->GetID() );
  }
  else
  {
    collectPointsNode->SetAndObserveProbeTransformNodeId( NULL );
  }

  this->updateGUIFromMRML();
}


//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onOutputNodeAdded( vtkMRMLNode* newNode )
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLMarkupsNode* outputMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( newNode );
  if ( outputMarkupsNode != NULL )
  {
    outputMarkupsNode->CreateDefaultDisplayNodes();
  }

  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( newNode );
  if ( outputModelNode != NULL )
  {
    // display node should be set to show points only
    outputModelNode->CreateDefaultDisplayNodes();
    vtkMRMLModelDisplayNode* outputModelDisplayNode = outputModelNode->GetModelDisplayNode();
    outputModelDisplayNode->SetRepresentation( vtkMRMLDisplayNode::PointsRepresentation );
    const int DEFAULT_POINT_SIZE = 5; // This could be made an advanced display setting in the GUI
    outputModelDisplayNode->SetPointSize( DEFAULT_POINT_SIZE );
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onOutputNodeSelected( vtkMRMLNode* newNode )
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  if ( newNode == NULL )
  {
    collectPointsNode->SetOutputNodeId( NULL );
  }
  else
  {
    collectPointsNode->SetOutputNodeId( newNode->GetID() );
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onColorButtonChanged( QColor qcolor )
{
  Q_D( qSlicerCollectPointsModuleWidget );
  
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( collectPointsNode->GetOutputNode() );
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( collectPointsNode->GetOutputNode() );
  if ( outputMarkupsNode != NULL )
  {
    vtkMRMLMarkupsDisplayNode* outputDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( outputMarkupsNode->GetMarkupsDisplayNode() );
    if ( outputDisplayNode != NULL )
    {
      double r = ( double )qcolor.redF();
      double g = ( double )qcolor.greenF();
      double b = ( double )qcolor.blueF();
      outputDisplayNode->SetSelectedColor( r, g, b );
    }
  }
  else if ( outputModelNode != NULL )
  {
    vtkMRMLModelDisplayNode* outputDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( outputModelNode->GetModelDisplayNode() );
    if ( outputDisplayNode != NULL )
    {
      double r = ( double )qcolor.redF();
      double g = ( double )qcolor.greenF();
      double b = ( double )qcolor.blueF();
      outputDisplayNode->SetColor( r, g, b );
    }
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onDeleteButtonClicked()
{
  Q_D( qSlicerCollectPointsModuleWidget );
  
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  d->logic()->RemoveLastPoint( collectPointsNode );

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onDeleteAllClicked()
{
  Q_D( qSlicerCollectPointsModuleWidget );
  
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  d->logic()->RemoveAllPoints( collectPointsNode );

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onVisibilityButtonClicked()
{
  Q_D( qSlicerCollectPointsModuleWidget );
  
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  vtkMRMLDisplayableNode* outputDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast( collectPointsNode->GetOutputNode() );
  if ( outputDisplayableNode == NULL )
  {
    return;
  }

  vtkMRMLDisplayNode* outputDisplayNode = vtkMRMLDisplayNode::SafeDownCast( outputDisplayableNode->GetDisplayNode() );
  if ( outputDisplayNode == NULL )
  {
    return;
  }

  int newVisibility = ! outputDisplayNode->GetVisibility();
  outputDisplayNode->SetVisibility( newVisibility );

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onLabelBaseChanged()
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  collectPointsNode->SetLabelBase( d->LabelBaseLineEdit->text().toStdString() );
  
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onLabelCounterChanged()
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  collectPointsNode->SetLabelCounter( d->LabelCounterSpinBox->value() );
  
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onMinimumDistanceChanged()
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  collectPointsNode->SetMinimumDistanceMm( d->MinimumDistanceSlider->value() );
  
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onCollectClicked()
{
  Q_D( qSlicerCollectPointsModuleWidget );
  
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  if ( d->CollectButton->checkState() == Qt::Checked )
  {
    // If Collect button is untoggled then make it unchecked, too
    d->CollectButton->setCheckState( Qt::Unchecked );
  }

  d->logic()->AddPoint( collectPointsNode );

  this->updateGUIFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::onCollectCheckboxToggled()
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    // should not be able to change parameters in a non-existant node...
    this->enableAllWidgets( false );
    return;
  }

  if ( d->CollectButton->checkState() == Qt::Checked )
  {
    collectPointsNode->SetCollectModeToAutomatic();
  }
  else
  {
    collectPointsNode->SetCollectModeToManual();
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModuleWidget::updateGUIFromMRML()
{
  Q_D( qSlicerCollectPointsModuleWidget );

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    // should not be able to change parameters in a non-existant node...
    this->enableAllWidgets( false );
    return;
  }

  // if a valid node is selected, then the various parameter widgets should be enabled
  this->enableAllWidgets( true );

  // temporarily block signals so nothing gets triggered when updating the GUI
  this->blockAllSignals( true );

  // update all widget contents
  d->ProbeTransformNodeComboBox->setCurrentNode( collectPointsNode->GetProbeTransformNode() );
  d->OutputNodeComboBox->setCurrentNode( collectPointsNode->GetOutputNode() );
  // output node buttons - disabled unless a valid output node is selected
  d->OutputColorButton->setEnabled( false );
  d->OutputColorButton->setColor( QColor() ); // default color
  d->OutputVisibilityButton->setEnabled( false );
  d->OutputDeleteButton->setEnabled( false );
  vtkMRMLDisplayableNode* outputDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast( collectPointsNode->GetOutputNode() );
  if ( outputDisplayableNode != NULL )
  {
    vtkMRMLDisplayNode* outputDisplayNode = outputDisplayableNode->GetDisplayNode();
    if ( outputDisplayNode != NULL )
    {
      if ( outputDisplayNode->GetVisibility() )
      {
        d->OutputVisibilityButton->setIcon( QIcon( ":/Icons/PointVisible.png" ) );
      }
      else
      {
        d->OutputVisibilityButton->setIcon( QIcon( ":/Icons/PointInvisible.png" ) );
      }
      double colorDouble[ 3 ];
      outputDisplayNode->GetSelectedColor( colorDouble );
      QColor colorQColor;
      colorQColor.setRgbF( colorDouble[ 0 ], colorDouble[ 1 ], colorDouble[ 2 ] );
      d->OutputColorButton->setColor( colorQColor );
      d->OutputColorButton->setEnabled( true );
      d->OutputVisibilityButton->setEnabled( true );
      if ( collectPointsNode->GetNumberOfPointsInOutput() > 0 )
      {
        d->OutputDeleteButton->setEnabled( true );
      }
    }
  }

  d->LabelCounterSpinBox->setValue( collectPointsNode->GetLabelCounter() );
  d->LabelBaseLineEdit->setText( collectPointsNode->GetLabelBase().c_str() );
  d->MinimumDistanceSlider->setValue( collectPointsNode->GetMinimumDistanceMm() );

  bool readyToCollect = collectPointsNode->GetProbeTransformNode() != NULL && collectPointsNode->GetOutputNode() != NULL;
  d->CollectButton->setEnabled( readyToCollect );
  if ( collectPointsNode->GetCollectMode() == vtkMRMLCollectPointsNode::Automatic )
  {
    d->CollectButton->setText( tr( "Auto-Collect" ) );
    d->CollectButton->setCheckable( true );
    d->CollectButton->setChecked( true );
  }
  else
  {
    d->CollectButton->setText( tr( "Collect" ) );
    d->CollectButton->setCheckable( false );
  }

  // update widget visibility
  bool isInputMarkupsNode = ( vtkMRMLMarkupsNode::SafeDownCast( collectPointsNode->GetOutputNode() ) != NULL );

  d->LabelCounterLabel->setVisible( isInputMarkupsNode );
  d->LabelCounterSpinBox->setVisible( isInputMarkupsNode );
  d->LabelBaseLabel->setVisible( isInputMarkupsNode );
  d->LabelBaseLineEdit->setVisible( isInputMarkupsNode );

  this->blockAllSignals( false );
}
