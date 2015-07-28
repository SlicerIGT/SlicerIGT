/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Widgets includes
#include "qSlicerTransformPreviewWidget.h"
#include "qSlicerApplication.h"

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

  vtkWeakPointer<vtkMRMLTransformNode> CurrentTransformNode;
  std::vector< vtkSmartPointer< vtkMRMLTransformableNode > > PreviewNodes;
};

// --------------------------------------------------------------------------
qSlicerTransformPreviewWidgetPrivate::qSlicerTransformPreviewWidgetPrivate( qSlicerTransformPreviewWidget& object) : q_ptr(&object)
, CurrentTransformNode(NULL)
{
}

qSlicerTransformPreviewWidgetPrivate::~qSlicerTransformPreviewWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void qSlicerTransformPreviewWidgetPrivate::setupUi(qSlicerTransformPreviewWidget* widget)
{
  this->Ui_qSlicerTransformPreviewWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTransformPreviewWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformPreviewWidget::qSlicerTransformPreviewWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerTransformPreviewWidgetPrivate(*this) )
{
  this->setup();
}

//-----------------------------------------------------------------------------
qSlicerTransformPreviewWidget::~qSlicerTransformPreviewWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::setup()
{
  Q_D(qSlicerTransformPreviewWidget);

  d->setupUi(this);

  connect( d->TransformPreviewComboBox, SIGNAL(checkedNodesChanged()), this, SLOT(onCheckedNodesChanged()) );

  connect( d->ApplyButton, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()) );
  connect( d->HardenButton, SIGNAL(clicked()), this, SLOT(onHardenButtonClicked()) );

  d->CurrentTransformNode = NULL;

  this->updateWidget();  
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::enter()
{
  this->updateWidget();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerTransformPreviewWidget::currentNode()
{
  Q_D(qSlicerTransformPreviewWidget);
  return d->CurrentTransformNode;
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::setCurrentNode( vtkMRMLNode* newNode )
{
  Q_D(qSlicerTransformPreviewWidget);
  vtkMRMLTransformNode* newTransformNode = vtkMRMLTransformNode::SafeDownCast( newNode );

  // Reset the combo box if the node has changed
  if ( newTransformNode == NULL || d->CurrentTransformNode == NULL || strcmp( d->CurrentTransformNode->GetID(), newTransformNode->GetID() ) != 0 )
  {
    // Remove all preview nodes from the scene and the GUI
    this->clearPreviewNodes();
    bool wasTransformPreviewComboBoxBlocked = d->TransformPreviewComboBox->blockSignals(true);
    for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
    {
      vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }
    d->TransformPreviewComboBox->blockSignals(wasTransformPreviewComboBoxBlocked);
  }

  d->CurrentTransformNode = newTransformNode;

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::onCheckedNodesChanged()
{
  Q_D(qSlicerTransformPreviewWidget);

  // First thing to do is delete all of the preview nodes and remove from scene
  this->clearPreviewNodes();

  bool wasTransformPreviewComboBoxBlocked = d->TransformPreviewComboBox->blockSignals(true);

  const char* currentTransformNodeId = (d->CurrentTransformNode ? d->CurrentTransformNode->GetID() : NULL);

  // Now, look at all of the checked nodes - add a preview node if its checked
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    bool isTransformNode = (currentTransformNodeId && strcmp( baseNode->GetID(), currentTransformNodeId ) == 0);
    bool isChildNode = (currentTransformNodeId && baseNode->GetTransformNodeID() != NULL && strcmp( baseNode->GetTransformNodeID(), currentTransformNodeId ) == 0);
    if ( isTransformNode || isChildNode )
    {
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }

    if ( d->TransformPreviewComboBox->checkState( baseNode ) == Qt::Checked )
    {
      this->createAndAddPreviewNode( baseNode );
    }

  }

  d->TransformPreviewComboBox->blockSignals(wasTransformPreviewComboBoxBlocked);

  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::onApplyButtonClicked()
{
  Q_D(qSlicerTransformPreviewWidget);

  bool wasTransformPreviewComboBoxBlocked = d->TransformPreviewComboBox->blockSignals(true);

  // Now, look at all of the checked nodes - put under the transform node if checked
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    if ( d->TransformPreviewComboBox->checkState( baseNode ) == Qt::Checked )
    {
      baseNode->SetAndObserveTransformNodeID( d->CurrentTransformNode->GetID() );
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }
  }

  d->TransformPreviewComboBox->blockSignals(wasTransformPreviewComboBoxBlocked);

  this->onCheckedNodesChanged(); // Force update the combo box
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::onHardenButtonClicked()
{
  Q_D(qSlicerTransformPreviewWidget);

  bool wasTransformPreviewComboBoxBlocked = d->TransformPreviewComboBox->blockSignals(true);

  // Now, look at all of the checked nodes - harden under the transform node if checked
  for ( int i = 0; i < d->TransformPreviewComboBox->nodeCount(); i++ )
  {
    vtkMRMLTransformableNode* baseNode = vtkMRMLTransformableNode::SafeDownCast( d->TransformPreviewComboBox->nodeFromIndex( i ) );

    if ( d->TransformPreviewComboBox->checkState( baseNode ) == Qt::Checked )
    {
#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
      vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
      d->CurrentTransformNode->GetMatrixTransformToParent(matrix);
      baseNode->ApplyTransformMatrix(matrix);
#else
      baseNode->ApplyTransformMatrix( d->CurrentTransformNode->GetMatrixTransformToParent() );
#endif
      d->TransformPreviewComboBox->setCheckState( baseNode, Qt::Unchecked );
    }
  }

  d->TransformPreviewComboBox->blockSignals(wasTransformPreviewComboBoxBlocked);

  this->onCheckedNodesChanged(); // Force update the combo box
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::updateWidget()
{
  Q_D(qSlicerTransformPreviewWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    // the scene might be in an inconsistent mode during scene loading/closing
    // don't try to update the widget
    return;
    }

  if ( this->mrmlScene() == NULL || d->CurrentTransformNode == NULL )
  {
    d->TransformPreviewComboBox->setEnabled( false );
    d->ApplyButton->setEnabled( false );
    d->HardenButton->setEnabled( false );
    d->TransformLabel->setText( QString::fromStdString( "" ) );
    return;
  }

  // Update hidden nodes

  QStringList hiddenNodeIDList;
  QStringList visibleNodeIDList;

  // Iterate over all nodes in the scene and observe them
  vtkCollection* sceneNodes = this->mrmlScene()->GetNodes();

  for ( int i = 0; i < sceneNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLTransformableNode* currentNode = vtkMRMLTransformableNode::SafeDownCast( sceneNodes->GetItemAsObject( i ) );

    if ( currentNode == NULL || currentNode->GetHideFromEditors() || d->CurrentTransformNode == NULL )
    {
      continue;
    }
    
    bool isTransformNode = strcmp( currentNode->GetID(), d->CurrentTransformNode->GetID() ) == 0;
    bool isChildNode = currentNode->GetTransformNodeID() != NULL && strcmp( currentNode->GetTransformNodeID(), d->CurrentTransformNode->GetID() ) == 0;

    if ( isTransformNode || isChildNode )
    {
      hiddenNodeIDList << QString( currentNode->GetID() );
    }
    else
    {
      visibleNodeIDList << QString( currentNode->GetID() );
    }
  }

  bool wasTransformPreviewComboBoxBlocked = d->TransformPreviewComboBox->blockSignals(true);
  d->TransformPreviewComboBox->sortFilterProxyModel()->setHiddenNodeIDs( hiddenNodeIDList );
  d->TransformPreviewComboBox->sortFilterProxyModel()->setVisibleNodeIDs( visibleNodeIDList );
  d->TransformPreviewComboBox->blockSignals(wasTransformPreviewComboBoxBlocked);

  // Update widget
  d->TransformPreviewComboBox->setEnabled( true );
  d->ApplyButton->setEnabled( true );
  d->HardenButton->setEnabled( true );
  d->TransformLabel->setText( QString::fromStdString( d->CurrentTransformNode->GetName() ) );
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::createAndAddPreviewNode( vtkMRMLNode* baseNode )
{
  Q_D(qSlicerTransformPreviewWidget);

  // Create a preview node, apply the transform and add to the vector of preview nodes
  vtkSmartPointer< vtkMRMLTransformableNode > previewNode;
  previewNode.TakeReference( vtkMRMLTransformableNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( baseNode->GetClassName() ) ) );
  previewNode->Copy( baseNode );

  QString copyName;
  copyName.append( baseNode->GetName() ); copyName.append( "_Copy" );
  previewNode->SetName( copyName.toStdString().c_str() );

  previewNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( previewNode );

  previewNode->SetAndObserveTransformNodeID( d->CurrentTransformNode->GetID() );

  // In case the preview node is a displayable node, then copy its display node
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast( previewNode );
  if ( displayableNode != NULL && displayableNode->GetDisplayNode() != NULL)
  {
    vtkSmartPointer< vtkMRMLDisplayNode > displayNode;
    displayNode.TakeReference( vtkMRMLDisplayNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( displayableNode->GetDisplayNode()->GetClassName() ) ) );
    displayNode->Copy( displayableNode->GetDisplayNode() );

    displayNode->SetScene( this->mrmlScene() );
    this->mrmlScene()->AddNode( displayNode );

    displayableNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  }

  d->PreviewNodes.push_back( previewNode );
}

//-----------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::clearPreviewNodes()
{
  Q_D(qSlicerTransformPreviewWidget);

  for ( int i = 0; i < d->PreviewNodes.size(); i++ )
  {
    this->mrmlScene()->RemoveNode( d->PreviewNodes.at(i) );
  }
  d->PreviewNodes.clear(); // Smart pointers will take care of deleting objects
}

//------------------------------------------------------------------------------
void qSlicerTransformPreviewWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerTransformPreviewWidget);
  this->qvtkReconnect(this->mrmlScene(), scene, vtkCommand::ModifiedEvent, this, SLOT(updateWidget()));
  this->qvtkReconnect(this->mrmlScene(), scene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(updateWidget()));
  Superclass::setMRMLScene(scene);
  this->updateWidget();
}
