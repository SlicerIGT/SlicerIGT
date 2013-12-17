/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerTransformPreviewWidget.h"

#include <QtGui>



//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerTransformPreviewWidgetPrivate
  : public Ui_qSlicerTransformPreviewWidget
{
  Q_DECLARE_PUBLIC(qSlicerTransformPreviewWidget);
protected:
  qSlicerTransformPreviewWidget* const q_ptr;

public:
  qSlicerTransformPreviewWidgetPrivate( qSlicerTransformPreviewWidget& object);
  ~qSlicerTransformPreviewWidgetPrivate();
  virtual void setupUi(qSlicerTransformPreviewWidget*);
};

// --------------------------------------------------------------------------
qSlicerTransformPreviewWidgetPrivate
::qSlicerTransformPreviewWidgetPrivate( qSlicerTransformPreviewWidget& object) : q_ptr(&object)
{
}

qSlicerTransformPreviewWidgetPrivate
::~qSlicerTransformPreviewWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerTransformPreviewWidgetPrivate
::setupUi(qSlicerTransformPreviewWidget* widget)
{
  this->Ui_qSlicerTransformPreviewWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTransformPreviewWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformPreviewWidget
::qSlicerTransformPreviewWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerTransformPreviewWidgetPrivate(*this) )
{
}


qSlicerTransformPreviewWidget
::~qSlicerTransformPreviewWidget()
{
}


qSlicerTransformPreviewWidget* qSlicerTransformPreviewWidget
::New( vtkMRMLScene* scene )
{
  qSlicerTransformPreviewWidget* newTransformPreviewWidget = new qSlicerTransformPreviewWidget();
  newTransformPreviewWidget->setup();
  newTransformPreviewWidget->setMRMLScene( scene );
  return newTransformPreviewWidget;
}


void qSlicerTransformPreviewWidget
::setup()
{
  Q_D(qSlicerTransformPreviewWidget);

  d->setupUi(this);

  connect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  connect( d->ApplyButton, SIGNAL( clicked() ), this, SLOT( onApplyButtonClicked() ) );
  connect( d->HardenButton, SIGNAL( clicked() ), this, SLOT( onHardenButtonClicked() ) );

  this->CurrentTransformNode = NULL;

  this->updateWidget();  
}


void qSlicerTransformPreviewWidget
::enter()
{
}


vtkMRMLNode* qSlicerTransformPreviewWidget
::GetCurrentNode()
{
  Q_D(qSlicerTransformPreviewWidget);

  return this->CurrentTransformNode;
}


void qSlicerTransformPreviewWidget
::SetCurrentNode( vtkMRMLNode* currentNode )
{
  Q_D(qSlicerTransformPreviewWidget);

  // Reset the combo box if the node has 
  if ( currentNode == NULL || this->CurrentTransformNode == NULL || strcmp( this->CurrentTransformNode->GetID(), currentNode->GetID() ) != 0 )
  {
    disconnect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

    for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
    {
      vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }

    connect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );
  }

  this->CurrentTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( currentNode );

  this->updateWidget();
}


void qSlicerTransformPreviewWidget
::onCheckedNodesChanged()
{
  Q_D(qSlicerTransformPreviewWidget);

  // First thing to do is delete all of the preview nodes and remove from scene
  for ( int i = 0; i < this->PreviewNodes.size(); i++ )
  {
    this->mrmlScene()->RemoveNode( this->PreviewNodes.at(i) );
  }
  this->PreviewNodes.clear(); // Smart pointers will take care of deleting objects

  disconnect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  // Now, look at all of the checked nodes - add a preview node if its checked
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    bool isTransformNode = strcmp( baseNode->GetID(), this->CurrentTransformNode->GetID() ) == 0;
    bool isChildNode = baseNode->GetTransformNodeID() != NULL && strcmp( baseNode->GetTransformNodeID(), this->CurrentTransformNode->GetID() ) == 0;

    if ( isTransformNode || isChildNode )
    {
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }

    if ( d->TransformPreviewComboBox->checkState( baseNode ) == Qt::Checked )
    {
      // Create a preview node, apply the transform and add to the vector of preview nodes
      vtkSmartPointer< vtkMRMLTransformableNode > previewNode;
      previewNode.TakeReference( vtkMRMLTransformableNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( baseNode->GetClassName() ) ) );
      previewNode->Copy( baseNode );

      QString copyName;
      copyName.append( baseNode->GetName() ); copyName.append( "_Copy" );
      previewNode->SetName( copyName.toStdString().c_str() );

      previewNode->SetScene( this->mrmlScene() );
      this->mrmlScene()->AddNode( previewNode );

      previewNode->SetAndObserveTransformNodeID( this->CurrentTransformNode->GetID() );

      this->PreviewNodes.push_back( previewNode );
    }

  }

  connect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  this->updateWidget();
}


void qSlicerTransformPreviewWidget
::onApplyButtonClicked()
{
  Q_D(qSlicerTransformPreviewWidget);

  disconnect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  // Now, look at all of the checked nodes - put under the transform node if checked
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    if ( d->TransformPreviewComboBox->checkState( baseNode ) == Qt::Checked )
    {
      baseNode->SetAndObserveTransformNodeID( this->CurrentTransformNode->GetID() );
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }
  }

  connect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  this->onCheckedNodesChanged(); // Force update the combo box
}



void qSlicerTransformPreviewWidget
::onHardenButtonClicked()
{
  Q_D(qSlicerTransformPreviewWidget);

  disconnect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  // Now, look at all of the checked nodes - harden under the transform node if checked
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    if ( d->TransformPreviewComboBox->checkState( baseNode ) == Qt::Checked )
    {
      baseNode->ApplyTransformMatrix( this->CurrentTransformNode->GetMatrixTransformToParent() );
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }
  }

  connect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

  this->onCheckedNodesChanged(); // Force update the combo box
}


void qSlicerTransformPreviewWidget
::HideChildNodes()
{
  Q_D(qSlicerTransformPreviewWidget);

  // First, remove nodes that shouldn't be hidden
  for ( int i = 0; i < this->HideNodeIDs.size(); i++ )
  {
    vtkMRMLTransformableNode* currentNode = vtkMRMLTransformableNode::SafeDownCast( this->mrmlScene()->GetNodeByID( this->HideNodeIDs.at( i ) ) );

    if ( currentNode == NULL ) // this could occur if the node is removed from the scene
    {
      this->HideNodeIDs.erase( this->HideNodeIDs.begin() + i );
      continue;
    }

    bool isTransformNode = strcmp( currentNode->GetID(), this->CurrentTransformNode->GetID() ) == 0;
    bool isChildNode = currentNode->GetTransformNodeID() != NULL && strcmp( currentNode->GetTransformNodeID(), this->CurrentTransformNode->GetID() ) == 0;

    if ( !( isTransformNode || isChildNode ) )
    {
      this->HideNodeIDs.erase( this->HideNodeIDs.begin() + i );
    }
  }

  // Next, add nodes that should be hidden
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    bool isTransformNode = strcmp( baseNode->GetID(), this->CurrentTransformNode->GetID() ) == 0;
    bool isChildNode = baseNode->GetTransformNodeID() != NULL && strcmp( baseNode->GetTransformNodeID(), this->CurrentTransformNode->GetID() ) == 0;

    if ( isTransformNode || isChildNode )
    {
      this->HideNodeIDs.push_back( d->TransformPreviewComboBox->nodeFromIndex( i )->GetID() );
    }    
  }

  // Finally, add the nodes to the filter and set it
  QStringList& hideNodeIDList = QStringList();
  for ( int i = 0; i < this->HideNodeIDs.size(); i++ )
  {
    hideNodeIDList.append( this->HideNodeIDs.at( i ).c_str() );
  }

  disconnect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );
  d->TransformPreviewComboBox->sortFilterProxyModel()->setHiddenNodeIDs( hideNodeIDList );
  connect( d->TransformPreviewComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedNodesChanged() ) );

}


void qSlicerTransformPreviewWidget
::updateWidget()
{
  Q_D(qSlicerTransformPreviewWidget);

  if ( this->CurrentTransformNode == NULL )
  {
    d->TransformPreviewComboBox->setEnabled( false );
    d->ApplyButton->setEnabled( false );
    d->HardenButton->setEnabled( false );
    d->TransformLabel->setText( QString::fromStdString( "" ) );
    return;
  }

  d->TransformPreviewComboBox->setEnabled( true );
  d->ApplyButton->setEnabled( true );
  d->HardenButton->setEnabled( true );
  d->TransformLabel->setText( QString::fromStdString( this->CurrentTransformNode->GetName() ) );

  this->HideChildNodes();
}
