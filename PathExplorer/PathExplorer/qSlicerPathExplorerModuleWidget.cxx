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
#include "qSlicerPathExplorerModuleWidget.h"
#include "ui_qSlicerPathExplorerModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPathExplorerModuleWidgetPrivate: public Ui_qSlicerPathExplorerModuleWidget
{
public:
  qSlicerPathExplorerModuleWidgetPrivate();

  std::vector<qSlicerPathExplorerReslicingWidget*> ReslicingWidgetList;
};

//-----------------------------------------------------------------------------
// qSlicerPathExplorerModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPathExplorerModuleWidgetPrivate::qSlicerPathExplorerModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPathExplorerModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPathExplorerModuleWidget::qSlicerPathExplorerModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPathExplorerModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerModuleWidget::~qSlicerPathExplorerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::setup()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->EntryListSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
	  this, SLOT(onEntryNodeActivated(vtkMRMLNode*)));

  connect(d->TargetListSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
	  this, SLOT(onTargetNodeActivated(vtkMRMLNode*)));

  connect(d->TrajectoryListSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
	  this, SLOT(onTrajectoryNodeActivated(vtkMRMLNode*)));

  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
	  this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));
}


//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!newScene || !d->TrajectoryWidget)
    {
    return;
    }

  if (d->EntryListSelector && d->TargetListSelector &&
      d->TrajectoryListSelector)
    {
    d->EntryListSelector->addNode();
    d->TargetListSelector->addNode();
    d->TrajectoryListSelector->addNode();
    }

  if (d->VisualizationFrame && d->ReslicingLayout)
    {
    // Clear reslicing widget layout
    QLayoutItem* item;
    while ( ( item = d->ReslicingLayout->takeAt(0)) != NULL)
      {
      delete item->widget();
      delete item;
      }
    d->ReslicingWidgetList.clear();
    
    // Add reslicing widgets
    vtkMRMLSliceNode* redViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeRed"));
    vtkMRMLSliceNode* yellowViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeYellow"));
    vtkMRMLSliceNode* greenViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeGreen"));
    
    qSlicerPathExplorerReslicingWidget* redReslicer =
      new qSlicerPathExplorerReslicingWidget(redViewer, d->VisualizationFrame);
    redReslicer->setMRMLScene(this->mrmlScene());
    d->ReslicingLayout->addWidget(redReslicer);
    d->ReslicingWidgetList.push_back(redReslicer);

    qSlicerPathExplorerReslicingWidget* yellowReslicer =
      new qSlicerPathExplorerReslicingWidget(yellowViewer, d->VisualizationFrame);
    yellowReslicer->setMRMLScene(this->mrmlScene());
    d->ReslicingLayout->addWidget(yellowReslicer);
    d->ReslicingWidgetList.push_back(yellowReslicer);

    qSlicerPathExplorerReslicingWidget* greenReslicer =
      new qSlicerPathExplorerReslicingWidget(greenViewer, d->VisualizationFrame);
    greenReslicer->setMRMLScene(this->mrmlScene());
    d->ReslicingLayout->addWidget(greenReslicer);
    d->ReslicingWidgetList.push_back(greenReslicer);

    connect(d->TrajectoryWidget, SIGNAL(selectedRulerChanged(vtkMRMLAnnotationRulerNode*)),
	    redReslicer, SLOT(setReslicingRulerNode(vtkMRMLAnnotationRulerNode*)));

    connect(d->TrajectoryWidget, SIGNAL(selectedRulerChanged(vtkMRMLAnnotationRulerNode*)),
	    yellowReslicer, SLOT(setReslicingRulerNode(vtkMRMLAnnotationRulerNode*)));

    connect(d->TrajectoryWidget, SIGNAL(selectedRulerChanged(vtkMRMLAnnotationRulerNode*)),
	    greenReslicer, SLOT(setReslicingRulerNode(vtkMRMLAnnotationRulerNode*)));
    }
}


//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onEntryNodeActivated(vtkMRMLNode* node)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLMarkupsFiducialNode* markupNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
  if (markupNode && d->EntryWidget && d->TrajectoryWidget)
    {
    // Disconnect previous signals
    vtkMRMLMarkupsFiducialNode* previousNode =
      d->EntryWidget->getMarkupFiducialNode();
    if (markupNode == previousNode)
      {
      return;
      }

    if (previousNode)
      {
      disconnect(d->EntryWidget, SIGNAL(markupSelected(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TrajectoryWidget, SLOT(setSelectedEntryMarkupID(vtkMRMLMarkupsFiducialNode*,int)));
      disconnect(d->EntryWidget, SIGNAL(markupModified(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TrajectoryWidget, SLOT(onEntryMarkupModified(vtkMRMLMarkupsFiducialNode*,int)));
      disconnect(d->EntryWidget, SIGNAL(markupRemoved(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TrajectoryWidget, SLOT(onEntryMarkupRemoved(vtkMRMLMarkupsFiducialNode*,int)));
      
      disconnect(d->TrajectoryWidget, SIGNAL(entryPointModified(vtkMRMLMarkupsFiducialNode*,int)),
		 d->EntryWidget, SLOT(setSelectedMarkup(vtkMRMLMarkupsFiducialNode*,int)));      
      }

    // Set new markup node
    d->EntryWidget->setAndObserveMarkupFiducialNode(markupNode);
    d->EntryWidget->setColor(0.3, 0.4, 0.7);
    d->TrajectoryWidget->setEntryMarkupsFiducialNode(markupNode);
    
    // Connect signals
    connect(d->EntryWidget, SIGNAL(addButtonToggled(bool)),
	    this, SLOT(onEntryAddButtonToggled(bool)));

    connect(d->EntryWidget, SIGNAL(markupSelected(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TrajectoryWidget, SLOT(setSelectedEntryMarkupID(vtkMRMLMarkupsFiducialNode*,int)));
    connect(d->EntryWidget, SIGNAL(markupModified(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TrajectoryWidget, SLOT(onEntryMarkupModified(vtkMRMLMarkupsFiducialNode*,int)));
    connect(d->EntryWidget, SIGNAL(markupRemoved(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TrajectoryWidget, SLOT(onEntryMarkupRemoved(vtkMRMLMarkupsFiducialNode*,int)));

    connect(d->TrajectoryWidget, SIGNAL(entryPointModified(vtkMRMLMarkupsFiducialNode*,int)),
	    d->EntryWidget, SLOT(setSelectedMarkup(vtkMRMLMarkupsFiducialNode*,int)));
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onTargetNodeActivated(vtkMRMLNode* node)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLMarkupsFiducialNode* markupNode =
    vtkMRMLMarkupsFiducialNode::SafeDownCast(node);
  if (markupNode && d->TargetWidget && d->TrajectoryWidget)
    {
    // Disconnect previous signals
    vtkMRMLMarkupsFiducialNode* previousNode =
      d->TargetWidget->getMarkupFiducialNode();
    if (markupNode == previousNode)
      {
      return;
      }

    if (previousNode)
      {
      disconnect(d->TargetWidget, SIGNAL(markupSelected(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TrajectoryWidget, SLOT(setSelectedTargetMarkupID(vtkMRMLMarkupsFiducialNode*,int)));
      disconnect(d->TargetWidget, SIGNAL(markupModified(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TrajectoryWidget, SLOT(onTargetMarkupModified(vtkMRMLMarkupsFiducialNode*,int)));
      disconnect(d->TargetWidget, SIGNAL(markupRemoved(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TrajectoryWidget, SLOT(onTargetMarkupRemoved(vtkMRMLMarkupsFiducialNode*,int)));
      
      disconnect(d->TrajectoryWidget, SIGNAL(entryPointModified(vtkMRMLMarkupsFiducialNode*,int)),
		 d->TargetWidget, SLOT(setSelectedMarkup(vtkMRMLMarkupsFiducialNode*,int)));      
      }

    // Set new markup node
    d->TargetWidget->setAndObserveMarkupFiducialNode(markupNode);
    d->TargetWidget->setColor(0.2, 0.8, 0.1);
    d->TrajectoryWidget->setTargetMarkupsFiducialNode(markupNode);
    
    // Connect signals
    connect(d->TargetWidget, SIGNAL(addButtonToggled(bool)),
	    this, SLOT(onTargetAddButtonToggled(bool)));

    connect(d->TargetWidget, SIGNAL(markupSelected(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TrajectoryWidget, SLOT(setSelectedTargetMarkupID(vtkMRMLMarkupsFiducialNode*,int)));
    connect(d->TargetWidget, SIGNAL(markupModified(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TrajectoryWidget, SLOT(onTargetMarkupModified(vtkMRMLMarkupsFiducialNode*,int)));
    connect(d->TargetWidget, SIGNAL(markupRemoved(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TrajectoryWidget, SLOT(onTargetMarkupRemoved(vtkMRMLMarkupsFiducialNode*,int)));

    connect(d->TrajectoryWidget, SIGNAL(entryPointModified(vtkMRMLMarkupsFiducialNode*,int)),
	    d->TargetWidget, SLOT(setSelectedMarkup(vtkMRMLMarkupsFiducialNode*,int)));
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onTrajectoryNodeActivated(vtkMRMLNode* node)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLAnnotationHierarchyNode* trajectoryNode =
    vtkMRMLAnnotationHierarchyNode::SafeDownCast(node);
  if (trajectoryNode && d->TrajectoryWidget)
    {
    d->TrajectoryWidget->setTrajectoryListNode(trajectoryNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onEntryAddButtonToggled(bool state)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (d->TargetWidget)
    {
    if (d->TargetWidget->getAddButtonState() && state)
      {
      d->TargetWidget->setAddButtonState(!state);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onTargetAddButtonToggled(bool state)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (d->EntryWidget)
    {
    if (d->EntryWidget->getAddButtonState() && state)
      {
      d->EntryWidget->setAddButtonState(!state);
      }
    }
}
