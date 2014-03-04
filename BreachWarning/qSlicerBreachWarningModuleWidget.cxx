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
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"


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


qSlicerBreachWarningModuleWidgetPrivate::qSlicerBreachWarningModuleWidgetPrivate( qSlicerBreachWarningModuleWidget& object ) 
  : q_ptr( &object )
  , ModuleWindowInitialized( false )
{
}


vtkSlicerBreachWarningLogic* qSlicerBreachWarningModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerBreachWarningModuleWidget );
  return vtkSlicerBreachWarningLogic::SafeDownCast( q->logic() );
}



//-----------------------------------------------------------------------------
// qSlicerBreachWarningModuleWidget methods



qSlicerBreachWarningModuleWidget
::qSlicerBreachWarningModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerBreachWarningModuleWidgetPrivate( *this ) )
{
}



qSlicerBreachWarningModuleWidget
::~qSlicerBreachWarningModuleWidget()
{
}



void
qSlicerBreachWarningModuleWidget
::setup()
{
  Q_D(qSlicerBreachWarningModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateFromMRMLNode() ) );

  // Make connections to update the mrml from the widget
  connect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateWathedModel() ) );
  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToolTipTransform() ) );
  connect( d->ColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateWarningColor( QColor ) ) );
  
  // Watch the logic - it is updated whenver the mrml node is updated
  this->qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( UpdateFromMRMLNode() ) );

  this->UpdateFromMRMLNode();
}



void
qSlicerBreachWarningModuleWidget
::enter()
{
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  Q_D(qSlicerBreachWarningModuleWidget);

  this->qSlicerAbstractModuleWidget::enter();

  if ( d->logic() == NULL )
  {
    qCritical() << "Invalid logic!";
    return;
  }
  vtkMRMLBreachWarningNode* moduleNode = d->logic()->GetBreachWarningNode();

  if ( moduleNode == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLBreachWarningNode" );
    if ( node )
    {
      moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( node );
      d->logic()->SetAndObserveBreachWarningNode( moduleNode );
      return;
    }
    else
    {
      vtkSmartPointer< vtkMRMLBreachWarningNode > newNode = vtkSmartPointer< vtkMRMLBreachWarningNode >::New();
      this->mrmlScene()->AddNode( newNode );
      d->logic()->SetAndObserveBreachWarningNode( newNode );
    }
  }
  
  this->UpdateFromMRMLNode();
  d->ModuleWindowInitialized = true;
}



void
qSlicerBreachWarningModuleWidget
::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerBreachWarningModuleWidget );

  this->Superclass::setMRMLScene( scene );

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportedEvent() ) );

  if ( scene && d->logic()->GetBreachWarningNode() == 0 )
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass( 0, "vtkMRMLBreachWarningNode" );
    if ( node )
    {
      this->setBreachWarningNode( vtkMRMLBreachWarningNode::SafeDownCast( node ) );
    }
  }
}



void
qSlicerBreachWarningModuleWidget
::onSceneImportedEvent()
{
  this->enter();
}



void
qSlicerBreachWarningModuleWidget
::setBreachWarningNode( vtkMRMLNode* node )
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( node );

  qvtkReconnect( d->logic()->GetBreachWarningNode(), moduleNode, vtkCommand::ModifiedEvent, this, SLOT( UpdateFromMRMLNode() ) );

  d->logic()->SetAndObserveBreachWarningNode( moduleNode );
  this->UpdateFromMRMLNode();
}



void qSlicerBreachWarningModuleWidget
::UpdateWatchedModel()
{
  Q_D( qSlicerBreachWarningModuleWidget );

  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( BreachWarningNode == NULL )
  {
    return;
  }

  this->qvtkBlockAll( true );

  if ( d->ModelNodeComboBox->currentNode() == NULL )
  {
    BreachWarningNode->SetWatchedModelID( "", vtkMRMLBreachWarningNode::NeverModify );
  }
  else
  {
    BreachWarningNode->SetWatchedModelID( d->ModelNodeComboBox->currentNode()->GetID(), vtkMRMLBreachWarningNode::NeverModify );
  }
  
  this->qvtkBlockAll( false );

  // The modified event will be blocked... Now allow it to happen
  d->ModuleNodeComboBox->currentNode()->Modified();
  this->UpdateFromMRMLNode();
}



void
qSlicerBreachWarningModuleWidget
::UpdateToolTipTransform()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( moduleNode == NULL )
  {
    return;
  }

  this->qvtkBlockAll( true );

  if ( d->ToolComboBox->currentNode() == NULL )
  {
    moduleNode->SetToolTipTransformID( "", vtkMRMLBreachWarningNode::NeverModify );
  }
  else
  {
    moduleNode->SetToolTipTransformID( d->ToolComboBox->currentNode()->GetID(), vtkMRMLBreachWarningNode::NeverModify );
  }

  this->qvtkBlockAll( false );

  d->ToolComboBox->currentNode()->Modified();
  this->UpdateFromMRMLNode();
}



void
qSlicerBreachWarningModuleWidget
::UpdateWarningColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);
  d->logic()->SetWarningColor( newColor.redF(), newColor.greenF(), newColor.blueF(), newColor.alphaF() );
}



void
qSlicerBreachWarningModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerBreachWarningModuleWidget );

  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( BreachWarningNode == NULL )
  {
    d->ModelNodeComboBox->setEnabled( false );
    d->ToolComboBox->setEnabled( false );
    return;
  }

  d->ModelNodeComboBox->setEnabled( true );
  d->ToolComboBox->setEnabled( true );

  // Disconnect to prevent signals form cuing slots
  disconnect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateWatchedModel() ) );
  disconnect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToolTipTransform() ) );
  
  d->ModelNodeComboBox->setCurrentNodeID( QString::fromStdString( BreachWarningNode->GetWatchedModelID() ) );
  d->ToolComboBox->setCurrentNodeID( QString::fromStdString( BreachWarningNode->GetToolTipTransformID() ) );
  // d->ColorPickerButton->
  
  // Unblock all singals from firing
  // TODO: Is there a more efficient way to do this by blokcing slots?
  connect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateWatchedModel() ) );
  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToolTipTransform() ) );
  
}