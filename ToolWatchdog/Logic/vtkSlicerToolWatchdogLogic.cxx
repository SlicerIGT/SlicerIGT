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
#include "vtkMRMLLinearTransformNode.h"
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



void vtkSlicerToolWatchdogLogic::UpdateToolState( vtkMRMLToolWatchdogNode* toolWatchdogNode )
{
  if ( toolWatchdogNode == NULL )
  {
    return;
  }

  vtkMRMLLinearTransformNode* toolToRasNode = toolWatchdogNode->GetToolTransformNode();
  if ( toolToRasNode == NULL )
  {
    return;
  }

  //vtkSmartPointer< vtkMatrix4x4 > ToolToRASMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  //toolToRasNode->GetMatrixTransformToWorld( ToolToRASMatrix );
  unsigned long timeStamp = toolToRasNode->GetTransformToWorldMTime();

  if(timeStamp==toolWatchdogNode->GetLastTimeStamp())
  {
    toolWatchdogNode->SetTransformStatus(OUT_OF_DATE);
    vtkWarningMacro("Time stamp is out of date"<<timeStamp);
  }
  else
  {
    toolWatchdogNode->SetTransformStatus(UP_TO_DATE);
    toolWatchdogNode->SetLastTimeStamp(timeStamp);
  }






  //double Origin[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  //double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };

  //ToolToRASMatrix->MultiplyPoint( Origin, P0 );


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


void
vtkSlicerToolWatchdogLogic
::SetObservedTransformNode( vtkMRMLLinearTransformNode* newTransform, vtkMRMLToolWatchdogNode* moduleNode )
{
  if ( newTransform == NULL || moduleNode == NULL )
  {
    vtkWarningMacro( "SetObservedTransformNode: Transform or module node invalid" );
    return;
  }

  moduleNode->SetAndObserveToolTransformNodeId( newTransform->GetID() );
}

void
vtkSlicerToolWatchdogLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
  {
    return;
  }

  vtkMRMLToolWatchdogNode* toolWatchdogNode = vtkMRMLToolWatchdogNode::SafeDownCast( callerNode );
  if ( toolWatchdogNode == NULL )
  {
    return;
  }

  UpdateFromMRMLScene();
  //this->UpdateToolState( toolWatchdogNode );
  //this->UpdateModelColor( toolWatchdogNode );
  //if(PlayWarningSound==true)
  //{
  //  this->PlaySound();
  //}
}

