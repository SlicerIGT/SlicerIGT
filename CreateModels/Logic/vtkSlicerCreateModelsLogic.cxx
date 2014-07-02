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
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkAppendPolyData.h"
#include "vtkCubeSource.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCreateModelsLogic);




void
vtkSlicerCreateModelsLogic
::CreateCylinderData( vtkPolyData* polyData, double height, double radius )
{
  vtkSmartPointer< vtkCylinderSource > s = vtkSmartPointer< vtkCylinderSource >::New();
  s->SetHeight( height );
  s->SetRadius( radius );
  s->SetResolution( 24 );
  s->Update();
  polyData->ShallowCopy(s->GetOutput());
}



void
vtkSlicerCreateModelsLogic
::CreateConeData( vtkPolyData* polyData, double height, double radius )
{
  vtkSmartPointer< vtkConeSource > s = vtkSmartPointer< vtkConeSource >::New();
  s->SetHeight( height );
  s->SetRadius( radius );
  s->SetResolution( 24 );
  s->Update();
  polyData->ShallowCopy( s->GetOutput() );
}



/**
 Creates two MRML models, one for the needle shaft and one for the centimeter marks.
 The two models can be given different colors to show the markings.
*/
void
vtkSlicerCreateModelsLogic
::CreateNeedle( double length, double radius, double tipRadius )
{
  double tip = radius * 2.0;
  
    // Create a PolyData append object to
  vtkSmartPointer< vtkAppendPolyData > appendPolyData = vtkSmartPointer< vtkAppendPolyData >::New();
  vtkSmartPointer< vtkAppendPolyData > appendShaftTip = vtkSmartPointer< vtkAppendPolyData >::New();
  vtkSmartPointer< vtkAppendPolyData > appendMarkings = vtkSmartPointer< vtkAppendPolyData >::New();
  
    // Create a cylinder to represent the needle shaft
  vtkSmartPointer< vtkPolyData > needleShaft = vtkSmartPointer< vtkPolyData >::New();
  this->CreateCylinderData( needleShaft, ( length - tip ), radius );
    
    // Set the transform of the needle shaft
  vtkSmartPointer< vtkTransform > needleShaftTransform = vtkSmartPointer< vtkTransform >::New();
  needleShaftTransform->Translate( 0, ( tip + length ) / 2.0, 0 );
    
    // Transform the needle shaft
  vtkSmartPointer< vtkTransformPolyDataFilter > needleShaftTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  needleShaftTransformFilter->SetTransform( needleShaftTransform );

#if (VTK_MAJOR_VERSION <= 5)
  needleShaftTransformFilter->SetInput( needleShaft );
#else
  needleShaftTransformFilter->SetInputData( needleShaft );
#endif

  needleShaftTransformFilter->Update();
    	
#if (VTK_MAJOR_VERSION <= 5)
  appendShaftTip->AddInput( needleShaftTransformFilter->GetOutput() );
#else
  appendShaftTip->AddInputData( needleShaftTransformFilter->GetOutput() );
#endif
  appendShaftTip->Update();

    // Create a cone to represent the needle tip
  vtkSmartPointer< vtkPolyData > needleTip = vtkSmartPointer< vtkPolyData >::New();
  CreateConeData( needleTip, tip, radius );
  
    // Set the transform of the needle tip
  vtkSmartPointer< vtkTransform > needleTipTransform = vtkSmartPointer< vtkTransform >::New();
  needleTipTransform->Translate( 0, tip / 2.0, 0 );
  needleTipTransform->RotateZ( -90 );
    
    // Transform the needle tip
  vtkSmartPointer< vtkTransformPolyDataFilter > needleTipTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  needleTipTransformFilter->SetTransform( needleTipTransform );
#if (VTK_MAJOR_VERSION <= 5)
  needleTipTransformFilter->SetInput( needleTip );
#else
  needleTipTransformFilter->SetInputData( needleTip );
#endif

  needleTipTransformFilter->Update();

#if (VTK_MAJOR_VERSION <= 5)
  appendShaftTip->AddInput( needleTipTransformFilter->GetOutput() );
#else
  appendShaftTip->AddInputData( needleTipTransformFilter->GetOutput() );
#endif

  appendShaftTip->Update();

  // Sphere for ball tip needle
  if ( tipRadius > 0.0 )
  {
    vtkSmartPointer< vtkSphereSource > needleBallSource = vtkSmartPointer< vtkSphereSource >::New();
    needleBallSource->SetRadius( tipRadius );
    needleBallSource->SetThetaResolution( 24 );
    needleBallSource->SetPhiResolution( 24 );
    needleBallSource->Update();

#if (VTK_MAJOR_VERSION <= 5)
    appendShaftTip->AddInput( needleBallSource->GetOutput() );
#else
    appendShaftTip->AddInputData( needleBallSource->GetOutput() );
#endif

	appendShaftTip->Update();
  }
    
    
  // Add needle centimeter markings
  
  int markPositionYMm = 10; // Start at 1 cm (10 mm).
  
  double markRadiusOffset = 0.01; // So the mark is always outside the shaft.
    
  while( markPositionYMm < length )
  {
    int numberOfLines = ( markPositionYMm / 10 ) % 5; // # Check for a 5 centimeter mark
    double markerHeight = 0.3;
    if ( numberOfLines == 0 )
    {
      markerHeight = 3 * markerHeight;
      numberOfLines = 1;
    }
        
    double firstMarkPositionY = markPositionYMm - ( ( numberOfLines - 1 ) * markerHeight );
      
      // Iterate over all markings
    int line = 0;
    while( line < numberOfLines )
    {
      line = line + 1; // Increase mark count
    
      vtkSmartPointer< vtkPolyData > linePolyData = vtkSmartPointer< vtkPolyData >::New();
      this->CreateCylinderData( linePolyData, markerHeight, ( radius + markRadiusOffset ) );
        
      vtkSmartPointer< vtkTransform > lineTransform = vtkSmartPointer< vtkTransform >::New();
      lineTransform->Translate( 0, firstMarkPositionY + 2 * ( line - 1 ) * markerHeight, 0 );
        
      vtkSmartPointer< vtkTransformPolyDataFilter > lineTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
      lineTransformFilter->SetTransform( lineTransform );
#if (VTK_MAJOR_VERSION <= 5)
      lineTransformFilter->SetInput( linePolyData );
#else
	  lineTransformFilter->SetInputData( linePolyData );
#endif

	    lineTransformFilter->Update();

#if (VTK_MAJOR_VERSION <= 5)
      appendMarkings->AddInput( lineTransformFilter->GetOutput() );
#else
	    appendMarkings->AddInputData( lineTransformFilter->GetOutput() );
#endif

	    appendMarkings->Update();
    }
      
      // Increment the centimeter marker
    markPositionYMm = markPositionYMm + 10;
  }

  // Rotate to the proper needle orientation (X shaft)
  vtkSmartPointer< vtkTransform > xShaftTransform = vtkSmartPointer< vtkTransform >::New();
  xShaftTransform->RotateZ( -90 );

  vtkSmartPointer< vtkTransformPolyDataFilter > xShaftNeedleTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  xShaftNeedleTransformFilter->SetTransform( xShaftTransform );

#if (VTK_MAJOR_VERSION <= 5)
  xShaftNeedleTransformFilter->SetInput( appendShaftTip->GetOutput() );
#else
  xShaftNeedleTransformFilter->SetInputData( appendShaftTip->GetOutput() );
#endif

  xShaftNeedleTransformFilter->Update();
  vtkSmartPointer< vtkPolyData > needlePolyData = xShaftNeedleTransformFilter->GetOutput();

  vtkSmartPointer< vtkTransformPolyDataFilter > xShaftMarkersTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  xShaftMarkersTransformFilter->SetTransform( xShaftTransform );

#if (VTK_MAJOR_VERSION <= 5)
  xShaftMarkersTransformFilter->SetInput( appendMarkings->GetOutput() );
#else
  xShaftMarkersTransformFilter->SetInputData( appendMarkings->GetOutput() );
#endif

  xShaftMarkersTransformFilter->Update();
  vtkSmartPointer< vtkPolyData > needleMarkersPolyData = xShaftMarkersTransformFilter->GetOutput();  
  
  
    // Add the needle poly data to the scene as a model
    
  vtkSmartPointer< vtkMRMLModelNode > needleModelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( needleModelNode );
  needleModelNode->SetName( "NeedleModel" );
  needleModelNode->SetAndObservePolyData( needlePolyData );
	
  vtkSmartPointer< vtkMRMLModelDisplayNode > needleDisplayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( needleDisplayNode );
  needleDisplayNode->SetName( "NeedleModelDisplay" );
  needleDisplayNode->SetColor( 0.0, 1.0, 1.0 );
    
  needleModelNode->SetAndObserveDisplayNodeID( needleDisplayNode->GetID() );
  needleDisplayNode->SetAmbient( 0.2 );
    
  
    // Add the markers model to the scene
  
  vtkSmartPointer< vtkMRMLModelNode > markersModelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( markersModelNode );
  markersModelNode->SetName( "NeedleMarkersModel" );
  markersModelNode->SetAndObservePolyData( needleMarkersPolyData );
  
  vtkSmartPointer< vtkMRMLModelDisplayNode > markersDisplayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( markersDisplayNode );
  markersDisplayNode->SetName( "NeedleMarkersDisplay" );
  markersDisplayNode->SetColor( 0.0, 0.0, 0.0 );
  
  markersModelNode->SetAndObserveDisplayNodeID( markersDisplayNode->GetID() );
  markersDisplayNode->SetAmbient( 0.2 );
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
  this->GetMRMLScene()->AddNode( modelNode );
  modelNode->SetName( "CubeModel" );
  modelNode->SetAndObservePolyData( cube->GetOutput() );

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( displayNode );
  displayNode->SetName( "CubeModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
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
  this->GetMRMLScene()->AddNode( modelNode );
  modelNode->SetName( "CylinderModel" );
  modelNode->SetAndObservePolyData( cube->GetOutput() );

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( displayNode );
  displayNode->SetName( "CylinderModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
}



void
vtkSlicerCreateModelsLogic
::CreateSphere( double radius )
{
  vtkSmartPointer< vtkSphereSource > sphere = vtkSmartPointer< vtkSphereSource >::New();
  sphere->SetRadius( radius );
  sphere->SetThetaResolution( 24 );
  sphere->SetPhiResolution( 12 );
  sphere->Update();
  
  vtkSmartPointer< vtkMRMLModelNode > modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( modelNode );
  modelNode->SetName( "SphereModel" );
  modelNode->SetAndObservePolyData( sphere->GetOutput() );
  
  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( displayNode );
  displayNode->SetName( "SphereModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
}



void
vtkSlicerCreateModelsLogic
::CreateCoordinate( double axisLength, double axisDiameter )
{
  vtkSmartPointer< vtkAppendPolyData > appendPolyData = vtkSmartPointer< vtkAppendPolyData >::New();
  
  // X axis
  
  vtkSmartPointer< vtkCylinderSource > XCylinderSource = vtkSmartPointer< vtkCylinderSource >::New();
  XCylinderSource->SetRadius( axisDiameter / 2.0 );
  XCylinderSource->SetHeight( axisLength );
  XCylinderSource->Update();

  vtkSmartPointer< vtkTransform > XCylinderTransform = vtkSmartPointer< vtkTransform >::New();
  XCylinderTransform->RotateZ( -90.0 );
  XCylinderTransform->Translate( 0.0, axisLength / 2.0, 0.0 );
  XCylinderTransform->Update();
  
  vtkSmartPointer< vtkTransformPolyDataFilter > XCylinderTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  
#if (VTK_MAJOR_VERSION <= 5)
  XCylinderTransformFilter->SetInput( XCylinderSource->GetOutput() );
#else
  XCylinderTransformFilter->SetInputData( XCylinderSource->GetOutput() );
#endif

  XCylinderTransformFilter->SetTransform( XCylinderTransform );
  XCylinderTransformFilter->Update();

    
#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( XCylinderTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( XCylinderTransformFilter->GetOutput() );
#endif

  
  appendPolyData->Update();

  vtkSmartPointer< vtkConeSource > XTipSource = vtkSmartPointer< vtkConeSource >::New();
  XTipSource->SetRadius( axisDiameter * 1.5 );
  XTipSource->SetHeight( axisLength / 4.0 );
  XTipSource->Update();
  
  vtkSmartPointer< vtkTransform > XTipTransform = vtkSmartPointer< vtkTransform >::New();
  XTipTransform->Translate( axisLength + axisLength * 0.1, 0.0, 0.0 );
  XTipTransform->Update();
  
  vtkSmartPointer< vtkTransformPolyDataFilter > XTipTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  
#if (VTK_MAJOR_VERSION <= 5)
  XTipTransformFilter->SetInput( XTipSource->GetOutput() );
#else
  XTipTransformFilter->SetInputData( XTipSource->GetOutput() );
#endif

  XTipTransformFilter->SetTransform( XTipTransform );
  XTipTransformFilter->Update();
  
#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( XTipTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( XTipTransformFilter->GetOutput() );
#endif
  
  
  // Y axis
  
  vtkSmartPointer< vtkCylinderSource > YCylinderSource = vtkSmartPointer< vtkCylinderSource >::New();
  YCylinderSource->SetRadius( axisDiameter / 2.0 );
  YCylinderSource->SetHeight( axisLength );
  YCylinderSource->Update();
  
  vtkSmartPointer< vtkTransform > YCylinderTransform = vtkSmartPointer< vtkTransform >::New();
  YCylinderTransform->Translate( 0.0, axisLength / 2.0, 0.0 );
  YCylinderTransform->Update();
  
  vtkSmartPointer< vtkTransformPolyDataFilter > YCylinderTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  
#if (VTK_MAJOR_VERSION <= 5)
  YCylinderTransformFilter->SetInput( YCylinderSource->GetOutput() );
#else
  YCylinderTransformFilter->SetInputData( YCylinderSource->GetOutput() );
#endif

  YCylinderTransformFilter->SetTransform( YCylinderTransform );
  YCylinderTransformFilter->Update();

#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( YCylinderTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( YCylinderTransformFilter->GetOutput() );
#endif

  appendPolyData->Update();
  
  // Z axis
  
  vtkSmartPointer< vtkCylinderSource > ZCylinderSource = vtkSmartPointer< vtkCylinderSource >::New();
  ZCylinderSource->SetRadius( axisDiameter / 2.0 );
  ZCylinderSource->SetHeight( axisLength );
  ZCylinderSource->Update();
  
  vtkSmartPointer< vtkTransform > ZCylinderTransform = vtkSmartPointer< vtkTransform >::New();
  ZCylinderTransform->RotateX( 90.0 );
  ZCylinderTransform->Translate( 0.0, axisLength / 2.0, 0.0 );
  ZCylinderTransform->Update();
  
  vtkSmartPointer< vtkTransformPolyDataFilter > ZCylinderTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  
#if (VTK_MAJOR_VERSION <= 5)
  ZCylinderTransformFilter->SetInput( ZCylinderSource->GetOutput() );
#else
  ZCylinderTransformFilter->SetInputData( ZCylinderSource->GetOutput() );
#endif

  ZCylinderTransformFilter->SetTransform( ZCylinderTransform );
  ZCylinderTransformFilter->Update();

  
#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( ZCylinderTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( ZCylinderTransformFilter->GetOutput() );
#endif

  appendPolyData->Update();
  
  
  // Model node
  
  vtkSmartPointer< vtkMRMLModelNode > modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( modelNode );
  
  modelNode->SetName( "CoordinateModel" );
  modelNode->SetAndObservePolyData( appendPolyData->GetOutput() );
  
  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( displayNode );
  
  displayNode->SetName( "CoordinateModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
}



vtkSlicerCreateModelsLogic::vtkSlicerCreateModelsLogic()
{
  
}




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