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
#include "qSlicerMarkupsToModelModuleWidget.h"
#include "ui_qSlicerMarkupsToModelModuleWidget.h"

#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkSlicerMarkupsToModelLogic.h"
#include "vtkMRMLMarkupsFiducialNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMarkupsToModelModuleWidgetPrivate: public Ui_qSlicerMarkupsToModelModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerMarkupsToModelModuleWidget ); 

protected:
  qSlicerMarkupsToModelModuleWidget* const q_ptr;
public:
  qSlicerMarkupsToModelModuleWidgetPrivate( qSlicerMarkupsToModelModuleWidget& object);
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
qSlicerMarkupsToModelModuleWidget::qSlicerMarkupsToModelModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerMarkupsToModelModuleWidgetPrivate ( *this ))
{
}

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModuleWidget::~qSlicerMarkupsToModelModuleWidget()
{
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onCurrentMarkupsNodeChanged()
{
  Q_D(qSlicerMarkupsToModelModuleWidget);

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( markupsToModelModuleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }

  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(d->MarkupsNodeComboBox->currentNode());
  d->logic()->SetMarkupsNode( markupsNode, markupsToModelModuleNode );
  d->logic()->UpdateOutputModel(markupsToModelModuleNode);
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::setup()
{
  Q_D(qSlicerMarkupsToModelModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect( d->ModuleNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelModuleNodeChanged() ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode* )), this, SLOT(onMarkupsToModelModuleNodeAddedByUser(vtkMRMLNode* ) ) );
  connect( d->ModuleNodeComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode* )), this, SLOT(onMarkupsToModelModuleNodeAboutToBeRemoved(vtkMRMLNode* ) ) );

  connect( d->MarkupsNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onCurrentMarkupsNodeChanged() ) );

  connect( d->UpdateOutputModelPushButton, SIGNAL( clicked() ) , this, SLOT( onUpdateOutputModelPushButton() ) );

}





//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::enter()
{
  Q_D(qSlicerMarkupsToModelModuleWidget);

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // Create a module MRML node if there is none in the scene.

  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLMarkupsToModelNode");
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

  //if ( d->ParameterNodeComboBox->currentNode() == NULL )
  //{
  //  d->ParameterNodeComboBox->setCurrentNodeID( node->GetID() );
  //}

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::updateWidget()
{
  Q_D(qSlicerMarkupsToModelModuleWidget);
  vtkMRMLNode* currentModuleNode = d->ModuleNodeComboBox->currentNode();
  if ( currentModuleNode == NULL )
  {
    d->MarkupsNodeComboBox->setCurrentNodeID( "" );
    d->MarkupsNodeComboBox->setEnabled( false );
    return;
  }
  vtkMRMLMarkupsToModelNode* MarkupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( currentModuleNode );
  if ( MarkupsToModelNode == NULL )
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
    // This will ensure that we refresh the widget next time we move to a non-null widget (since there is guaranteed to be a modified status of larger than zero)
    //return;
    d->DeleteAllPushButton->setEnabled(false);
    d->DeleteLastPushButton->setEnabled(false);
    d->UpdateOutputModelPushButton->setEnabled(false);
  }
  else
  {
    // Set the button indicating if this list is active
    if ( MarkupsToModelNode->GetMarkupsNode()->GetNumberOfFiducials() > 0 )
    {
      d->DeleteAllPushButton->setEnabled(true);
      d->DeleteLastPushButton->setEnabled(true);
      if(MarkupsToModelNode->GetMarkupsNode()->GetNumberOfFiducials() > 10)
      {
        d->UpdateOutputModelPushButton->setEnabled(true);
      }
    }
    else
    {
      d->DeleteAllPushButton->setEnabled(false);
      d->DeleteLastPushButton->setEnabled(false);
      d->UpdateOutputModelPushButton->setEnabled(false);
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
  updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onMarkupsToModelModuleNodeChanged()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onMarkupsToModelModuleNodeAddedByUser(vtkMRMLNode* nodeAdded)
{
  Q_D( qSlicerMarkupsToModelModuleWidget );
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default module.
  if(nodeAdded==NULL)
  {
    return;
  }
  vtkMRMLMarkupsToModelNode* MarkupsToModelNodeAdded = vtkMRMLMarkupsToModelNode::SafeDownCast( nodeAdded );
  if(MarkupsToModelNodeAdded==NULL)
  {
    return;
  }

  //if(d->ToolBarManager->GetToolBarHash()==NULL)
  //{
  //  return;
  //}

  //qMRMLMarkupsToModelToolBar *MarkupsToModelToolBar=d->ToolBarManager->GetToolBarHash()->value(QString(MarkupsToModelNodeAdded->GetID()));
  //connect(MarkupsToModelToolBar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolBarVisibilityChanged(bool)) );

  this->updateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModuleWidget::onUpdateOutputModelPushButton()
{
  Q_D( qSlicerMarkupsToModelModuleWidget );

  vtkMRMLMarkupsToModelNode* moduleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( moduleNode == NULL )
  {
    qCritical( "Model node changed with no module node selection" );
    return;
  }

  //vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(d->ModelNodeComboBox->currentNode());
  //if ( markupsNode == NULL )
  //{
  //  qCritical( "Model node changed with no module node selection" );
  //  return;
  //}
  d->logic()->UpdateOutputModel(moduleNode);
}

