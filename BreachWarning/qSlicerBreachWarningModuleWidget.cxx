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

};



//-----------------------------------------------------------------------------
// qSlicerBreachWarningModuleWidgetPrivate methods


qSlicerBreachWarningModuleWidgetPrivate::qSlicerBreachWarningModuleWidgetPrivate( qSlicerBreachWarningModuleWidget& object ) : q_ptr( &object )
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



void qSlicerBreachWarningModuleWidget
::setup()
{
  Q_D(qSlicerBreachWarningModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateFromMRMLNode() ) );

  // Make connections to update the mrml from the widget
  // connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  // connect( d->OutputTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  // connect( d->RigidRadioButton, SIGNAL( toggled( bool ) ), this, SLOT( UpdateToMRMLNode() ) );
  
  // Watch the logic - it is updated whenver the mrml node is updated
  this->qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( UpdateFromMRMLNode() ) );

  this->UpdateFromMRMLNode();
}


void qSlicerBreachWarningModuleWidget
::enter()
{
  Q_D(qSlicerBreachWarningModuleWidget);

  this->qSlicerAbstractModuleWidget::enter();

  // Create a node by default if none already exists
  int numBreachWarningNodes = this->mrmlScene()->GetNumberOfNodesByClass( "vtkMRMLBreachWarningNode" );
  if ( numBreachWarningNodes == 0 )
  {
    vtkSmartPointer< vtkMRMLNode > BreachWarningNode;
    BreachWarningNode.TakeReference( this->mrmlScene()->CreateNodeByClass( "vtkMRMLBreachWarningNode" ) );
    BreachWarningNode->SetScene( this->mrmlScene() );
    this->mrmlScene()->AddNode( BreachWarningNode );
    d->ModuleNodeComboBox->setCurrentNode( BreachWarningNode );
  }
}


void qSlicerBreachWarningModuleWidget
::UpdateToMRMLNode()
{
  Q_D( qSlicerBreachWarningModuleWidget );

  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( BreachWarningNode == NULL )
  {
    return;
  }

  this->qvtkBlockAll( true );
/*
  if ( d->ProbeTransformComboBox->currentNode() == NULL )
  {
    BreachWarningNode->SetProbeTransformID( "", vtkMRMLBreachWarningNode::NeverModify );
  }
  else
  {
    BreachWarningNode->SetProbeTransformID( d->ProbeTransformComboBox->currentNode()->GetID(), vtkMRMLBreachWarningNode::NeverModify );
  }
*/
  
  this->qvtkBlockAll( false );

  // The modified event will be blocked... Now allow it to happen
  d->ModuleNodeComboBox->currentNode()->Modified();
  this->UpdateFromMRMLNode();
}


void qSlicerBreachWarningModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerBreachWarningModuleWidget );

  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( BreachWarningNode == NULL )
  {
    // d->ProbeTransformComboBox->setEnabled( false );

    return;
  }

  // d->ProbeTransformComboBox->setEnabled( true );

  // Disconnect to prevent signals form cuing slots
  // disconnect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  
  // d->ProbeTransformComboBox->setCurrentNodeID( QString::fromStdString( BreachWarningNode->GetProbeTransformID() ) );
  
  // Unblock all singals from firing
  // TODO: Is there a more efficient way to do this by blokcing slots?
  // connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( UpdateToMRMLNode() ) );
  
}