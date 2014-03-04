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
#include <vtkSelectEnclosedPoints.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
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
}



vtkSlicerBreachWarningLogic
::~vtkSlicerBreachWarningLogic()
{
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
vtkSlicerBreachWarningLogic
::UpdateModelColor( vtkMRMLBreachWarningNode* bwNode )
{
  if ( bwNode == NULL )
  {
    return;
  }

  vtkMRMLModelNode* modelNode = bwNode->GetWatchedModelNode();
  vtkMRMLLinearTransformNode* toolToRasNode = bwNode->GetToolTransformNode();

  if ( modelNode == NULL || toolToRasNode == NULL )
  {
    return;
  }

  vtkPolyData* body = modelNode->GetPolyData();
  if ( body == NULL )
  {
    vtkWarningMacro( "No surface model in node" );
    return;
  }


  vtkSmartPointer< vtkSelectEnclosedPoints > EnclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  EnclosedFilter->Initialize( body );
  
  vtkSmartPointer< vtkMatrix4x4 > ToolToRASMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  toolToRasNode->GetMatrixTransformToWorld( ToolToRASMatrix );
 
  double Origin[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };

  ToolToRASMatrix->MultiplyPoint( Origin, P0 );
  
  int inside = EnclosedFilter->IsInsideSurface( P0[ 0 ], P0[ 1 ], P0[ 2 ] );

  if ( inside )
  {
    double r = bwNode->GetWarningColorComponent( 0 );
    double g = bwNode->GetWarningColorComponent( 1 );
    double b = bwNode->GetWarningColorComponent( 2 );
    modelNode->GetDisplayNode()->SetColor( r, g, b );
  }
  else
  {
    double r = bwNode->GetOriginalColorComponent( 0 );
    double g = bwNode->GetOriginalColorComponent( 1 );
    double b = bwNode->GetOriginalColorComponent( 2 );
    modelNode->GetDisplayNode()->SetColor( r, g, b );
  }
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

  if ( node->IsA( "vtkMRMLBreachWarningNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( node ); // Remove previous observers.
    vtkObserveMRMLNodeMacro( node );
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

  if ( node->IsA( "vtkMRMLBreachWarningNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}



void
vtkSlicerBreachWarningLogic
::SetWatchedModelNode( vtkMRMLModelNode* newModel, vtkMRMLBreachWarningNode* moduleNode )
{
  if ( newModel == NULL || moduleNode == NULL )
  {
    vtkWarningMacro( "SetWatchedModelNode: Model or module node not specified" );
    return;
  }
  
  double originalColor[ 3 ];
  newModel->GetDisplayNode()->GetColor( originalColor );
  moduleNode->SetOriginalColor( originalColor[ 0 ], originalColor[ 1 ], originalColor[ 2 ], 1.0 );

  moduleNode->SetWatchedModelNodeID( newModel->GetID() );
}



void
vtkSlicerBreachWarningLogic
::SetObservedTransformNode( vtkMRMLLinearTransformNode* newTransform, vtkMRMLBreachWarningNode* moduleNode )
{
  if ( newTransform == NULL || moduleNode == NULL )
  {
    vtkWarningMacro( "SetObservedTransformNode: Transform or module node invalid" );
    return;
  }

  moduleNode->SetAndObserveToolTransformNodeId( newTransform->GetID() );
}



void
vtkSlicerBreachWarningLogic
::SetWarningColor( double red, double green, double blue, double alpha, vtkMRMLBreachWarningNode* moduleNode )
{
  moduleNode->SetWarningColor( red, green, blue, alpha );
}



double
vtkSlicerBreachWarningLogic
::GetWarningColorComponent( int c, vtkMRMLBreachWarningNode* moduleNode )
{
  if ( c < 0 || c > 3 )
  {
    vtkErrorMacro( "Invalid color component index" );
    return 0.0;
  }
  return moduleNode->GetWarningColorComponent( c );
}



void
vtkSlicerBreachWarningLogic
::SetOriginalColor( double red, double green, double blue, double alpha, vtkMRMLBreachWarningNode* moduleNode )
{
  ModuleNode->SetOriginalColor( red, green, blue, alpha );
}



double
vtkSlicerBreachWarningLogic
::GetOriginalColorComponent( int c, vtkMRMLBreachWarningNode* moduleNode )
{
  if ( c < 0 || c > 3 )
  {
    vtkErrorMacro( "Invalid color component index" );
    return 0.0;
  }
  return moduleNode->GetOriginalColorComponent( c );
}



void
vtkSlicerBreachWarningLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
  {
    return;
  }
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( callerNode );
  if ( bwNode == NULL )
  {
    return;
  }

  this->UpdateModelColor( bwNode );
}

