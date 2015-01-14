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
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLTransformNode.h"
#include <vtkMRMLScene.h>


// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWatchdogLogic);

//----------------------------------------------------------------------------
vtkSlicerWatchdogLogic::vtkSlicerWatchdogLogic()
{
  vtkWarningMacro("Initialize watchdog logic!");
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

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkWarningMacro("set scene internal watchdog logic!");
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::RegisterNodes()
{
  vtkWarningMacro("Register Nodes watchdog logic!");
  assert(this->GetMRMLScene() != 0);
  if( ! this->GetMRMLScene() )
  {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLWatchdogNode >::New() );
}

void vtkSlicerWatchdogLogic::AddToolNode( vtkMRMLWatchdogNode* watchdogNode, vtkMRMLDisplayableNode *toolNode)
{
  if ( watchdogNode == NULL )
  {
    return;
  }
  watchdogNode->AddToolNode(toolNode);
}

void vtkSlicerWatchdogLogic::UpdateToolStatus( vtkMRMLWatchdogNode* watchdogNode, unsigned long ElapsedTimeSec  )
{
  if ( watchdogNode == NULL )
  {
    return;
  }
  std::list<WatchedTool> * toolToRasVectorPtr = watchdogNode->GetToolNodes();

  if ( toolToRasVectorPtr==NULL )
  {
    return;
  }

  for (std::list<WatchedTool>::iterator it = toolToRasVectorPtr->begin() ; it != toolToRasVectorPtr->end(); ++it)
  {
    unsigned long timeStamp = 0; 
    if((*it).tool== NULL)
    {
      return;
    }
     
    vtkMRMLTransformNode* transform=vtkMRMLTransformNode::SafeDownCast((*it).tool);
    if (transform!=NULL)
    {
      timeStamp = transform->GetTransformToWorldMTime();
    }
    else
    {
      timeStamp=(*it).tool->GetMTime();
    }
    
    if(timeStamp ==(*it).lastTimeStamp )
    {
      (*it).status=OUT_OF_DATE;
      vtkWarningMacro("Time stamp is out of date"<<timeStamp);
    }
    else
    {
      (*it).status=UP_TO_DATE;
      (*it).lastTimeStamp=timeStamp;
      (*it).lastElapsedTimeStamp=ElapsedTimeSec;
    }

  }
}


//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLWatchdogNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );

    vtkUnObserveMRMLNodeMacro( node ); // Remove previous observers.
    vtkObserveMRMLNodeMacro( node );

    vtkMRMLWatchdogNode* watchdogNode =vtkMRMLWatchdogNode::SafeDownCast(node);
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Module node added. Number of tools" <<watchdogNode->GetNumberOfTools());
    for (int i = 0; i< watchdogNode->GetNumberOfTools(); i++)
    {
      vtkMRMLDisplayableNode* dispNode= vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(watchdogNode->GetToolNode(i)->id));
      watchdogNode->GetToolNode(i)->tool=dispNode;
      vtkWarningMacro(" tool "<< watchdogNode->GetToolNode(i)->tool<<" ID "<< watchdogNode->GetToolNode(i)->id)
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLWatchdogNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}


//void
//vtkSlicerWatchdogLogic
//::SetObservedToolNode( vtkMRMLDisplayableNode* newTool, vtkMRMLWatchdogNode* moduleNode )
//{
//  if ( newTool == NULL || moduleNode == NULL )
//  {
//    vtkWarningMacro( "SetObservedTransformNode: Transform or module node invalid" );
//    return;
//  }
//
//  //moduleNode->SetAndObserveToolNodeId( newTool->GetID() );
//}

//void
//vtkSlicerWatchdogLogic
//::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
//{
//  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
//  if ( callerNode == NULL )
//  {
//    return;
//  }
//
//  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( callerNode );
//  if ( watchdogNode == NULL )
//  {
//    return;
//  }
//
//  //UpdateFromMRMLScene();
//  //this->UpdateToolState( watchdogNode, ElapsedTime );
//  //this->UpdateModelColor( watchdogNode );
//  //if(PlayWarningSound==true)
//  //{
//  //  this->PlaySound();
//  //}
//}

