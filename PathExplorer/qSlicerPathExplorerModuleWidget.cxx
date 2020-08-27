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
#include <QShortcut>

// SlicerQt includes
#include "qSlicerPathExplorerModuleWidget.h"
#include "qMRMLSubjectHierarchyModel.h"

#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMRMLPathPlannerTrajectoryNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"

#include "ui_qSlicerPathExplorerModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPathExplorerModuleWidgetPrivate : public Ui_qSlicerPathExplorerModuleWidget
{
public:
  qSlicerPathExplorerModuleWidgetPrivate();

  qSlicerPathExplorerReslicingWidget* RedReslicer;
  qSlicerPathExplorerReslicingWidget* YellowReslicer;
  qSlicerPathExplorerReslicingWidget* GreenReslicer;
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
  : Superclass(_parent)
  , d_ptr(new qSlicerPathExplorerModuleWidgetPrivate)
{
  this->eToAddShortcut = 0;
  this->tToAddShortcut = 0;
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

  d->EntryWidget->markupsSelectorComboBox()->setBaseName("Entry");
  d->TargetWidget->markupsSelectorComboBox()->setBaseName("Target");

  d->TrajectoryTreeView->setColumnHidden(d->TrajectoryTreeView->model()->idColumn(), true);
  d->TrajectoryTreeView->setColumnHidden(d->TrajectoryTreeView->model()->transformColumn(), true);
  // TODO: display entry and target point names in description. It can only be implemented if measurements can be disabled,
  // because currently measurement is displayed in description.
  d->TrajectoryTreeView->setColumnHidden(d->TrajectoryTreeView->model()->descriptionColumn(), true);

  // Add reslicing widgets
  d->RedReslicer = new qSlicerPathExplorerReslicingWidget();
  d->ReslicingLayout->addWidget(d->RedReslicer);

  d->YellowReslicer = new qSlicerPathExplorerReslicingWidget();
  d->ReslicingLayout->addWidget(d->YellowReslicer);

  d->GreenReslicer = new qSlicerPathExplorerReslicingWidget();
  d->ReslicingLayout->addWidget(d->GreenReslicer);

  connect(d->AddPathButton, SIGNAL(clicked()), this, SLOT(onAddPath()));

  connect(d->TrajectoryTreeView, SIGNAL(currentItemChanged(vtkIdType)), this, SLOT(selectedPathLineItem(vtkIdType)));

  connect(d->TrajectoryListSelector, SIGNAL(nodeActivated(vtkMRMLNode*)), this, SLOT(onTrajectoryNodeActivated(vtkMRMLNode*)));
  connect(d->EntryWidget, SIGNAL(markupsNodeChanged()), this, SLOT(onEntryNodeSelected()));
  connect(d->TargetWidget, SIGNAL(markupsNodeChanged()), this, SLOT(onTargetNodeSelected()));

  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));

  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->RedReslicer, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));
  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->YellowReslicer, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));
  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->GreenReslicer, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::selectedPathLineItem(vtkIdType selectedItemId)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
  if (!shNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: Unable to access subject hierarchy node";
    return;
  }
  vtkMRMLMarkupsLineNode* pathLineNode = vtkMRMLMarkupsLineNode::SafeDownCast(shNode->GetItemDataNode(selectedItemId));

  d->RedReslicer->setReslicingRulerNode(pathLineNode);
  d->YellowReslicer->setReslicingRulerNode(pathLineNode);
  d->GreenReslicer->setReslicingRulerNode(pathLineNode);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::enter()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  if (this->eToAddShortcut == 0)
  {
    this->eToAddShortcut = new QShortcut(QKeySequence(QString("e")), this);
  }

  if (this->tToAddShortcut == 0)
  {
    this->tToAddShortcut = new QShortcut(QKeySequence(QString("t")), this);
  }

  QObject::connect(this->eToAddShortcut, SIGNAL(activated()),
    this, SLOT(onEKeyPressed()));

  QObject::connect(this->tToAddShortcut, SIGNAL(activated()),
    this, SLOT(onTKeyPressed()));

  if (!d->TrajectoryListSelector->currentNode())
  {
    vtkMRMLPathPlannerTrajectoryNode* trajectoryNode = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(
      this->mrmlScene()->GetFirstNodeByClass("vtkMRMLPathPlannerTrajectoryNode"));
    if (!trajectoryNode)
    {
      trajectoryNode = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLPathPlannerTrajectoryNode", "Trajectory list"));
    }
    d->TrajectoryListSelector->setCurrentNode(trajectoryNode);
  }
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onEKeyPressed()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  d->EntryWidget->placeActive(true);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onTKeyPressed()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  d->TargetWidget->placeActive(true);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onAddPath()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  vtkMRMLPathPlannerTrajectoryNode* trajectory = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(d->TrajectoryListSelector->currentNode());
  vtkMRMLMarkupsNode* entryPointsNode = vtkMRMLMarkupsNode::SafeDownCast(d->EntryWidget->currentNode());
  vtkMRMLMarkupsNode* targetPointsNode = vtkMRMLMarkupsNode::SafeDownCast(d->TargetWidget->currentNode());
  int entryPointIndex = d->EntryWidget->tableWidget()->currentRow();
  int targetPointIndex = d->TargetWidget->tableWidget()->currentRow();
  if (!trajectory || !entryPointsNode || !targetPointsNode || entryPointIndex < 0 || targetPointIndex < 0)
  {
    return;
  }
  trajectory->AddPath(
    entryPointsNode->GetNthControlPointID(entryPointIndex),
    targetPointsNode->GetNthControlPointID(targetPointIndex));
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::exit()
{
  this->eToAddShortcut->disconnect(SIGNAL(activated()));
  this->tToAddShortcut->disconnect(SIGNAL(activated()));
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  Q_D(qSlicerPathExplorerModuleWidget);

  if (!newScene)
  {
    return;
  }

  // Add reslicing widgets
  vtkMRMLSliceNode* redViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeRed"));
  vtkMRMLSliceNode* yellowViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeYellow"));
  vtkMRMLSliceNode* greenViewer = vtkMRMLSliceNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLSliceNodeGreen"));

  d->RedReslicer->setSliceNode(redViewer);
  d->YellowReslicer->setSliceNode(yellowViewer);
  d->GreenReslicer->setSliceNode(greenViewer);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onTrajectoryNodeActivated(vtkMRMLNode* node)
{
  Q_D(qSlicerPathExplorerModuleWidget);
  vtkMRMLPathPlannerTrajectoryNode* trajectoryNode = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(node);
  if (trajectoryNode)
  {
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
    d->TrajectoryTreeView->setRootItem(shNode->GetItemByDataNode(trajectoryNode));
    d->TrajectoryTreeView->setColumnHidden(d->TrajectoryTreeView->model()->idColumn(), true);
    d->TrajectoryTreeView->setColumnHidden(d->TrajectoryTreeView->model()->transformColumn(), true);
    // TODO: move entry->target points to description column
    d->TrajectoryTreeView->setColumnHidden(d->TrajectoryTreeView->model()->descriptionColumn(), true);
    d->TrajectoryTreeView->resetColumnSizesToDefault(); 
  }
  else
  {
    d->TrajectoryTreeView->setRootItem(-1);
  }
  // Find/pre-create entry and target points for user's convenience
  vtkMRMLMarkupsNode* entryPoints = trajectoryNode->GetEntryPointsNode();
  vtkMRMLMarkupsNode* targetPoints = trajectoryNode->GetTargetPointsNode();
  if (!entryPoints || !targetPoints)
  {
    // First try to find existing fiducial nodes
    int numberOfMarkupFiducialNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialNode");
    for (int nodeIndex = 0; nodeIndex < numberOfMarkupFiducialNodes && (!entryPoints || !targetPoints); nodeIndex++)
    {
      vtkMRMLMarkupsFiducialNode* pointListNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
        this->mrmlScene()->GetNthNodeByClass(nodeIndex, "vtkMRMLMarkupsFiducialNode"));
      if (!pointListNode)
        {
        continue;
        }
      if (!entryPoints && pointListNode != targetPoints)
      {
        entryPoints = pointListNode;
        continue;
      }
      if (!targetPoints && pointListNode != entryPoints)
      {
        targetPoints = pointListNode;
        continue;
      }
    }
    // If no usable point list nodes found then create new ones
    if (!entryPoints)
    {
      entryPoints = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode",
        this->mrmlScene()->GetUniqueNameByString("Entry")));
    }
    if (!targetPoints)
    {
      targetPoints = vtkMRMLMarkupsFiducialNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode",
        this->mrmlScene()->GetUniqueNameByString("Target")));
    }
    // Store nodes
    trajectoryNode->SetAndObserveEntryPointsNodeId(entryPoints->GetID());
    trajectoryNode->SetAndObserveTargetPointsNodeId(targetPoints->GetID());
  }
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onEntryNodeSelected()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  vtkMRMLPathPlannerTrajectoryNode* trajectory = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(d->TrajectoryListSelector->currentNode());
  if (!trajectory)
  {
    return;
  }
  vtkMRMLMarkupsNode* entryPointsNode = vtkMRMLMarkupsNode::SafeDownCast(d->EntryWidget->currentNode());
  trajectory->SetAndObserveEntryPointsNodeId(entryPointsNode ? entryPointsNode->GetID() : nullptr);
  vtkMRMLMarkupsNode* targetPointsNode = vtkMRMLMarkupsNode::SafeDownCast(d->TargetWidget->currentNode());
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::onTargetNodeSelected()
{
  Q_D(qSlicerPathExplorerModuleWidget);
  vtkMRMLPathPlannerTrajectoryNode* trajectory = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(d->TrajectoryListSelector->currentNode());
  if (!trajectory)
  {
    return;
  }
  vtkMRMLMarkupsNode* targetPointsNode = vtkMRMLMarkupsNode::SafeDownCast(d->TargetWidget->currentNode());
  trajectory->SetAndObserveTargetPointsNodeId(targetPointsNode ? targetPointsNode->GetID() : nullptr);
}

//------------------------------------------------------------------------------
void qSlicerPathExplorerModuleWidget::updateGUIFromMRML()
{
  Q_D(qSlicerPathExplorerModuleWidget);

  vtkMRMLPathPlannerTrajectoryNode* trajectoryNode = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(d->TrajectoryListSelector->currentNode());
  d->PlanningFrame->setEnabled(trajectoryNode!=nullptr);
  d->VisualizationFrame->setEnabled(trajectoryNode != nullptr);
  if (!trajectoryNode)
  {
    return;
  }

  d->EntryWidget->setCurrentNode(trajectoryNode->GetEntryPointsNode());
  d->TargetWidget->setCurrentNode(trajectoryNode->GetTargetPointsNode());
}
