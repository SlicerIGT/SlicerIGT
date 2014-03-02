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

// BreachWarning includes
#include "vtkSlicerBreachWarningLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkPCAStatistics.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>
#include <sstream>


// Helper methods -------------------------------------------------------------------

double EIGENVALUE_THRESHOLD = 1e-4;

vtkPoints* MarkupsFiducialNodeToVTKPoints( vtkMRMLMarkupsFiducialNode* markupsFiducialNode )
{
  vtkPoints* points = vtkPoints::New();
  for ( int i = 0; i < markupsFiducialNode->GetNumberOfFiducials(); i++ )
  {
    double currentFiducial[ 3 ] = { 0, 0, 0 };
    markupsFiducialNode->GetNthFiducialPosition( i, currentFiducial );
    points->InsertNextPoint( currentFiducial );
  }

  return points;
}


// Slicer methods -------------------------------------------------------------------

vtkStandardNewMacro(vtkSlicerBreachWarningLogic);



vtkSlicerBreachWarningLogic::vtkSlicerBreachWarningLogic()
{
}



vtkSlicerBreachWarningLogic::~vtkSlicerBreachWarningLogic()
{
}



void vtkSlicerBreachWarningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


void vtkSlicerBreachWarningLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerBreachWarningLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::New();
  this->GetMRMLScene()->RegisterNodeClass( moduleNode );
  moduleNode->Delete();
}



void vtkSlicerBreachWarningLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}



void vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

// Variable getters and setters -----------------------------------------------------
// Note: vtkSetMacro doesn't call a modified event if the replacing value is the same as before
// We want a modified event always
std::string vtkSlicerBreachWarningLogic
::GetOutputMessage( std::string nodeID )
{
  return this->OutputMessages[ nodeID ];
}


void vtkSlicerBreachWarningLogic
::SetOutputMessage( std::string nodeID, std::string newOutputMessage )
{
  this->OutputMessages[ nodeID ] = newOutputMessage;
  this->Modified();
}



// Module-specific methods ----------------------------------------------------------



void
vtkSlicerBreachWarningLogic
::SetModelNodeID( std::string newNodeID )
{
  vtkMRMLNode* newNode = this->GetMRMLScene()->GetNodeByID( newNodeID );
  if ( newNode == NULL )
  {
    vtkWarningMacro( "MRML node ID not found" );
    return;
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( newNode );
  if ( modelNode == NULL )
  {
    vtkWarningMacro( "Unexpected MRML node type" );
    return;
  }

  
}



void
vtkSlicerBreachWarningLogic
::SetObservedTransformNode( vtkMRMLNode* newNode )
{

}



// Node update methods ----------------------------------------------------------

void vtkSlicerBreachWarningLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLBreachWarningNode* callerNode = vtkMRMLBreachWarningNode::SafeDownCast( caller );
  // The caller must be a vtkMRMLBreachWarningNode
  if ( callerNode != NULL )
  {
    // Will create modified event to update widget
  }
}


void vtkSlicerBreachWarningLogic
::ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLScene* callerNode = vtkMRMLScene::SafeDownCast( caller );

  // If the added node was a fiducial registration wizard node then observe it
  vtkMRMLNode* addedNode = reinterpret_cast< vtkMRMLNode* >( callData );
  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && BreachWarningNode != NULL )
  {
    // This will get called exactly once, and we will add the observer only once (since node is never replaced)
    BreachWarningNode->AddObserver( vtkCommand::ModifiedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
    BreachWarningNode->UpdateScene( this->GetMRMLScene() );
    BreachWarningNode->ObserveAllReferenceNodes(); // This will update
    // this->CalculateTransform( BreachWarningNode ); // Will create modified event to update widget
  }

}