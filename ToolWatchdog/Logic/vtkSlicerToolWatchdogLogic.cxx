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

// ToolWatchdog Logic includes
#include "vtkSlicerToolWatchdogLogic.h"

// MRML includes
#include "vtkMRMLToolWatchdogNode.h"
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
vtkStandardNewMacro(vtkSlicerToolWatchdogLogic);

//----------------------------------------------------------------------------
vtkSlicerToolWatchdogLogic::vtkSlicerToolWatchdogLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerToolWatchdogLogic::~vtkSlicerToolWatchdogLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerToolWatchdogLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerToolWatchdogLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerToolWatchdogLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  if( ! this->GetMRMLScene() )
  {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLToolWatchdogNode >::New() );
}

void vtkSlicerToolWatchdogLogic::AddToolNode( vtkMRMLToolWatchdogNode* toolWatchdogNode, vtkMRMLDisplayableNode *toolNode)
{
  if ( toolWatchdogNode == NULL )
  {
    return;
  }
  toolWatchdogNode->AddToolNode(toolNode);
}

void vtkSlicerToolWatchdogLogic::UpdateToolStatus( vtkMRMLToolWatchdogNode* toolWatchdogNode, unsigned long ElapsedTimeSec  )
{
  if ( toolWatchdogNode == NULL )
  {
    return;
  }
  std::list<WatchedTool> * toolToRasVectorPtr = toolWatchdogNode->GetToolNodes();

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
void vtkSlicerToolWatchdogLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerToolWatchdogLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLToolWatchdogNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( node ); // Remove previous observers.
    vtkObserveMRMLNodeMacro( node );
  }
}

//---------------------------------------------------------------------------
void vtkSlicerToolWatchdogLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLToolWatchdogNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}


//void
//vtkSlicerToolWatchdogLogic
//::SetObservedToolNode( vtkMRMLDisplayableNode* newTool, vtkMRMLToolWatchdogNode* moduleNode )
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
//vtkSlicerToolWatchdogLogic
//::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
//{
//  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
//  if ( callerNode == NULL )
//  {
//    return;
//  }
//
//  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( callerNode );
//  if ( toolWatchdogNode == NULL )
//  {
//    return;
//  }
//
//  //UpdateFromMRMLScene();
//  //this->UpdateToolState( toolWatchdogNode, ElapsedTime );
//  //this->UpdateModelColor( toolWatchdogNode );
//  //if(PlayWarningSound==true)
//  //{
//  //  this->PlaySound();
//  //}
//}

