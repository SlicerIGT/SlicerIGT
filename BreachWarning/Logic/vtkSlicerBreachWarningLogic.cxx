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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtkImplicitPolyDataDistance.h>

//Qt includes
#include <QDir>

// STD includes
#include <cassert>
#include <sstream>



// Slicer methods 

vtkStandardNewMacro(vtkSlicerBreachWarningLogic);



vtkSlicerBreachWarningLogic
::vtkSlicerBreachWarningLogic()
: LastToolState( UNDEFINED ),
  CurrentToolState( UNDEFINED ),
  BreachSound(NULL),
  PlayWarningSound(1)
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

  if(BreachSound==NULL)
  {
    BreachSound=new QSound( QDir::toNativeSeparators( QString::fromStdString( GetModuleShareDirectory()+"/alarm.wav" ) ) );
  }
}



void
vtkSlicerBreachWarningLogic
::UpdateToolState( vtkMRMLBreachWarningNode* bwNode )
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
  
  vtkSmartPointer< vtkSelectEnclosedPoints > enclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  vtkSmartPointer< vtkImplicitPolyDataDistance > implicitDistanceFilter = vtkSmartPointer< vtkImplicitPolyDataDistance >::New();

  // Transform the body poly data if there is a parent transform.

  vtkMRMLTransformNode* bodyParentTransform = modelNode->GetParentTransformNode();
  if ( bodyParentTransform != NULL )
  {
    vtkSmartPointer< vtkGeneralTransform > bodyToRasTransform = vtkSmartPointer< vtkGeneralTransform >::New();
    bodyParentTransform->GetTransformToWorld( bodyToRasTransform );

    vtkSmartPointer< vtkTransformPolyDataFilter > bodyToRasFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
#if (VTK_MAJOR_VERSION <= 5)
	bodyToRasFilter->SetInput( body );
#else
	bodyToRasFilter->SetInputData( body );
#endif
    bodyToRasFilter->SetTransform( bodyToRasTransform );
    bodyToRasFilter->Update();

    enclosedFilter->Initialize( bodyToRasFilter->GetOutput() ); // expensive: builds a locator
    implicitDistanceFilter->SetInput( bodyToRasFilter->GetOutput() ); // expensive: builds a locator
  }
  else
  {
    enclosedFilter->Initialize( body ); // expensive: builds a locator
    implicitDistanceFilter->SetInput( body ); // expensive: builds a locator
  }

  // Note: Performance can be improved by reusing the same locator in both enclosedFilter and implicitDistanceFilter.
  // EnclosedFilter may not be necessary at all, because the implicitDistanceFilter returns a signed distance (negative if inside)
  // Even more performance improvement can be reached by keeping the filters in memory for all the observed nodes.
  
  vtkSmartPointer< vtkMatrix4x4 > ToolToRASMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  toolToRasNode->GetMatrixTransformToWorld( ToolToRASMatrix );
 
  double Origin[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };

  ToolToRASMatrix->MultiplyPoint( Origin, P0 );
  
  int inside = enclosedFilter->IsInsideSurface( P0[ 0 ], P0[ 1 ], P0[ 2 ] );

  this->LastToolState = this->CurrentToolState;
  
  if ( inside )
  {
    this->CurrentToolState = INSIDE;
  }
  else
  {
    this->CurrentToolState = OUTSIDE;
  }

  bwNode->SetClosestDistanceToModelFromToolTransform(implicitDistanceFilter->EvaluateFunction( P0 ));
  //vtkWarningMacro("Closest Distance to model from transform "<< bwNode->GetClosestDistanceToModelFromToolTransform());
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

  if ( modelNode == NULL )
  {
    return;
  }

 
  if(bwNode->GetDisplayWarningColor()==0)
  {
    double r = bwNode->GetOriginalColorComponent( 0 );
    double g = bwNode->GetOriginalColorComponent( 1 );
    double b = bwNode->GetOriginalColorComponent( 2 );
    modelNode->GetDisplayNode()->SetColor( r, g, b );
    return;
  }

  if ( this->CurrentToolState == INSIDE )
  {
    double r = bwNode->GetWarningColorComponent( 0 );
    double g = bwNode->GetWarningColorComponent( 1 );
    double b = bwNode->GetWarningColorComponent( 2 );
    modelNode->GetDisplayNode()->SetColor( r, g, b );
  }
  
  if ( this->CurrentToolState == OUTSIDE )
  {
    double r = bwNode->GetOriginalColorComponent( 0 );
    double g = bwNode->GetOriginalColorComponent( 1 );
    double b = bwNode->GetOriginalColorComponent( 2 );
    modelNode->GetDisplayNode()->SetColor( r, g, b );
  }
}



void
vtkSlicerBreachWarningLogic
::PlaySound()
{
  if ( this->CurrentToolState == INSIDE )
  {
    if(BreachSound->isFinished())
    {
      BreachSound->setLoops(1);
      BreachSound->play();
    }
  }
  if ( this->CurrentToolState == OUTSIDE )
  {
      BreachSound->stop();
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
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLBreachWarningNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( node, events.GetPointer() );
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
    this->CurrentToolState =UNDEFINED;
    return;
  }

  vtkMRMLModelNode* previousModel;
  double previousOriginalColor[ 3 ];
  if(this->CurrentToolState == INSIDE/*moduleNode->GetToolInsideModel()*/)
  {
    for (int i =0; i<3; i++)
    {
      previousOriginalColor[i]=moduleNode->GetOriginalColorComponent(i);
    }
    previousModel=moduleNode->GetWatchedModelNode();
  }

  if(this->CurrentToolState == OUTSIDE || this->CurrentToolState == UNDEFINED)
  {
    double originalColor[ 3 ];
    newModel->GetDisplayNode()->GetColor( originalColor );
    moduleNode->SetOriginalColor( originalColor[ 0 ], originalColor[ 1 ], originalColor[ 2 ], 1.0 );
  }

  moduleNode->SetAndObserveWatchedModelNodeID( newModel->GetID() );

  if(this->CurrentToolState == INSIDE /*moduleNode->GetToolInsideModel()*/ &&   previousModel!= NULL)
  {
    previousModel->GetDisplayNode()->SetColor(previousOriginalColor[0],previousOriginalColor[1],previousOriginalColor[2]);
  }
}



void
vtkSlicerBreachWarningLogic
::SetObservedTransformNode( vtkMRMLLinearTransformNode* newTransform, vtkMRMLBreachWarningNode* moduleNode )
{
  if ( newTransform == NULL || moduleNode == NULL )
  {
    vtkWarningMacro( "SetObservedTransformNode: Transform or module node invalid" );
    this->CurrentToolState =UNDEFINED;
    return;
  }

  moduleNode->SetAndObserveToolTransformNodeId( newTransform->GetID() );
}



void
vtkSlicerBreachWarningLogic
::SetWarningColor( double red, double green, double blue, double alpha, vtkMRMLBreachWarningNode* moduleNode )
{
  moduleNode->SetWarningColor( red, green, blue, alpha );
  this->UpdateModelColor( moduleNode );
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


double
vtkSlicerBreachWarningLogic
::GetClosestDistanceToModelFromToolTransform( vtkMRMLBreachWarningNode* moduleNode )
{
  return moduleNode->GetClosestDistanceToModelFromToolTransform();
}


void
vtkSlicerBreachWarningLogic
::SetOriginalColor( double red, double green, double blue, double alpha, vtkMRMLBreachWarningNode* moduleNode )
{
  moduleNode->SetOriginalColor( red, green, blue, alpha );
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
::SetDisplayWarningColor(int displayWarningColor, vtkMRMLBreachWarningNode* moduleNode )
{
  moduleNode->SetDisplayWarningColor(displayWarningColor);
  this->UpdateModelColor( moduleNode );
}

int 
vtkSlicerBreachWarningLogic
::GetDisplayWarningColor( vtkMRMLBreachWarningNode* moduleNode )
{
  return moduleNode->GetDisplayWarningColor();
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
  
  if (event==vtkMRMLBreachWarningNode::InputDataModifiedEvent)
  {
    // only recompute output if the input is changed
    // (for example we do not recompute the distance if the computed distance is changed)
    this->UpdateToolState( bwNode );
    this->UpdateModelColor( bwNode );
    if(PlayWarningSound==1)
    {
      this->PlaySound();
    }
  }
}

