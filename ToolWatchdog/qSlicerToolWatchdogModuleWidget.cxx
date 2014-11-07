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
#include <QDebug>

// SlicerQt includes
#include "qSlicerToolWatchdogModuleWidget.h"
#include "ui_qSlicerToolWatchdogModuleWidget.h"

#include "vtkSlicerToolWatchdogLogic.h"

#include "vtkMRMLToolWatchdogNode.h"
#include "vtkMRMLLinearTransformNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerToolWatchdogModuleWidgetPrivate: public Ui_qSlicerToolWatchdogModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerToolWatchdogModuleWidget ); 

protected:
  qSlicerToolWatchdogModuleWidget* const q_ptr;
public:
  qSlicerToolWatchdogModuleWidgetPrivate( qSlicerToolWatchdogModuleWidget& object );
  vtkSlicerToolWatchdogLogic* logic() const;

  bool ModuleWindowInitialized;
};

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidgetPrivate::qSlicerToolWatchdogModuleWidgetPrivate( qSlicerToolWatchdogModuleWidget& object )
: q_ptr( &object )
, ModuleWindowInitialized( false )
{
}



vtkSlicerToolWatchdogLogic* qSlicerToolWatchdogModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerToolWatchdogModuleWidget );
  return vtkSlicerToolWatchdogLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidget::qSlicerToolWatchdogModuleWidget(QWidget* _parent)
: Superclass( _parent )
, d_ptr( new qSlicerToolWatchdogModuleWidgetPrivate ( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModuleWidget::~qSlicerToolWatchdogModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerToolWatchdogModuleWidget::setup()
{
  Q_D(qSlicerToolWatchdogModuleWidget);
  
  d->setupUi(this);
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeChanged() ) );

  connect( d->ToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onToolTransformChanged() ) );

  this->UpdateFromMRMLNode();
}



void
qSlicerToolWatchdogModuleWidget
::enter()
{
  Q_D(qSlicerToolWatchdogModuleWidget);

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // Create a module MRML node if there is none in the scene.

  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLToolWatchdogNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLToolWatchdogNode > newNode = vtkSmartPointer< vtkMRMLToolWatchdogNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLToolWatchdogNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node" );
    return;
  }

  // For convenience, select a default module.

  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    d->ModuleNodeComboBox->setCurrentNodeID( node->GetID() );
  }

  this->Superclass::enter();
}



void
qSlicerToolWatchdogModuleWidget
::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  this->Superclass::setMRMLScene( scene );
}



void
qSlicerToolWatchdogModuleWidget
::onSceneImportedEvent()
{
  this->enter();
}



void
qSlicerToolWatchdogModuleWidget
::onModuleNodeChanged()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  this->UpdateFromMRMLNode();
}




//-----------------------------------------------------------------------------
void qSlicerToolWatchdogModuleWidget::onToolTransformChanged()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  vtkMRMLToolWatchdogNode* moduleNode = vtkMRMLToolWatchdogNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
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
qSlicerToolWatchdogModuleWidget
::UpdateFromMRMLNode()
{
  Q_D( qSlicerToolWatchdogModuleWidget );

  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  if ( currentNode == NULL )
  {
    d->ToolComboBox->setCurrentNodeID( "" );
    d->ToolComboBox->setEnabled( false );
    return;
  }

  vtkMRMLToolWatchdogNode* bwNode = vtkMRMLToolWatchdogNode::SafeDownCast( currentNode );
  if ( bwNode == NULL )
  {
    qCritical( "Selected node not a valid module node" );
    return;
  }

  d->ToolComboBox->setEnabled( true );

  if ( bwNode->GetToolTransformNode() != NULL )
  {
    d->ToolComboBox->setCurrentNodeID( QString::fromStdString( bwNode->GetToolTransformNode()->GetID() ) );
  }
  else
  {
    d->ToolComboBox->setCurrentNodeID( "" );
  }

}
