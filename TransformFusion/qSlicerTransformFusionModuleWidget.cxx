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
#include <QRegExp>
#include <QValidator>
#include <QRegExpValidator>

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
  this->update();
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setTransformFusionParametersNode(vtkMRMLNode *node)
{
  Q_D(qSlicerTransformFusionModuleWidget);
  
  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast(node);
  qvtkReconnect( d->logic()->GetTransformFusionNode(), pNode, vtkCommand::ModifiedEvent, this, SLOT(update()));
  d->logic()->SetAndObserveTransformFusionNode(pNode);
  this->update();
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
  d->outputTransformComboBox->setCurrentNode(pNode->GetOutputTransformNode());

  // Parameters
  // update transform list
  d->updateRateBox->setValue(pNode->GetUpdatesPerSecond());
}


//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onSingleUpdate()
{

}


//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::onOutputTransformNodeSelected()
{

}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModuleWidget::setup()
{
  Q_D(qSlicerTransformFusionModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect(d->outputTransformComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onOutputTransformNodeSelected(vtkMRMLNode*)));
  
  //connect(d->updateButton, SIGNAL(clicked()), this, SLOT(onSingleUpdate()));
}

