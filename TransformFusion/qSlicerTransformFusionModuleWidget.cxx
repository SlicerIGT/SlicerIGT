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

  This file was originally developed by Franklin King, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/


// Qt includes
#include <QTimer>
#include <QListWidgetItem>

// SlicerQt includes
#include "qSlicerTransformFusionModuleWidget.h"
#include "ui_qSlicerTransformFusionModule.h"

// Transform Fusion includes
#include "vtkSlicerTransformFusionLogic.h"
#include "vtkMRMLTransformFusionNode.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformFusion
class qSlicerTransformFusionModuleWidgetPrivate: public Ui_qSlicerTransformFusionModule
{
  Q_DECLARE_PUBLIC( qSlicerTransformFusionModuleWidget ); 
  
protected:
  qSlicerTransformFusionModuleWidget* const q_ptr;
public:
  qSlicerTransformFusionModuleWidgetPrivate( qSlicerTransformFusionModuleWidget& object );
  vtkSlicerTransformFusionLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerTransformFusionModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerTransformFusionModuleWidgetPrivate::qSlicerTransformFusionModuleWidgetPrivate( qSlicerTransformFusionModuleWidget& object ) : q_ptr( &object )
{
}

vtkSlicerTransformFusionLogic* qSlicerTransformFusionModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTransformFusionModuleWidget );
  return vtkSlicerTransformFusionLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerTransformFusionModuleWidget methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
qSlicerTransformFusionModuleWidget::qSlicerTransformFusionModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerTransformFusionModuleWidgetPrivate( *this ) )
{
  currentlyUpdating = false;

  this->updateTimer = new QTimer();
  updateTimer->setSingleShot(false);
  updateTimer->setInterval(17);
}

//-----------------------------------------------------------------------------
qSlicerTransformFusionModuleWidget::~qSlicerTransformFusionModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerTransformFusionModuleWidget);
  this->Superclass::setMRMLScene(scene);
  if (scene &&  d->logic()->GetTransformFusionNode() == 0)
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass(0, "vtkMRMLTransformFusionNode");
    if (node){
      this->setTransformFusionParametersNode(vtkMRMLTransformFusionNode::SafeDownCast(node));
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onEnter()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  
  if (!this->mrmlScene() || d->logic() == NULL)
  {
    std::cerr << "Error: Unable to initialize module" << std::endl;
    return;
  }

  //Check for existing parameter node
  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (pNode == NULL)
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLTransformFusionNode");
    if (node)
    {
      pNode = vtkMRMLTransformFusionNode::SafeDownCast(node);
      d->logic()->SetAndObserveTransformFusionNode(pNode);
    }
    else
    {
      vtkSmartPointer<vtkMRMLTransformFusionNode> newNode = vtkSmartPointer<vtkMRMLTransformFusionNode>::New();
      this->mrmlScene()->AddNode(newNode);
      d->logic()->SetAndObserveTransformFusionNode(newNode);
    }
  }
  
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setTransformFusionParametersNode(vtkMRMLNode *node)
{
  Q_D(qSlicerTransformFusionModuleWidget);
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast(node);
  qvtkReconnect( d->logic()->GetTransformFusionNode(), pNode, vtkCommand::ModifiedEvent, this, SLOT(update()));
  d->logic()->SetAndObserveTransformFusionNode(pNode);
  this->updateWidget();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateWidget()
{
  Q_D(qSlicerTransformFusionModuleWidget);

  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (pNode == NULL || this->mrmlScene() == NULL)
  {
    std::cerr << "Error: Unable to update widget" << std::endl;
    return;
  }

  // Update widget from MRML
  d->ParameterComboBox->setCurrentNode(pNode);
  // Populate list
  d->outputTransformComboBox->setCurrentNode(pNode->GetOutputTransformNode());

  // Parameters
  d->updateRateBox->setValue(pNode->GetUpdatesPerSecond());

  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::updateButtons()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene())
  {
    std::cerr << "Could not find Transform Fusion node" << std::endl;
    return;
  }

  if (pNode->GetInputTransforms().size() >= 2 && pNode->GetOutputTransformNode() != NULL)
  {
    d->updateButton->setEnabled(true);
    if (!currentlyUpdating)
    {
      d->startUpdateButton->setEnabled(true);
    }
  }
  else
  {
    d->updateButton->setEnabled(false);
    d->startUpdateButton->setEnabled(false);
  }

}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onSingleUpdate()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene() || pNode->GetOutputTransformNode() == NULL || pNode->GetInputTransforms().size() < 2)
  {
    std::cerr << "Error: Failed to update transform" << std::endl;
    return;
  }

  /*
    SIMPLE_AVERAGE = 0
    LERP_AND_SLERP = 1
  */
  d->logic()->fuseInputTransforms(d->techniqueBox->currentIndex());
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setUpdatesPerSecond(double updatesPerSecond)
{
  Q_D(qSlicerTransformFusionModuleWidget);
  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene())
  {
    return;
  }

  pNode->SetUpdatesPerSecond(updatesPerSecond);
  updateTimer->setInterval((1/updatesPerSecond)*1000);
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onStartAutoUpdate()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene() || pNode->GetOutputTransformNode() == NULL || pNode->GetInputTransforms().size() < 2)
  {
    std::cerr << "Error: Failed to start auto-update" << std::endl;
    return;
  }

  updateTimer->start();
  currentlyUpdating = true;
  d->startUpdateButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onStopAutoUpdate()
{
  updateTimer->stop();
  currentlyUpdating = false;
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onOutputTransformNodeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerTransformFusionModuleWidget);
  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene() || !node)
  {
    std::cerr << "Error: Unable to set output transform node" << std::endl;
    return;
  }
  pNode->SetAndObserveOutputTransformNode(node);
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onAddTransform()
{
  Q_D(qSlicerTransformFusionModuleWidget);

  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene())
  {
    std::cerr << "Error: Unable to add transform" << std::endl;
    return;
  }

  vtkMRMLLinearTransformNode* inputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->addTransformComboBox->currentNode());
  if (inputTransform == NULL)
  {
    std::cerr << "Error: Input is not a linear transform node" << std::endl;
    return;    
  }

  pNode->AddInputTransform(inputTransform);
  new QListWidgetItem(tr(inputTransform->GetName()), d->inputTransformList);
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onRemoveTransform()
{
  Q_D(qSlicerTransformFusionModuleWidget);

  vtkMRMLTransformFusionNode* pNode = d->logic()->GetTransformFusionNode();
  if (!pNode || !this->mrmlScene())
  {
    std::cerr << "Error: Unable to remove transform" << std::endl;
    return;
  }

  pNode->RemoveInputTransform(d->inputTransformList->currentRow());
  delete d->inputTransformList->currentItem();
  this->updateButtons();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setup()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect(d->ParameterComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setTransformFusionParametersNode(vtkMRMLNode*)));

  connect(d->outputTransformComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onOutputTransformNodeSelected(vtkMRMLNode*)));
  
  connect(d->addTransformButton, SIGNAL(clicked()), this, SLOT(onAddTransform()));
  connect(d->removeTransformButton, SIGNAL(clicked()), this, SLOT(onRemoveTransform()));

  connect(d->updateButton, SIGNAL(clicked()), this, SLOT(onSingleUpdate()));

  connect(d->updateRateBox, SIGNAL(valueChanged(double)), this, SLOT(setUpdatesPerSecond(double)));
  connect(d->startUpdateButton, SIGNAL(clicked()), this, SLOT(onStartAutoUpdate()));
  connect(d->stopUpdateButton, SIGNAL(clicked()), this, SLOT(onStopAutoUpdate()));
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(onSingleUpdate()));
}

