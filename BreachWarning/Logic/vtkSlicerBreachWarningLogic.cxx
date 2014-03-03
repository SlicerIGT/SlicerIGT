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
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPCAStatistics.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>

// STD includes
#include <cassert>
#include <sstream>



// Slicer methods 

vtkStandardNewMacro(vtkSlicerBreachWarningLogic);



vtkSlicerBreachWarningLogic::vtkSlicerBreachWarningLogic()
{
  this->ModuleNode = NULL;
}



vtkSlicerBreachWarningLogic::~vtkSlicerBreachWarningLogic()
{
  if ( this->ModuleNode != NULL )
  {
    this->ModuleNode->Delete();
    this->ModuleNode = NULL;
  }
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
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }

  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::New();
  this->GetMRMLScene()->RegisterNodeClass( moduleNode );
  moduleNode->Delete();
}



void
vtkSlicerBreachWarningLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);


}



void
vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}



void
vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
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

  if ( this->ModuleNode == NULL )
  {
    vtkWarningMacro( "Module node not set yet" );
    return;
  }

  this->ModuleNode->SetWatchedModelID( newNodeID );
}



void
vtkSlicerBreachWarningLogic
::SetObservedTransformNode( vtkMRMLNode* newNode )
{
  if ( newNode == NULL )
  {
    this->ModuleNode->SetToolTipTransformID( "" );
    return;
  }

  vtkMRMLLinearTransformNode* ltNode = vtkMRMLLinearTransformNode::SafeDownCast( newNode );
  if ( ltNode == NULL )
  {
    vtkWarningMacro( "Not a transform specified" );
    return;
  }

  this->ModuleNode->SetToolTipTransformID( std::string( ltNode->GetID() ) );
  newNode->AddObserver( vtkMRMLTransformNode::TransformModifiedEvent, (vtkCommand*) this->GetMRMLNodesCallbackCommand() );
}



void
vtkSlicerBreachWarningLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( caller );
  if ( moduleNode != NULL )
  {
    this->ProcessModuleNodeEvents( caller, event, callData );
    return;
  }

  vtkMRMLTransformNode* callerNode = vtkMRMLTransformNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;
  
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID( this->ModuleNode->GetToolTipTransformID() );
  if ( node == NULL ) return; // No tool selected yet.
  
  vtkMRMLLinearTransformNode* toolTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  if ( toolTransformNode == NULL ) return;
  
  if ( this->ModuleNode->GetToolTipTransformID().compare( callerNode->GetID() ) != 0 )
  {
    return; // Not the tool of interest moved.
  }

  if ( this->ModuleNode->GetWatchedModelID().compare( "" ) == 0 ) return;

  vtkMRMLNode* modelMrmlNode = this->GetMRMLScene()->GetNodeByID( this->ModuleNode->GetWatchedModelID() );
  if ( modelMrmlNode == NULL ) return;

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( modelMrmlNode );

  if ( modelNode == NULL )
  {
    vtkWarningMacro( "Model node expected" );
    return;
  }

  // Prepare inside-outside decision.

  vtkPolyData* body = modelNode->GetPolyData();
  if ( body == NULL )
  {
    vtkWarningMacro( "No surface model in node" );
    return;
  }

  vtkSmartPointer< vtkSelectEnclosedPoints > EnclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  EnclosedFilter->Initialize( body );
  
  vtkSmartPointer< vtkMatrix4x4 > ToolToRASMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  toolTransformNode->GetMatrixTransformToWorld( ToolToRASMatrix );
 
  double Origin[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };

  ToolToRASMatrix->MultiplyPoint( Origin, P0 );
  
  int inside = EnclosedFilter->IsInsideSurface( P0[ 0 ], P0[ 1 ], P0[ 2 ] );

  if ( inside )
  {
    modelNode->GetDisplayNode()->SetColor( 255, 0, 0 ); // TODO: Use color by user.
  }
  else
  {
    modelNode->GetDisplayNode()->SetColor( 0, 255, 0 ); // TODO: Use color by user.
  }

}



void
vtkSlicerBreachWarningLogic
::ProcessModuleNodeEvents( vtkObject* caller, unsigned long event, void* callData )
{
  
}



void
vtkSlicerBreachWarningLogic
::ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLScene* callerNode = vtkMRMLScene::SafeDownCast( caller );

  // If the added module node, then observe it.
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