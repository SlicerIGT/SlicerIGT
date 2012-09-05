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

// CreateModels Logic includes
#include "vtkSlicerCreateModelsLogic.h"

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"

// VTK includes
#include "vtkAppendPolyData.h"
#include "vtkCubeSource.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include <vtkNew.h>

// STD includes
#include <cassert>



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCreateModelsLogic);



void
vtkSlicerCreateModelsLogic
::LabelPolyData( vtkPolyData* polyData, double value )
{
  vtkSmartPointer< vtkDoubleArray > a = vtkSmartPointer< vtkDoubleArray >::New();
  a->SetNumberOfComponents( 1 );
  a->SetNumberOfTuples( polyData->GetNumberOfPoints() );
  a->SetName( "NeedleColors" );
  for ( int i = 0; i < polyData->GetNumberOfPoints(); ++ i )
  {
    a->SetValue( i, value );
  }
  polyData->GetPointData()->AddArray( a );
}



vtkSmartPointer< vtkPolyData >
vtkSlicerCreateModelsLogic
::CreateCylinder( double height, double radius, double value )
{
  vtkSmartPointer< vtkCylinderSource > s = vtkSmartPointer< vtkCylinderSource >::New();
  s->SetHeight( height );
  s->SetRadius( radius );
  s->SetResolution( 24 );
  s->Update();
  vtkSmartPointer< vtkPolyData > p = s->GetOutput();
  this->LabelPolyData( p, value );
  return p;
}



vtkSmartPointer< vtkPolyData >
vtkSlicerCreateModelsLogic
::CreateCone( double height, double radius, double value )
{
  vtkSmartPointer< vtkConeSource > s = vtkSmartPointer< vtkConeSource >::New();
  s->SetHeight( height );
  s->SetRadius( radius );
  s->SetResolution( 24 );
  s->Update();
  vtkSmartPointer< vtkPolyData > p = s->GetOutput();
  this->LabelPolyData( p, value );
  return p;
}



void
vtkSlicerCreateModelsLogic
::CreateNeedle( double length, double radius )
{
  double tip = radius * 2.0;
  
    // Create a PolyData append object to
  vtkSmartPointer< vtkAppendPolyData > appendPolyData = vtkSmartPointer< vtkAppendPolyData >::New();
  vtkSmartPointer< vtkAppendPolyData > appendShaftTip = vtkSmartPointer< vtkAppendPolyData >::New();
  vtkSmartPointer< vtkAppendPolyData > appendMarkings = vtkSmartPointer< vtkAppendPolyData >::New();
  
    // Create a cylinder to represent the needle shaft
  vtkSmartPointer< vtkPolyData > needleShaft = this->CreateCylinder( ( length - tip ), radius, this->NeedleLabel );
    
    // Set the transform of the needle shaft
  vtkSmartPointer< vtkTransform > needleShaftTransform = vtkSmartPointer< vtkTransform >::New();
  needleShaftTransform->Translate( 0, ( tip + length ) / 2.0, 0 );
    
    // Transform the needle shaft
  vtkSmartPointer< vtkTransformPolyDataFilter > needleShaftTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  needleShaftTransformFilter->SetTransform( needleShaftTransform );
  needleShaftTransformFilter->SetInput( needleShaft );
  needleShaftTransformFilter->Update();
    	
  appendShaftTip->AddInput( needleShaftTransformFilter->GetOutput() );
  
    
    // Create a cone to represent the needle tip
  vtkSmartPointer< vtkPolyData > needleTip = CreateCone( tip, radius, NeedleLabel );
    
    // Set the transform of the needle tip
  vtkSmartPointer< vtkTransform > needleTipTransform = vtkSmartPointer< vtkTransform >::New();
  needleTipTransform->Translate( 0, tip / 2.0, 0 );
  needleTipTransform->RotateZ( -90 );
    
    // Transform the needle tip
  vtkSmartPointer< vtkTransformPolyDataFilter > needleTipTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  needleTipTransformFilter->SetTransform( needleTipTransform );
  needleTipTransformFilter->SetInput( needleTip );
  needleTipTransformFilter->Update();
    
  appendShaftTip->AddInput( needleTipTransformFilter->GetOutput() );
    
  vtkSmartPointer< vtkPolyData > needlePolyData = appendShaftTip->GetOutput();
    
    
    // Iterate, and add needle centrimeter markings
  int cm = 10;
  double markerRadiusOffset = 0.01;
    
  while( cm < length )
  {
      // # Check for a 5 centimeter mark
    int numMark = ( cm / 10 ) % 5;
    double markerHeight = 0.3;
    if ( numMark == 0 )
    {
      markerHeight = 4 * markerHeight;
      numMark = 1;
    }
        
    double firstMark = cm - ( ( numMark - 1 ) * markerHeight );
      
      // Iterate over all markings
    int mark = 0;
    while( mark < numMark )
    {
      mark = mark + 1; //Increase mark count
    
        // Create a cylinder to represent the needle shaft
      vtkSmartPointer< vtkPolyData > needleCM = this->CreateCylinder( markerHeight, ( radius + markerRadiusOffset ), this->MarkerLabel );
        
        // Set the transform of the needle shaft
      vtkSmartPointer< vtkTransform > needleCMTransform = vtkSmartPointer< vtkTransform >::New();
      needleCMTransform->Translate( 0, firstMark + 2 * ( mark - 1 ) * markerHeight, 0 );
        
        // Transform the needle shaft
      vtkSmartPointer< vtkTransformPolyDataFilter > needleCMTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
      needleCMTransformFilter->SetTransform( needleCMTransform );
      needleCMTransformFilter->SetInput( needleCM );
      needleCMTransformFilter->Update();
            
      appendMarkings->AddInput( needleCMTransformFilter->GetOutput() );
    }
      
      // Increment the centimeter marker
    cm = cm + 10;
  }
  
  
  vtkSmartPointer< vtkPolyData > markingPolyData = appendMarkings->GetOutput();
  
    // Assemble the whole needle model
    
  vtkSmartPointer< vtkAppendPolyData > appendAll = vtkSmartPointer< vtkAppendPolyData >::New();
  appendAll->AddInput( needlePolyData );
  appendAll->AddInput( markingPolyData );
  appendAll->Update();
    
    // Add the needle poly data to the scene as a model
    
  vtkSmartPointer< vtkMRMLModelNode > needleModelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  needleModelNode->SetScene( this->GetMRMLScene() );
  needleModelNode->SetName( "NeedleModel" );
  needleModelNode->SetAndObservePolyData( appendAll->GetOutput() );
	
  vtkSmartPointer< vtkMRMLModelDisplayNode > needleDisplayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  needleDisplayNode->SetScene( this->GetMRMLScene() );
  needleDisplayNode->SetName( "NeedleModelDisplay" );
  this->GetMRMLScene()->AddNode( needleDisplayNode );
    
  needleModelNode->SetAndObserveDisplayNodeID( needleDisplayNode->GetID() );
  needleDisplayNode->SetScalarVisibility( 1 );
  needleDisplayNode->SetAmbient( 0.2 );
    
  this->GetMRMLScene()->AddNode( needleModelNode );
    
  needleDisplayNode->SetAndObserveColorNodeID( "vtkMRMLColorTableNodeRainbow" );
  needleDisplayNode->SetActiveScalarName( "NeedleColors" );  
}



void
vtkSlicerCreateModelsLogic
::CreateCube( double x, double y, double z )
{
  vtkSmartPointer< vtkCubeSource > cube = vtkSmartPointer< vtkCubeSource >::New();
  cube->SetXLength( x );
  cube->SetYLength( y );
  cube->SetZLength( z );
  cube->Update();
  
    // Add the needle poly data to the scene as a model
  
  vtkSmartPointer< vtkMRMLModelNode > modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  modelNode->SetScene( this->GetMRMLScene() );
  modelNode->SetName( "CubeModel" );
  modelNode->SetAndObservePolyData( cube->GetOutput() );

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  displayNode->SetScene( this->GetMRMLScene() );
  displayNode->SetName( "CubeModelDisplay" );
  this->GetMRMLScene()->AddNode( displayNode );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  
  this->GetMRMLScene()->AddNode( modelNode );
}



void
vtkSlicerCreateModelsLogic
::CreateCylinder( double h, double r )
{
  vtkSmartPointer< vtkCylinderSource > cube = vtkSmartPointer< vtkCylinderSource >::New();
  cube->SetHeight( h );
  cube->SetRadius( r );
  cube->SetResolution( 24 );
  cube->Update();
  
    // Add the needle poly data to the scene as a model
  
  vtkSmartPointer< vtkMRMLModelNode > modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  modelNode->SetScene( this->GetMRMLScene() );
  modelNode->SetName( "CylinderModel" );
  modelNode->SetAndObservePolyData( cube->GetOutput() );

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  displayNode->SetScene( this->GetMRMLScene() );
  displayNode->SetName( "CylinderModelDisplay" );
  this->GetMRMLScene()->AddNode( displayNode );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  
  this->GetMRMLScene()->AddNode( modelNode );
}



//----------------------------------------------------------------------------
vtkSlicerCreateModelsLogic::vtkSlicerCreateModelsLogic()
{
  this->NeedleLabel = 50.0;
  this->MarkerLabel = 80.0;
}



//----------------------------------------------------------------------------
vtkSlicerCreateModelsLogic::~vtkSlicerCreateModelsLogic()
{
}



//----------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



//---------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

