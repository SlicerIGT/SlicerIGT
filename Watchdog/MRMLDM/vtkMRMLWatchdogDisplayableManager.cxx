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

This file was originally developed by Andras Lasso and Franklin King at
PerkLab, Queen's University and was supported through the Applied Cancer
Research Unit program of Cancer Care Ontario with funds provided by the
Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


// MRMLDisplayableManager includes
#include "vtkMRMLWatchdogDisplayableManager.h"

#include "vtkSlicerWatchdogLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLWatchdogDisplayNode.h>
#include <vtkMRMLWatchdogNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTriangleFilter.h>

const double TEXT_MARGIN_PERCENT=20; // margin around text, percentage of font size

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkMRMLWatchdogDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLWatchdogDisplayableManager::vtkInternal
{
public:

  vtkInternal(vtkMRMLWatchdogDisplayableManager* external);
  ~vtkInternal();

  struct Pipeline
  {
    vtkSmartPointer<vtkTextActor> TextActor;
    vtkSmartPointer<vtkActor2D> BackgroundActor;
    vtkSmartPointer<vtkPoints> BackgroundCornerPoints;
  };

  typedef std::map < vtkMRMLWatchdogDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLWatchdogNode*, std::set< vtkMRMLWatchdogDisplayNode* > > WatchdogToDisplayCacheType;
  WatchdogToDisplayCacheType WatchdogToDisplayNodes;

  // Watchdogs
  void AddWatchdogNode(vtkMRMLWatchdogNode* displayableNode);
  void RemoveWatchdogNode(vtkMRMLWatchdogNode* displayableNode);
  void UpdateDisplayableWatchdogs(vtkMRMLWatchdogNode *node);

  // Display Nodes
  void AddDisplayNode(vtkMRMLWatchdogNode*, vtkMRMLWatchdogDisplayNode*);
  void UpdateDisplayNode(vtkMRMLWatchdogDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLWatchdogDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkMRMLWatchdogDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLWatchdogNode* node);
  void RemoveObservations(vtkMRMLWatchdogNode* node);
  bool IsNodeObserved(vtkMRMLWatchdogNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLWatchdogDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLWatchdogDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLWatchdogNode* node);
  void ClearDisplayableNodes();

private:
  vtkMRMLWatchdogDisplayableManager* External;
  bool AddingWatchdogNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLWatchdogDisplayableManager::vtkInternal::vtkInternal(vtkMRMLWatchdogDisplayableManager * external)
: External(external)
, AddingWatchdogNode(false)
{
}

//---------------------------------------------------------------------------
vtkMRMLWatchdogDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLWatchdogDisplayableManager::vtkInternal::UseDisplayNode(vtkMRMLWatchdogDisplayNode* displayNode)
{
  //if (displayNode && !displayNode->IsDisplayableInView(this->External->GetMRMLViewNode()->GetID()))
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetMRMLDisplayableNode()->GetID()))
  {
    return false;
  }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkMRMLWatchdogDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkMRMLWatchdogDisplayableManager::vtkInternal::IsVisible(vtkMRMLWatchdogDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::AddWatchdogNode(vtkMRMLWatchdogNode* node)
{
  if (this->AddingWatchdogNode)
  {
    return;
  }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
  {
    return;
  }

  this->AddingWatchdogNode = true;

  // Add Display Nodes
  this->AddObservations(node);
  int nnodes = node->GetNumberOfDisplayNodes();
  for (int i=0; i<nnodes; i++)
  {
    vtkMRMLWatchdogDisplayNode *dnode = vtkMRMLWatchdogDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if ( this->UseDisplayNode(dnode) )
    {
      this->WatchdogToDisplayNodes[node].insert(dnode);
      this->AddDisplayNode( node, dnode );
    }
  }
  this->AddingWatchdogNode = false;
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::RemoveWatchdogNode(vtkMRMLWatchdogNode* node)
{
  if (!node)
  {
    return;
  }
  vtkInternal::WatchdogToDisplayCacheType::iterator displayableIt = this->WatchdogToDisplayNodes.find(node);
  if(displayableIt == this->WatchdogToDisplayNodes.end())
  {
    // already removed
    return;
  }

  std::set<vtkMRMLWatchdogDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkMRMLWatchdogDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
  {
    this->RemoveDisplayNode(*diter);
  }
  this->RemoveObservations(node);
  this->WatchdogToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::UpdateDisplayableWatchdogs(vtkMRMLWatchdogNode* mNode)
{
  // Update the pipeline for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkMRMLWatchdogDisplayNode* > displayNodes = this->WatchdogToDisplayNodes[mNode];
  std::set< vtkMRMLWatchdogDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
  {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
    {
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::RemoveDisplayNode(vtkMRMLWatchdogDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
  {
    return;
  }
  const Pipeline* pipeline = actorsIt->second;
  this->External->GetRenderer()->RemoveActor(pipeline->TextActor);
  this->External->GetRenderer()->RemoveActor(pipeline->BackgroundActor);
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::AddDisplayNode(vtkMRMLWatchdogNode* mNode, vtkMRMLWatchdogDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
  {
    return;
  }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a watchdog node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterwards.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
  {
    return;
  }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();

  pipeline->TextActor = vtkSmartPointer<vtkTextActor>::New();
  pipeline->TextActor->GetTextProperty()->SetColor(1, 1, 1);
  pipeline->TextActor->GetTextProperty()->SetBold(1);
  pipeline->TextActor->GetTextProperty()->SetShadow(1);
  pipeline->TextActor->SetVisibility(false);

  pipeline->BackgroundActor = vtkSmartPointer<vtkActor2D>::New();
  vtkNew<vtkPolyDataMapper2D> mapper;
  pipeline->BackgroundActor->SetMapper(mapper.GetPointer());
  pipeline->BackgroundActor->SetVisibility(false);

  // Add a rectangular area with 4 points
  pipeline->BackgroundCornerPoints = vtkSmartPointer<vtkPoints>::New();
  const int numberOfCornerPoints = 4;
  for (int i=0; i<numberOfCornerPoints; i++)
  {
    pipeline->BackgroundCornerPoints->InsertNextPoint(0, 0, 0);
  }
  vtkNew<vtkCellArray> polyCells;
  polyCells->InsertNextCell(numberOfCornerPoints + 1);
  for (int i=0; i<numberOfCornerPoints; i++)
  {
    polyCells->InsertCellPoint(i);
  }
  polyCells->InsertCellPoint(0); // Rejoin at the end
  vtkNew<vtkPolyData> rectanglePolyData;
  rectanglePolyData->SetPoints(pipeline->BackgroundCornerPoints);
  rectanglePolyData->SetPolys(polyCells.GetPointer());
  vtkNew<vtkTriangleFilter> triangleFilter;
#if VTK_MAJOR_VERSION <= 5
  triangleFilter->SetInput(rectanglePolyData.GetPointer());
#else
  triangleFilter->SetInputData(rectanglePolyData.GetPointer());
#endif
  mapper->SetInputConnection(triangleFilter->GetOutputPort());

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->BackgroundActor );
  this->External->GetRenderer()->AddActor( pipeline->TextActor );

  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  this->UpdateDisplayableWatchdogs(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::UpdateDisplayNode(vtkMRMLWatchdogDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update.
  //   otherwise, add as new node

  if (!displayNode)
  {
    return;
  }
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
  {
    this->UpdateDisplayNodePipeline(displayNode, it->second);
  }
  else
  {
    this->AddWatchdogNode( vtkMRMLWatchdogNode::SafeDownCast(displayNode->GetDisplayableNode()) );
  }
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLWatchdogDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline polydata input, update color
  //   calculate and set pipeline watchdogs.

  if (!displayNode || !pipeline)
  {
    return;
  }

  if ( !this->UseDisplayNode(displayNode) )
  {
    pipeline->TextActor->SetVisibility(false);
    pipeline->BackgroundActor->SetVisibility(false);
    return;
  }

  vtkMRMLWatchdogNode* watchdogNode=vtkMRMLWatchdogNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (watchdogNode==NULL)
  {
    pipeline->TextActor->SetVisibility(false);
    pipeline->BackgroundActor->SetVisibility(false);
    return;
  }

  std::string displayedText;
  int numberOfWatchedNodes = watchdogNode->GetNumberOfWatchedNodes();
  for (int watchedNodeIndex = 0; watchedNodeIndex < numberOfWatchedNodes; watchedNodeIndex++ )
  {
    if (!watchdogNode->GetWatchedNodeUpToDate(watchedNodeIndex))
    {
      // Node outdated, add warning text
      if (!displayedText.empty())
      {
        displayedText += "\n";
      }
      displayedText += watchdogNode->GetWatchedNodeWarningMessage(watchedNodeIndex);
    }
  }
  if (displayedText.empty())
  {
    pipeline->TextActor->SetVisibility(false);
    pipeline->BackgroundActor->SetVisibility(false);
    return;
  }

  pipeline->TextActor->SetInput(displayedText.c_str());
  pipeline->TextActor->GetTextProperty()->SetFontSize(displayNode->GetFontSize());
  int margin = TEXT_MARGIN_PERCENT/100.0*displayNode->GetFontSize();
  pipeline->TextActor->SetPosition(margin, margin);

  double boundingBox[4]={0};
  pipeline->TextActor->GetBoundingBox(this->External->GetRenderer(), boundingBox);
  boundingBox[0]-=margin;
  boundingBox[1]+=margin*2;
  boundingBox[2]-=margin;
  boundingBox[3]+=margin*2;
  pipeline->BackgroundCornerPoints->SetPoint(0,boundingBox[0],boundingBox[2],0);
  pipeline->BackgroundCornerPoints->SetPoint(1,boundingBox[1],boundingBox[2],0);
  pipeline->BackgroundCornerPoints->SetPoint(2,boundingBox[1],boundingBox[3],0);
  pipeline->BackgroundCornerPoints->SetPoint(3,boundingBox[0],boundingBox[3],0);

  // Update visibility
  bool visible = this->IsVisible(displayNode);
  pipeline->TextActor->SetVisibility(visible);
  pipeline->BackgroundActor->SetVisibility(visible);
  if (!visible)
  {
    return;
  }

  // Update properties

  pipeline->BackgroundActor->GetProperty()->SetPointSize(displayNode->GetPointSize());
  pipeline->BackgroundActor->GetProperty()->SetLineWidth(displayNode->GetLineWidth());
  pipeline->BackgroundActor->GetProperty()->SetColor(displayNode->GetEdgeColor());
  pipeline->BackgroundActor->GetProperty()->SetOpacity(displayNode->GetOpacity());
  if (displayNode->GetSelected())
  {
    pipeline->TextActor->GetTextProperty()->SetColor(displayNode->GetSelectedColor());
  }
  else
  {
    pipeline->TextActor->GetTextProperty()->SetColor(displayNode->GetColor());
  }


}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::AddObservations(vtkMRMLWatchdogNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
  {
    broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
  {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  }
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLWatchdogNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLWatchdogDisplayableManager::vtkInternal::IsNodeObserved(vtkMRMLWatchdogNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations = broker->GetObservationsForSubject(node);
  if (observations->GetNumberOfItems() > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::vtkInternal::ClearDisplayableNodes()
{
  while(this->WatchdogToDisplayNodes.size() > 0)
  {
    this->RemoveWatchdogNode(this->WatchdogToDisplayNodes.begin()->first);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLWatchdogDisplayableManager::vtkInternal::UseDisplayableNode(vtkMRMLWatchdogNode* node)
{
  bool use = node && node->IsA("vtkMRMLWatchdogNode");
  return use;
}

//---------------------------------------------------------------------------
// vtkMRMLWatchdogDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLWatchdogDisplayableManager::vtkMRMLWatchdogDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLWatchdogDisplayableManager::~vtkMRMLWatchdogDisplayableManager()
{
  delete this->Internal;
  this->Internal=NULL;
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkMRMLWatchdogDisplayableManager: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( !node->IsA("vtkMRMLWatchdogNode") )
  {
    return;
  }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    this->SetUpdateFromMRMLRequested(1);
    return;
  }

  this->Internal->AddWatchdogNode(vtkMRMLWatchdogNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node
    && (!node->IsA("vtkMRMLWatchdogNode"))
    && (!node->IsA("vtkMRMLWatchdogDisplayNode")) )
  {
    return;
  }

  vtkMRMLWatchdogNode* watchdogNode = NULL;
  vtkMRMLWatchdogDisplayNode* displayNode = NULL;

  bool modified = false;
  if ( (watchdogNode = vtkMRMLWatchdogNode::SafeDownCast(node)) )
  {
    this->Internal->RemoveWatchdogNode(watchdogNode);
    modified = true;
  }
  else if ( (displayNode = vtkMRMLWatchdogDisplayNode::SafeDownCast(node)) )
  {
    this->Internal->RemoveDisplayNode(displayNode);
    modified = true;
  }
  if (modified)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if ( scene->IsBatchProcessing() )
  {
    return;
  }

  vtkMRMLWatchdogNode* displayableNode = vtkMRMLWatchdogNode::SafeDownCast(caller);

  if ( displayableNode )
  {
    vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
    vtkMRMLWatchdogDisplayNode* displayNode = vtkMRMLWatchdogDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkMRMLDisplayableNode::DisplayModifiedEvent) )
    {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
    }
    else if (event == vtkCommand::ModifiedEvent)
    {
      this->Internal->UpdateDisplayableWatchdogs(displayableNode);
      this->RequestRender();
    }
  }
  else
  {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(0);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkDebugMacro( "vtkMRMLWatchdogDisplayableManager->UpdateFromMRML: Scene is not set.")
      return;
  }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLWatchdogNode* mNode = NULL;
  std::vector<vtkMRMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkMRMLWatchdogNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
  {
    mNode  = vtkMRMLWatchdogNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
    {
      this->Internal->AddWatchdogNode(mNode);
    }
  }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(1);
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLWatchdogDisplayableManager::Create()
{
  Superclass::Create();
  this->SetUpdateFromMRMLRequested(1);
}
