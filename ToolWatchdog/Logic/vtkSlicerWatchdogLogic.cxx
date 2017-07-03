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

// Watchdog Logic includes
#include "vtkSlicerWatchdogLogic.h"

// MRML includes
#include "vtkMRMLWatchdogNode.h"
#include "vtkMRMLWatchdogDisplayNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWatchdogLogic);

//----------------------------------------------------------------------------
vtkSlicerWatchdogLogic::vtkSlicerWatchdogLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerWatchdogLogic::~vtkSlicerWatchdogLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    vtkErrorMacro( "vtkSlicerWatchdogLogic::RegisterNodes failed: MRML scene is invalid" );
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLWatchdogNode >::New() );
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLWatchdogDisplayNode >::New() );
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::UpdateAllWatchdogNodes(bool &watchedNodeBecomeUpToDateSound, bool &watchedNodeBecomeOutdatedSound)
{
  watchedNodeBecomeUpToDateSound = false;
  watchedNodeBecomeOutdatedSound = false;
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene==NULL)
  {
    return;
  }
  vtkSmartPointer<vtkCollection> watchdogNodes = vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLWatchdogNode"));
  vtkSmartPointer<vtkCollectionIterator> watchdogNodeIt = vtkSmartPointer<vtkCollectionIterator>::New();
  watchdogNodeIt->SetCollection( watchdogNodes );
  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
    if (watchdogNode==NULL)
    {
      continue;
    }
    watchdogNode->UpdateWatchedNodesStatus(watchedNodeBecomeUpToDateSound, watchedNodeBecomeOutdatedSound);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  vtkMRMLWatchdogNode *watchdogNode = vtkMRMLWatchdogNode::SafeDownCast(node);
  if (!watchdogNode)
    {
    return;
    }
  if (watchdogNode->GetDisplayNode() == NULL)
    {
    // add a display node
    int modifyFlag = watchdogNode->StartModify();
    std::string displayNodeID = this->AddNewDisplayNodeForWatchdogNode(watchdogNode);
    watchdogNode->EndModify(modifyFlag);
    vtkDebugMacro("Added a display node with id " << displayNodeID.c_str()
                  << " for watchdog node with id " << watchdogNode->GetID());
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerWatchdogLogic::AddNewDisplayNodeForWatchdogNode(vtkMRMLNode *mrmlNode)
{
  std::string id;
  if (!mrmlNode || !mrmlNode->GetScene())
    {
    vtkErrorMacro("AddNewDisplayNodeForWatchdogNode: unable to add a watchdog display node!");
    return id;
    }

  // is there already a display node?
  vtkMRMLDisplayableNode *displayableNode = vtkMRMLDisplayableNode::SafeDownCast(mrmlNode);
  if (displayableNode && displayableNode->GetDisplayNode() != NULL)
    {
    return displayableNode->GetDisplayNodeID();
    }

  // create the display node
  vtkMRMLWatchdogDisplayNode *displayNode = vtkMRMLWatchdogDisplayNode::New();
  // set it from the defaults
   vtkDebugMacro("AddNewDisplayNodeForWatchdogNode: set display node to defaults");

  // add it to the scene
  //mrmlNode->GetScene()->AddNode(displayNode);
  vtkMRMLNode *n = mrmlNode->GetScene()->InsertBeforeNode(mrmlNode, displayNode);
  if (!n)
    {
    vtkErrorMacro("AddNewDisplayNodeForWatchdogNode: error on insert before node");
    return id;
    }

  // get the node id to return
  id = std::string(displayNode->GetID());

  // cast to watchdog node
  vtkMRMLWatchdogNode *watchdogNode = vtkMRMLWatchdogNode::SafeDownCast(mrmlNode);
  if (watchdogNode)
    {
    // observe the display node
    watchdogNode->DisableModifiedEventOn();
    watchdogNode->AddAndObserveDisplayNodeID(id.c_str());
    watchdogNode->DisableModifiedEventOff();
    }

  // clean up
  displayNode->Delete();

  return id;
}

//---------------------------------------------------------------------------
std::string vtkSlicerWatchdogLogic::AddNewWatchdogNode(const char *name, vtkMRMLScene *scene)
{
  if (!scene && !this->GetMRMLScene())
    {
    vtkErrorMacro("AddNewWatchdogNode: no scene to add a watchdog node to");
    return "";
    }

  vtkMRMLScene *addToThisScene;
  if (scene)
    {
    addToThisScene = scene;
    }
  else
    {
    addToThisScene = this->GetMRMLScene();
    }

  // create and add the node
  vtkMRMLWatchdogNode mnode = addToThisScene->AddNewNodeByClass("vtkMRMLWatchdogNode");
  if (name != NULL)
    {
    mnode->SetName(name);
    }

  // add a display node
  this->AddNewDisplayNodeForWatchdogNode(mnode);

  std::string id = (mnode->GetID() ? mnode->GetID() : "");
  return id;
}
