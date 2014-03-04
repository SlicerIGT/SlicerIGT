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
#include "vtkMRMLBreachWarningNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>

// STD includes
#include <cassert>
#include <sstream>



// Slicer methods 

vtkStandardNewMacro(vtkSlicerBreachWarningLogic);



vtkSlicerBreachWarningLogic
::vtkSlicerBreachWarningLogic()
{
  this->BreachWarningNode = NULL;
  for ( int i = 0; i < 3; ++ i )
  {
    this->OriginalColor[ i ] = 0.0;
    this->WarningColor[ i ] = 0.0;
  }
  this->OriginalColor[ 3 ] = 1.0;
  this->WarningColor[ 3 ] = 1.0;
}



vtkSlicerBreachWarningLogic
::~vtkSlicerBreachWarningLogic()
{
  vtkSetAndObserveMRMLNodeMacro( this->BreachWarningNode, NULL );
}



void
vtkSlicerBreachWarningLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void
vtkSlicerBreachWarningLogic
::SetMRMLSceneInternal(vtkMRMLScene * newScene)
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

  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLBreachWarningNode >::New() );
}



void
vtkSlicerBreachWarningLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);

  this->Modified();
}



void
vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  if ( this->GetMRMLScene()->IsBatchProcessing() )
  {
    return;
  }

  if ( node->IsA( "vtkMRMLBreachWarningNode" ) )
  {
    this->Modified();
  }
}



void
vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( this->GetMRMLScene()->IsBatchProcessing() )
  {
    return;
  }

  if ( node->IsA( "vtkMRMLBreachWarningNode" ) )
  {
    this->Modified();
  }
}



void
vtkSlicerBreachWarningLogic
::OnMRMLSceneEndImport()
{
  // If a module node was imported, make that the selected node.
  vtkMRMLBreachWarningNode* moduleNode = NULL;
  vtkMRMLNode* node = this->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLBreachWarningNode" );
  if ( node != NULL )
  {
    moduleNode = vtkMRMLBreachWarningNode::SafeDownCast( node );
    vtkSetAndObserveMRMLNodeMacro( this->BreachWarningNode, moduleNode );
  }
}



void
vtkSlicerBreachWarningLogic
::OnMRMLSceneEndClose()
{
  this->Modified();
}



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

  if ( this->BreachWarningNode == NULL )
  {
    vtkWarningMacro( "vtkSlicerBreachWarningLogic: Module node not set yet" );
    return;
  }

  this->BreachWarningNode->SetWatchedModelNodeID( newNode->GetID() );
}



void
vtkSlicerBreachWarningLogic
::SetObservedTransformNode( vtkMRMLNode* newNode )
{
  if ( newNode == NULL )
  {
    this->BreachWarningNode->SetAndObserveToolTransformNodeId( "" );
    return;
  }

  vtkMRMLLinearTransformNode* ltNode = vtkMRMLLinearTransformNode::SafeDownCast( newNode );
  if ( ltNode == NULL )
  {
    vtkWarningMacro( "Not a transform specified" );
    return;
  }

  this->BreachWarningNode->SetAndObserveToolTransformNodeId( newNode->GetID() );
}



void
vtkSlicerBreachWarningLogic
::SetWarningColor( double red, double green, double blue, double alpha )
{
  this->WarningColor[ 0 ] = red;
  this->WarningColor[ 1 ] = green;
  this->WarningColor[ 2 ] = blue;
  this->WarningColor[ 3 ] = alpha;
}



double
vtkSlicerBreachWarningLogic
::GetWarningColorComponent( int c )
{
  if ( c < 0 || c > 3 )
  {
    vtkErrorMacro( "Invalid color component index" );
    return 0.0;
  }
  return this->WarningColor[ c ];
}



void
vtkSlicerBreachWarningLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;
  
  if ( this->BreachWarningNode == NULL ) return;

  if ( std::string( callerNode->GetID() ).compare( this->BreachWarningNode->GetID() ) == 0 )
  {
    // The module node was modified.
    this->ColorModel( this->BreachWarningNode->GetToolInsideModel() );
  }
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
    // BreachWarningNode->ObserveAllReferenceNodes(); // This will update
    // this->CalculateTransform( BreachWarningNode ); // Will create modified event to update widget
  }

}



void
vtkSlicerBreachWarningLogic
::SetAndObserveBreachWarningNode( vtkMRMLBreachWarningNode* node )
{
  vtkSetAndObserveMRMLNodeMacro( this->BreachWarningNode, node );
}



void
vtkSlicerBreachWarningLogic
::ColorModel( bool inside )
{
  if ( this->ModuleNode == NULL ) return;

  vtkMRMLModelNode* modelNode = this->ModuleNode->GetWatchedModelNode();
  if ( modelNode == NULL ) return;

  if ( inside )
  {
    modelNode->GetDisplayNode()->GetColor( this->OriginalColor );
    modelNode->GetDisplayNode()->SetColor( this->WarningColor );
  }
  else
  {
    modelNode->GetDisplayNode()->SetColor( this->OriginalColor );
  }
}
