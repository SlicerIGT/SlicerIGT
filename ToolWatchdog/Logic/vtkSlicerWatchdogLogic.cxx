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

//Qt includes
//#include <QDir>
#include <QtCore/QObject>
#include <QEvent>
#include <QTimer>

//#include "qMRMLWatchdogToolBar.h"

#include "QVTKSlicerWatchdogLogicInternal.h"

// STD includes
#include <cassert>
#include <limits>

QVTKSlicerWatchdogLogicInternal::QVTKSlicerWatchdogLogicInternal(vtkSlicerWatchdogLogic* p)
: Parent(p)
{

}

QVTKSlicerWatchdogLogicInternal::~QVTKSlicerWatchdogLogicInternal()
{
}

void QVTKSlicerWatchdogLogicInternal::onTimerEvent()
{
  Parent->TimerEvent();
  emit updateTable();
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWatchdogLogic);

//----------------------------------------------------------------------------
vtkSlicerWatchdogLogic::vtkSlicerWatchdogLogic()
{
  //vtkDebugMacro("Initialize watchdog logic!");
  this->Internal = new QVTKSlicerWatchdogLogicInternal(this);
}

//----------------------------------------------------------------------------
vtkSlicerWatchdogLogic::~vtkSlicerWatchdogLogic()
{
  this->Internal->Timer->stop();
  QObject::connect( this->Internal->Timer, SIGNAL( timeout() ), this->Internal, SLOT( onTimerEvent() ) );
}

//----------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerWatchdogLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  //vtkDebugMacro("set scene internal watchdog logic!");
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
  //vtkDebugMacro("Register Nodes watchdog logic!");
  assert(this->GetMRMLScene() != 0);
  if( ! this->GetMRMLScene() )
  {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLWatchdogNode >::New() );

  this->Internal->Timer = new QTimer( this->Internal );

  ElapsedTimeSec=0.0;
  StatusRefreshTimeSec=0.20;

  QObject::connect( this->Internal->Timer, SIGNAL( timeout() ), this->Internal, SLOT( onTimerEvent() ) );
}

void vtkSlicerWatchdogLogic::AddToolNode( vtkMRMLWatchdogNode* watchdogNode, vtkMRMLDisplayableNode *toolNode)
{
  if ( watchdogNode == NULL )
  {
    return;
  }

  if(watchdogNode->AddToolNode(toolNode)&& !this->Internal->Timer->isActive())
  {
    this->Internal->Timer->start( 1000.0*StatusRefreshTimeSec );
  }
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
      (*it).lastElapsedTimeStamp=ElapsedTimeSec;
    }

  }
}

QVTKSlicerWatchdogLogicInternal* vtkSlicerWatchdogLogic::GetQVTKLogicInternal()
{
  return this->Internal;
}

void vtkSlicerWatchdogLogic::SetStatusRefreshTimeMiliSec( double statusRefeshRateMiliSec)
{
  this->Internal->Timer->stop();
  StatusRefreshTimeSec=((double)statusRefeshRateMiliSec)/1000;
  this->Internal->Timer->start(statusRefeshRateMiliSec);
}

void  vtkSlicerWatchdogLogic::TimerEvent()
{
  //vtkDebugMacro("Timer event");
  if(ElapsedTimeSec>=std::numeric_limits<double>::max()-1.0)
  {
    ElapsedTimeSec=0.0;
  }
  ElapsedTimeSec = ElapsedTimeSec+StatusRefreshTimeSec;

  vtkCollection* watchdogNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
  vtkCollectionIterator* watchdogNodeIt = vtkCollectionIterator::New();
  watchdogNodeIt->SetCollection( watchdogNodes );
  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
    //if(watchdogNode->WatchdogToolbar->isVisible())
    //{
      this->UpdateToolStatus( watchdogNode);
      std::list<WatchedTool>* toolsVectorPtr = watchdogNode->GetToolNodes();
      int numberTools = toolsVectorPtr->size();
      if ( toolsVectorPtr == NULL /*|| numberTools!= d->ToolsTableWidget->rowCount()*/)
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
    //}
  }
  watchdogNodeIt->Delete();
  watchdogNodes->Delete();
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
      //TODO Somewhere here should be initialized the widget toolbar 
      //watchdogNode->InitializeToolbar();
      vtkDebugMacro( "OnMRMLSceneEndImport: Module node added. Number of tools " <<watchdogNode->GetNumberOfTools());
      for (int i = 0; i< watchdogNode->GetNumberOfTools(); i++)
      {
        vtkMRMLDisplayableNode* dispNode= vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(watchdogNode->GetToolNode(i)->id));
        watchdogNode->GetToolNode(i)->tool=dispNode;
        //TODO Somewhere here add toolbar labels
        //watchdogNode->WatchdogToolbar->ToolNodeAdded(watchdogNode->GetToolNode(i)->label.c_str());
        vtkDebugMacro(" tool "<< watchdogNode->GetToolNode(i)->tool<<" ID "<< watchdogNode->GetToolNode(i)->id);
        hasTools=1;
      }
    }
  }
  watchdogNodeIt->Delete();
  watchdogNodes->Delete();
  if(hasTools==1)
  {
    this->Internal->Timer->start( 1000.0*StatusRefreshTimeSec );
  }
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
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added."<< node->GetName() );
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
    //watchdogNodeToBeRemoved->RemoveToolbar();

    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}
