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
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>

//// STD includes
//#include <limits>

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

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  if( ! this->GetMRMLScene() )
  {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLWatchdogNode >::New() );

  this->ElapsedTimeSec=0.0;
  this->StatusRefreshTimeSec=0.20;
}

void vtkSlicerWatchdogLogic::AddToolNode( vtkMRMLWatchdogNode* watchdogNode, vtkMRMLDisplayableNode *toolNode)
{
  if ( watchdogNode == NULL )
  {
    return;
  }
  watchdogNode->AddToolNode(toolNode);
}

void vtkSlicerWatchdogLogic::UpdateToolStatus( vtkMRMLWatchdogNode* watchdogNode )
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
      vtkDebugMacro("Time stamp is out of date"<<timeStamp);
    }
    else
    {
      (*it).status=UP_TO_DATE;
      (*it).lastTimeStamp=timeStamp;
      (*it).lastElapsedTimeStamp=this->ElapsedTimeSec;
    }
  }
}

void  vtkSlicerWatchdogLogic::UpdateWatchdogNodes()
{
  //if(this->ElapsedTimeSec==std::numeric_limits<double>::max()-1.0)
  //{
  //  this->ElapsedTimeSec=0.0;
  //}
  this->ElapsedTimeSec = this->ElapsedTimeSec+this->StatusRefreshTimeSec;

  vtkCollection* watchdogNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
  vtkCollectionIterator* watchdogNodeIt = vtkCollectionIterator::New();
  watchdogNodeIt->SetCollection( watchdogNodes );
  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
      this->UpdateToolStatus( watchdogNode);
      std::list<WatchedTool>* toolsVectorPtr = watchdogNode->GetToolNodes();
      int numberTools = toolsVectorPtr->size();
      if ( toolsVectorPtr == NULL)
      {
        return;
      }
      int row=0;
      for (std::list<WatchedTool>::iterator itTool = toolsVectorPtr->begin() ; itTool != toolsVectorPtr->end(); ++itTool)
      {
        if((*itTool).tool==NULL)
        {
          return;
        }
        row++;
      }
  }
  watchdogNodeIt->Delete();
  watchdogNodes->Delete();

  this->InvokeEvent(WatchdogLogicUpdatedEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::OnMRMLSceneEndImport()
{
  assert(this->GetMRMLScene() != 0);
  vtkCollection* watchdogNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
  vtkCollectionIterator* watchdogNodeIt = vtkCollectionIterator::New();
  watchdogNodeIt->SetCollection( watchdogNodes );
  int hasTools=0;
  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
    if ( watchdogNode != NULL)
    {
      vtkDebugMacro( "OnMRMLSceneEndImport: Module node added. Number of tools " <<watchdogNode->GetNumberOfTools());
      for (int i = 0; i< watchdogNode->GetNumberOfTools(); i++)
      {
        vtkMRMLDisplayableNode* dispNode= vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(watchdogNode->GetToolNode(i)->id));
        watchdogNode->GetToolNode(i)->tool=dispNode;
        //vtkDebugMacro(" tool "<< watchdogNode->GetToolNode(i)->tool<<" ID "<< watchdogNode->GetToolNode(i)->id);
        hasTools=1;
      }
    }
  }
  watchdogNodeIt->Delete();
  watchdogNodes->Delete();
  this->Modified();
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
    //vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added."<< node->GetName() );
    vtkUnObserveMRMLNodeMacro( node ); // Remove previous observers.
    vtkObserveMRMLNodeMacro( node );
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
    vtkMRMLWatchdogNode* watchdogNodeToBeRemoved = vtkMRMLWatchdogNode::SafeDownCast( node );
    if(watchdogNodeToBeRemoved==NULL)
    {
      return;
    }

    //vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}
