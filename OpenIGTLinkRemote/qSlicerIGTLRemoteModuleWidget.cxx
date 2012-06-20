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

// SlicerQt includes
#include "qSlicerIGTLRemoteModuleWidget.h"
#include "ui_qSlicerIGTLRemoteModule.h"

#include "qSlicerApplication.h"

#include "vtkMRMLIGTLConnectorNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IGTLRemote
class qSlicerIGTLRemoteModuleWidgetPrivate: public Ui_qSlicerIGTLRemoteModule
{
public:
  qSlicerIGTLRemoteModuleWidgetPrivate();

public:
  QButtonGroup typeButtonGroup;
  
  enum {
    TYPE_IMAGE,
    TYPE_LABEL,
    TYPE_ALL,
  };

  vtkMRMLIGTLConnectorNode * connectorNode;
  
};

//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidgetPrivate::qSlicerIGTLRemoteModuleWidgetPrivate()
{
}


//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidget::qSlicerIGTLRemoteModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIGTLRemoteModuleWidgetPrivate )
{
  
}

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidget::~qSlicerIGTLRemoteModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setup()
{
  Q_D(qSlicerIGTLRemoteModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->typeButtonGroup.addButton(d->typeImageRadioButton, qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_IMAGE);
  d->typeButtonGroup.addButton(d->typeLabelRadioButton, qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_LABEL);
  d->typeButtonGroup.addButton(d->typeAllRadioButton, qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_ALL);

  QObject::connect(d->connectorNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(setConnectorNode(vtkMRMLNode*)));
  
}

//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

  vtkMRMLScene* oldScene = this->mrmlScene();

  this->Superclass::setMRMLScene(newScene);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  
  if (oldScene != newScene)
    {
    if (d->connectorNodeSelector)
      {
      d->connectorNodeSelector->setMRMLScene(newScene);
      }
    }


  // Search the scene for the available view nodes and create a
  // Controller and connect it up
  newScene->InitTraversal();
  //for (vtkMRMLNode *sn = NULL; (sn=newScene->GetNextNodeByClass("vtkMRMLSliceNode"));)
  //  {
  //  vtkMRMLSliceNode *snode = vtkMRMLSliceNode::SafeDownCast(sn);
  //  if (snode)
  //    {
  //    d->createController(snode, layoutManager);
  //    }
  //  }
  //
  //// Need to listen for any new slice or view nodes being added
  //this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeAddedEvent, 
  //                    this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));
  //
  //// Need to listen for any slice or view nodes being removed
  //this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeRemovedEvent, 
  //                    this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));
  //

}


//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setConnectorNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

  vtkMRMLIGTLConnectorNode* cnode =
    vtkMRMLIGTLConnectorNode::SafeDownCast(node);
  if (cnode)
    {
    d->connectorNode = cnode;
    }
}


