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

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeChanged() ) );

  // Make connections to update the mrml from the widget
  connect( d->ModelNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModelNodeChanged() ) );
  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolTransformChanged() ) );
  connect( d->ColorPickerButton, SIGNAL( colorChanged( QColor ) ), this, SLOT( UpdateWarningColor( QColor ) ) );
  
  this->UpdateFromMRMLNode();
}



void
qSlicerBreachWarningModuleWidget
::enter()
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

  this->Superclass::enter();
}



void
qSlicerBreachWarningModuleWidget
::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerBreachWarningModuleWidget );

  this->Superclass::setMRMLScene( scene );
}



void
qSlicerBreachWarningModuleWidget
::onSceneImportedEvent()
{
  this->enter();
}



void
qSlicerBreachWarningModuleWidget
::onModuleNodeChanged()
{
  Q_D( qSlicerBreachWarningModuleWidget );

  this->UpdateFromMRMLNode();
}



void qSlicerBreachWarningModuleWidget
::onModelNodeChanged()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( bwNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }
  
  vtkMRMLNode* currentNode = d->ModelNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->logic()->SetWatchedModelNode( NULL, bwNode );
  }
  else
  {
    d->logic()->SetWatchedModelNode( vtkMRMLModelNode::SafeDownCast( currentNode ), bwNode );
  }
}



void
qSlicerBreachWarningModuleWidget
::onToolTransformChanged()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( moduleNode == NULL )
  {
    qCritical( "Transform node should not be changed when no module node selected" );
    return;
  }
  
  vtkMRMLNode* currentNode = d->ToolComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->logic()->SetObservedTransformNode( NULL, moduleNode );
  }
  else
  {
    d->logic()->SetObservedTransformNode( vtkMRMLLinearTransformNode::SafeDownCast( currentNode ), moduleNode );
  }
}



void
qSlicerBreachWarningModuleWidget
::UpdateWarningColor( QColor newColor )
{
  Q_D(qSlicerBreachWarningModuleWidget);

  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( moduleNode == NULL )
  {
    qCritical( "Color selected without module node" );
    return;
  }

  d->logic()->SetWarningColor( newColor.redF(), newColor.greenF(), newColor.blueF(), newColor.alphaF(), moduleNode );
}



void
qSlicerBreachWarningModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerBreachWarningModuleWidget );
  
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ModelNodeComboBox->setCurrentNodeID( "" );
    d->ModelNodeComboBox->setEnabled( false );
    d->ToolComboBox->setEnabled( false );
    return;
  }
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( currentNode );
  if ( bwNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }
  
  d->ModelNodeComboBox->setEnabled( true );
  d->ToolComboBox->setEnabled( true );
  
  if ( bwNode->GetToolTransformNode() != NULL )
  {
    d->ToolComboBox->setCurrentNodeID( QString::fromStdString( bwNode->GetToolTransformNode()->GetID() ) );
  }
  else
  {
    d->ToolComboBox->setCurrentNodeID( "" );
  }

  if ( bwNode->GetWatchedModelNode() != NULL )
  {
    d->ModelNodeComboBox->setCurrentNodeID( QString::fromStdString( bwNode->GetWatchedModelNode()->GetID() ) );
  }
  else
  {
    d->ModelNodeComboBox->setCurrentNodeID( "" );
  }
  
  d->ColorPickerButton->setColor( QColor( d->logic()->GetWarningColorComponent( 0, bwNode ), d->logic()->GetWarningColorComponent( 1, bwNode ), d->logic()->GetWarningColorComponent( 2, bwNode ) ) );
  
}