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
#include "qSlicerVolumeResliceDriverModuleWidget.h"
#include "ui_qSlicerVolumeResliceDriverModule.h"
#include "qSlicerReslicePropertyWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

#include <QButtonGroup>

#include "vtkSmartPointer.h"
#include "vtkCollection.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLLayoutLogic.h"


#include "vtkSlicerVolumeResliceDriverLogic.h"


#include <map>

#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeResliceDriver
class qSlicerVolumeResliceDriverModuleWidgetPrivate: public Ui_qSlicerVolumeResliceDriverModule
{
  Q_DECLARE_PUBLIC( qSlicerVolumeResliceDriverModuleWidget ); 

protected:
  qSlicerVolumeResliceDriverModuleWidget* const q_ptr;
  
public:
  qSlicerVolumeResliceDriverModuleWidgetPrivate( qSlicerVolumeResliceDriverModuleWidget& object );
  ~qSlicerVolumeResliceDriverModuleWidgetPrivate();
  vtkSlicerVolumeResliceDriverLogic* logic() const;
  
  /// Create a Controller for a Node and pack in the widget
  void createController(vtkMRMLNode *n, qSlicerLayoutManager *lm);

  /// Remove the Controller for a Node from the widget
  void removeController(vtkMRMLNode *n);

  typedef std::map<vtkSmartPointer<vtkMRMLNode>, qSlicerReslicePropertyWidget* > WidgetMapType;
  WidgetMapType WidgetMap;
  int showAdvanced;
};


//-----------------------------------------------------------------------------
// qSlicerVolumeResliceDriverModuleWidgetPrivate methods


qSlicerVolumeResliceDriverModuleWidgetPrivate
::qSlicerVolumeResliceDriverModuleWidgetPrivate( qSlicerVolumeResliceDriverModuleWidget& object )
 : q_ptr(&object), showAdvanced( 0 )
{
}


qSlicerVolumeResliceDriverModuleWidgetPrivate
::~qSlicerVolumeResliceDriverModuleWidgetPrivate()
{
}


vtkSlicerVolumeResliceDriverLogic* qSlicerVolumeResliceDriverModuleWidgetPrivate
::logic() const
{
  Q_Q( const qSlicerVolumeResliceDriverModuleWidget );
  return vtkSlicerVolumeResliceDriverLogic::SafeDownCast( q->logic() );
}


void qSlicerVolumeResliceDriverModuleWidgetPrivate
::createController( vtkMRMLNode *n, qSlicerLayoutManager *layoutManager )
{
  if (this->WidgetMap.find(n) != this->WidgetMap.end())
    {
    qDebug() << "qSlicerVolumeResliceDriverModuleWidgetPrivate::createController - Node already added to module";
    return;
    }

  // create the ControllerWidget and wire it to the appropriate node
  vtkMRMLSliceNode *sn = vtkMRMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    qSlicerReslicePropertyWidget *widget = new qSlicerReslicePropertyWidget( this->logic(), this->resliceCollapsibleButton );
    //widget->setSliceViewName( sn->GetName() ); // call before setting slice node
    widget->setSliceViewName( sn->GetLayoutLabel() );
    widget->setMRMLSliceNode( sn );
    QColor layoutColor = QColor::fromRgbF(sn->GetLayoutColor()[0], sn->GetLayoutColor()[1], sn->GetLayoutColor()[2]);
    widget->setSliceViewColor( layoutColor );
    widget->setLayoutBehavior( qMRMLViewControllerBar::Panel );
    widget->showAdvanced( this->showAdvanced );
    resliceLayout->addWidget(widget);
    this->WidgetMap[n] = widget;
    
    this->logic()->InvokeEvent( vtkCommand::ModifiedEvent );
    }
}

//-----------------------------------------------------------------------------
void 
qSlicerVolumeResliceDriverModuleWidgetPrivate::removeController(vtkMRMLNode *n)
{
  // find the widget for the SliceNode
  WidgetMapType::iterator cit = this->WidgetMap.find(n);
  if (cit == this->WidgetMap.end())
    {
    qDebug() << "qSlicerVolumeResliceDriverModuleWidgetPrivate::removeController - Node has no Controller managed by this module.";
    return;
    }

  // unpack the widget
  vtkMRMLSliceNode *sn = vtkMRMLSliceNode::SafeDownCast(n);
  if (sn)
    {
    resliceLayout->removeWidget((*cit).second);
    }

  // delete the widget
  delete (*cit).second;

  // remove entry from the map
  this->WidgetMap.erase(cit);
}



//-----------------------------------------------------------------------------
// qSlicerVolumeResliceDriverModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeResliceDriverModuleWidget::qSlicerVolumeResliceDriverModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerVolumeResliceDriverModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeResliceDriverModuleWidget::~qSlicerVolumeResliceDriverModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeResliceDriverModuleWidget::setup()
{
  Q_D(qSlicerVolumeResliceDriverModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect( d->advancedCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onAdvancedCheckBoxChanged( int ) ) );
}



//-----------------------------------------------------------------------------
void qSlicerVolumeResliceDriverModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerVolumeResliceDriverModuleWidget);

  vtkMRMLScene* oldScene = this->mrmlScene();

  this->Superclass::setMRMLScene(newScene);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  // Search the scene for the available view nodes and create a
  // Controller and connect it up
  newScene->InitTraversal();
  for (vtkMRMLNode *sn = NULL; (sn=newScene->GetNextNodeByClass("vtkMRMLSliceNode"));)
    {
    vtkMRMLSliceNode *snode = vtkMRMLSliceNode::SafeDownCast(sn);
    if (snode)
      {
      d->createController(snode, layoutManager);
      }
    }

  // Need to listen for any new slice or view nodes being added
  this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));

  // Need to listen for any slice or view nodes being removed
  this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));

  // Listen to changes in the Layout so we only show controllers for
  // the visible nodes
  QObject::connect(layoutManager, SIGNAL(layoutChanged(int)), this, SLOT(onLayoutChanged(int)));

}


// --------------------------------------------------------------------------
void qSlicerVolumeResliceDriverModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerVolumeResliceDriverModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    qDebug() << "qSlicerVolumeResliceDriverModuleWidget::onNodeAddedEvent - layoutName:" << layoutName;

    // create the slice controller
    d->createController(sliceNode, layoutManager);
    }

}

// --------------------------------------------------------------------------
void qSlicerVolumeResliceDriverModuleWidget::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerVolumeResliceDriverModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    qDebug() << "qSlicerVolumeResliceDriverModuleWidget::onNodeRemovedEvent - layoutName:" << layoutName;
                                             
    // destroy the slice controller
    d->removeController(sliceNode);
    }

}

// --------------------------------------------------------------------------
void qSlicerVolumeResliceDriverModuleWidget::onLayoutChanged(int)
{
  Q_D(qSlicerVolumeResliceDriverModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  qDebug() << "qSlicerVolumeResliceDriverModuleWidget::onLayoutChanged";

  // add the controllers for any newly visible SliceNodes and remove
  // the controllers for any SliceNodes no longer visible

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }
  
  vtkMRMLLayoutLogic *layoutLogic = layoutManager->layoutLogic();
  vtkCollection *visibleViews = layoutLogic->GetViewNodes();
  
  // hide Controllers for Nodes not currently visible in
  // the layout
  qSlicerVolumeResliceDriverModuleWidgetPrivate::WidgetMapType::iterator cit;
  for (cit = d->WidgetMap.begin(); cit != d->WidgetMap.end(); ++cit)
    {
    // is mananaged Node not currently displayed in the layout?
    if (!visibleViews->IsItemPresent((*cit).first))
      {
      // hide it
      (*cit).second->hide();
      }
    }

  // show Controllers for Nodes not currently being managed
  // by this widget
  vtkObject *v;
  for (visibleViews->InitTraversal(); (v = visibleViews->GetNextItemAsObject());)
    {
    vtkMRMLNode *vn = vtkMRMLNode::SafeDownCast(v);
    if (vn)
      {
      // find the controller
      qSlicerVolumeResliceDriverModuleWidgetPrivate::WidgetMapType::iterator cit = d->WidgetMap.find(vn);
      if (cit != d->WidgetMap.end())
        {
        // show it
        (*cit).second->show();
        }
      }
    }
}


void qSlicerVolumeResliceDriverModuleWidget::onAdvancedCheckBoxChanged( int state )
{
  Q_D(qSlicerVolumeResliceDriverModuleWidget);

  d->showAdvanced = state;

  qSlicerVolumeResliceDriverModuleWidgetPrivate::WidgetMapType::iterator cit;
  for (cit = d->WidgetMap.begin(); cit != d->WidgetMap.end(); ++cit)
    {
    (*cit).second->showAdvanced( d->showAdvanced );
    }
}
