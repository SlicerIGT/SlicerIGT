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
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCreateModelsLogic);

//----------------------------------------------------------------------------
vtkSlicerCreateModelsLogic::vtkSlicerCreateModelsLogic()
{
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

//----------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic::CreateCylinderData( vtkPolyData* polyData, double height, double radius )
{
  vtkNew<vtkCylinderSource> s;
  s->SetHeight( height );
  s->SetRadius( radius );
  s->SetResolution( 24 );
  s->Update();
  polyData->ShallowCopy(s->GetOutput());
}

//----------------------------------------------------------------------------
void vtkSlicerCreateModelsLogic::CreateConeData( vtkPolyData* polyData, double height, double radius )
{
  vtkNew<vtkConeSource> s;
  s->SetHeight( height );
  s->SetRadius( radius );
  s->SetResolution( 24 );
  s->Update();
  polyData->ShallowCopy( s->GetOutput() );
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerCreateModelsLogic::CreateNeedle( double length, double radius, double tipRadius, bool markers )
{
  double tip = radius * 2.0;
  
    // Create a PolyData append object to
  vtkNew<vtkAppendPolyData> appendPolyData;
  vtkNew<vtkAppendPolyData> appendShaftTip;
  vtkNew<vtkAppendPolyData> appendMarkings;
  
    // Create a cylinder to represent the needle shaft
  vtkNew<vtkPolyData> needleShaft;
  this->CreateCylinderData( needleShaft.GetPointer(), ( length - tip ), radius );
    
    // Set the transform of the needle shaft
  vtkNew<vtkTransform> needleShaftTransform;
  needleShaftTransform->Translate( 0, ( tip + length ) / 2.0, 0 );
    
    // Transform the needle shaft
  vtkNew<vtkTransformPolyDataFilter> needleShaftTransformFilter;
  needleShaftTransformFilter->SetTransform( needleShaftTransform.GetPointer() );

#if (VTK_MAJOR_VERSION <= 5)
  needleShaftTransformFilter->SetInput( needleShaft );
#else
  needleShaftTransformFilter->SetInputData( needleShaft.GetPointer() );
#endif

  needleShaftTransformFilter->Update();
    
#if (VTK_MAJOR_VERSION <= 5)
  appendShaftTip->AddInput( needleShaftTransformFilter->GetOutput() );
#else
  appendShaftTip->AddInputData( needleShaftTransformFilter->GetOutput() );
#endif
  appendShaftTip->Update();

    // Create a cone to represent the needle tip
  vtkNew<vtkPolyData> needleTip;
  CreateConeData( needleTip.GetPointer(), tip, radius );
  
    // Set the transform of the needle tip
  vtkNew<vtkTransform> needleTipTransform;
  needleTipTransform->Translate( 0, tip / 2.0, 0 );
  needleTipTransform->RotateZ( -90 );
    
    // Transform the needle tip
  vtkNew<vtkTransformPolyDataFilter> needleTipTransformFilter;
  needleTipTransformFilter->SetTransform( needleTipTransform.GetPointer() );
#if (VTK_MAJOR_VERSION <= 5)
  needleTipTransformFilter->SetInput( needleTip );
#else
  needleTipTransformFilter->SetInputData( needleTip.GetPointer() );
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
    vtkNew<vtkSphereSource> needleBallSource;
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
    
      vtkNew<vtkPolyData> linePolyData;
      this->CreateCylinderData( linePolyData.GetPointer(), markerHeight, ( radius + markRadiusOffset ) );
        
      vtkNew<vtkTransform> lineTransform;
      lineTransform->Translate( 0, firstMarkPositionY + 2 * ( line - 1 ) * markerHeight, 0 );
        
      vtkNew<vtkTransformPolyDataFilter> lineTransformFilter;
      lineTransformFilter->SetTransform( lineTransform.GetPointer() );
#if (VTK_MAJOR_VERSION <= 5)
      lineTransformFilter->SetInput( linePolyData );
#else
	  lineTransformFilter->SetInputData( linePolyData.GetPointer() );
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

  // Rotate to the proper needle orientation (Z shaft)
  vtkNew<vtkTransform> zShaftTransform;
  zShaftTransform->RotateX( -90 );

  vtkNew<vtkTransformPolyDataFilter> zShaftNeedleTransformFilter;
  zShaftNeedleTransformFilter->SetTransform( zShaftTransform.GetPointer() );

#if (VTK_MAJOR_VERSION <= 5)
  zShaftNeedleTransformFilter->SetInput( appendShaftTip->GetOutput() );
#else
  zShaftNeedleTransformFilter->SetInputData( appendShaftTip->GetOutput() );
#endif

  zShaftNeedleTransformFilter->Update();
  vtkPolyData* needlePolyData = zShaftNeedleTransformFilter->GetOutput();

  vtkNew<vtkTransformPolyDataFilter> zShaftMarkersTransformFilter;
  zShaftMarkersTransformFilter->SetTransform( zShaftTransform.GetPointer() );

#if (VTK_MAJOR_VERSION <= 5)
  zShaftMarkersTransformFilter->SetInput( appendMarkings->GetOutput() );
#else
  zShaftMarkersTransformFilter->SetInputData( appendMarkings->GetOutput() );
#endif

  zShaftMarkersTransformFilter->Update();
  vtkPolyData* needleMarkersPolyData = zShaftMarkersTransformFilter->GetOutput();  
  
  vtkNew<vtkAppendPolyData> finalAppend;
#if (VTK_MAJOR_VERSION <= 5)
  finalAppend->AddInput( needlePolyData );
#else
  finalAppend->AddInputData( needlePolyData );
#endif  

  const char colorScalarName[]="Color";
  if ( markers )
  {
    int needleColorIndex=4;
    int needleMarkersColorIndex=12;

    vtkNew<vtkIntArray> colorArrayNeedle;
    colorArrayNeedle->SetName(colorScalarName);
    colorArrayNeedle->SetNumberOfComponents(1);
    colorArrayNeedle->SetNumberOfTuples(needlePolyData->GetNumberOfPoints());
    colorArrayNeedle->FillComponent(0, needleColorIndex);
    needlePolyData->GetPointData()->SetScalars(colorArrayNeedle.GetPointer());

    vtkNew<vtkIntArray> colorArrayNeedleMarkers;
    colorArrayNeedleMarkers->SetName(colorScalarName);
    colorArrayNeedleMarkers->SetNumberOfComponents(1);
    colorArrayNeedleMarkers->SetNumberOfTuples(needleMarkersPolyData->GetNumberOfPoints());
    colorArrayNeedleMarkers->FillComponent(0, needleMarkersColorIndex);
    needleMarkersPolyData->GetPointData()->SetScalars(colorArrayNeedleMarkers.GetPointer());

#if (VTK_MAJOR_VERSION <= 5)
    finalAppend->AddInput( needleMarkersPolyData );
#else
	  finalAppend->AddInputData( needleMarkersPolyData );
#endif
  }
   
  // Add the needle poly data to the scene as a model

  vtkNew<vtkMRMLModelNode> needleModelNode;
  this->GetMRMLScene()->AddNode( needleModelNode.GetPointer() );
  needleModelNode->SetName( "NeedleModel" );
  //needleModelNode->SetAndObservePolyData( needlePolyData.GetPointer() );
  finalAppend->Update();
  needleModelNode->SetAndObservePolyData( finalAppend->GetOutput() );
	
  vtkNew< vtkMRMLModelDisplayNode > needleDisplayNode;
  this->GetMRMLScene()->AddNode( needleDisplayNode.GetPointer() );
  needleDisplayNode->SetName( "NeedleModelDisplay" );
  needleDisplayNode->SetColor( 0.0, 1.0, 1.0 );
    
  needleModelNode->SetAndObserveDisplayNodeID( needleDisplayNode->GetID() );
  needleDisplayNode->SetAmbient( 0.2 );
    
  if (markers)
  {
    needleDisplayNode->SetActiveScalarName(colorScalarName);
    needleDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFileGenericColors.txt");
    needleDisplayNode->SetScalarVisibility(1);
    needleDisplayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseColorNodeScalarRange);
    needleDisplayNode->SetAutoScalarRange(0);
  }

  // Add the markers model to the scene
/*  
  if ( markers )
  {
    vtkNew< vtkMRMLModelNode > markersModelNode;
    this->GetMRMLScene()->AddNode( markersModelNode.GetPointer() );
    markersModelNode->SetName( "NeedleMarkersModel" );
    markersModelNode->SetAndObservePolyData( needleMarkersPolyData );
    
    vtkNew< vtkMRMLModelDisplayNode > markersDisplayNode;
    this->GetMRMLScene()->AddNode( markersDisplayNode.GetPointer() );
    markersDisplayNode->SetName( "NeedleMarkersDisplay" );
    markersDisplayNode->SetColor( 0.0, 0.0, 0.0 );
    
    markersModelNode->SetAndObserveDisplayNodeID( markersDisplayNode->GetID() );
    markersDisplayNode->SetAmbient( 0.2 );
  }
  */
  return needleModelNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerCreateModelsLogic::CreateCube( double x, double y, double z )
{
  vtkNew< vtkCubeSource > cube;
  cube->SetXLength( x );
  cube->SetYLength( y );
  cube->SetZLength( z );
  cube->Update();
  
    // Add the needle poly data to the scene as a model
  
  vtkNew< vtkMRMLModelNode > modelNode;
  this->GetMRMLScene()->AddNode( modelNode.GetPointer() );
  modelNode->SetName( "CubeModel" );
  modelNode->SetAndObservePolyData( cube->GetOutput() );

  vtkNew< vtkMRMLModelDisplayNode > displayNode;
  this->GetMRMLScene()->AddNode( displayNode.GetPointer() );
  displayNode->SetName( "CubeModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );

  return modelNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerCreateModelsLogic::CreateCylinder( double h, double r )
{
  vtkNew< vtkCylinderSource > cylinder;
  cylinder->SetHeight( h );
  cylinder->SetRadius( r );
  cylinder->SetResolution( 24 );
  
  // Set the transform of the needle tip
  vtkNew< vtkTransform > rotateToLongAxisZ;
  rotateToLongAxisZ->RotateX( -90 );
    
    // Transform the needle tip
  vtkNew< vtkTransformPolyDataFilter > transformFilter;
  transformFilter->SetTransform( rotateToLongAxisZ.GetPointer() );
#if (VTK_MAJOR_VERSION <= 5)
  cylinder->Update();
  transformFilter->SetInput( cylinder->GetOutput() );
#else
  transformFilter->SetInputConnection( cylinder->GetOutputPort() );
#endif

  // Add the needle poly data to the scene as a model
  
  vtkNew< vtkMRMLModelNode > modelNode;
  this->GetMRMLScene()->AddNode( modelNode.GetPointer() );
  modelNode->SetName( "CylinderModel" );
  transformFilter->Update();
  modelNode->SetAndObservePolyData( transformFilter->GetOutput() );

  vtkNew< vtkMRMLModelDisplayNode > displayNode;
  this->GetMRMLScene()->AddNode( displayNode.GetPointer() );
  displayNode->SetName( "CylinderModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );

  return modelNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerCreateModelsLogic::CreateSphere( double radius )
{
  vtkNew< vtkSphereSource > sphere;
  sphere->SetRadius( radius );
  sphere->SetThetaResolution( 24 );
  sphere->SetPhiResolution( 12 );
  sphere->Update();
  
  vtkNew< vtkMRMLModelNode > modelNode;
  this->GetMRMLScene()->AddNode( modelNode.GetPointer() );
  modelNode->SetName( "SphereModel" );
  modelNode->SetAndObservePolyData( sphere->GetOutput() );
  
  vtkNew< vtkMRMLModelDisplayNode > displayNode;
  this->GetMRMLScene()->AddNode( displayNode.GetPointer() );
  displayNode->SetName( "SphereModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );

  return modelNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerCreateModelsLogic::CreateCoordinate( double axisLength, double axisDiameter )
{
  vtkNew< vtkAppendPolyData > appendPolyData;
  
  // X axis
  
  vtkNew< vtkCylinderSource > xCylinderSource;
  xCylinderSource->SetRadius( axisDiameter / 2.0 );
  xCylinderSource->SetHeight( axisLength );
  xCylinderSource->Update();

  vtkNew< vtkTransform > xCylinderTransform;
  xCylinderTransform->RotateZ( -90.0 );
  xCylinderTransform->Translate( 0.0, axisLength / 2.0, 0.0 );
  xCylinderTransform->Update();
  
  vtkNew< vtkTransformPolyDataFilter > xCylinderTransformFilter;
  
#if (VTK_MAJOR_VERSION <= 5)
  xCylinderTransformFilter->SetInput( xCylinderSource->GetOutput() );
#else
  xCylinderTransformFilter->SetInputData( xCylinderSource->GetOutput() );
#endif

  xCylinderTransformFilter->SetTransform( xCylinderTransform.GetPointer() );
  xCylinderTransformFilter->Update();

    
#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( xCylinderTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( xCylinderTransformFilter->GetOutput() );
#endif

  
  appendPolyData->Update();

  vtkNew< vtkConeSource > XTipSource;
  XTipSource->SetRadius( axisDiameter * 1.5 );
  XTipSource->SetHeight( axisLength / 4.0 );
  XTipSource->Update();
  
  vtkNew< vtkTransform > XTipTransform;
  XTipTransform->Translate( axisLength + axisLength * 0.1, 0.0, 0.0 );
  XTipTransform->Update();
  
  vtkNew< vtkTransformPolyDataFilter > XTipTransformFilter;
  
#if (VTK_MAJOR_VERSION <= 5)
  XTipTransformFilter->SetInput( XTipSource->GetOutput() );
#else
  XTipTransformFilter->SetInputData( XTipSource->GetOutput() );
#endif

  XTipTransformFilter->SetTransform( XTipTransform.GetPointer() );
  XTipTransformFilter->Update();
  
#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( XTipTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( XTipTransformFilter->GetOutput() );
#endif
  
  
  // Y axis
  
  vtkNew< vtkCylinderSource > yCylinderSource;
  yCylinderSource->SetRadius( axisDiameter / 2.0 );
  yCylinderSource->SetHeight( axisLength );
  yCylinderSource->Update();
  
  vtkNew< vtkTransform > yCylinderTransform;
  yCylinderTransform->Translate( 0.0, axisLength / 2.0, 0.0 );
  yCylinderTransform->Update();
  
  vtkNew< vtkTransformPolyDataFilter > yCylinderTransformFilter;
  
#if (VTK_MAJOR_VERSION <= 5)
  yCylinderTransformFilter->SetInput( yCylinderSource->GetOutput() );
#else
  yCylinderTransformFilter->SetInputData( yCylinderSource->GetOutput() );
#endif

  yCylinderTransformFilter->SetTransform( yCylinderTransform.GetPointer() );
  yCylinderTransformFilter->Update();

#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( yCylinderTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( yCylinderTransformFilter->GetOutput() );
#endif

  appendPolyData->Update();
  
  // Z axis
  
  vtkNew< vtkCylinderSource > zCylinderSource;
  zCylinderSource->SetRadius( axisDiameter / 2.0 );
  zCylinderSource->SetHeight( axisLength );
  zCylinderSource->Update();
  
  vtkNew< vtkTransform > zCylinderTransform;
  zCylinderTransform->RotateX( 90.0 );
  zCylinderTransform->Translate( 0.0, axisLength / 2.0, 0.0 );
  zCylinderTransform->Update();
  
  vtkNew< vtkTransformPolyDataFilter > zCylinderTransformFilter;
  
#if (VTK_MAJOR_VERSION <= 5)
  zCylinderTransformFilter->SetInput( zCylinderSource->GetOutput() );
#else
  zCylinderTransformFilter->SetInputData( zCylinderSource->GetOutput() );
#endif

  zCylinderTransformFilter->SetTransform( zCylinderTransform.GetPointer() );
  zCylinderTransformFilter->Update();
  
#if (VTK_MAJOR_VERSION <= 5)
  appendPolyData->AddInput( zCylinderTransformFilter->GetOutput() );
#else
  appendPolyData->AddInputData( zCylinderTransformFilter->GetOutput() );
#endif

  appendPolyData->Update();
  
  // Model node
  
  vtkNew< vtkMRMLModelNode > modelNode;
  this->GetMRMLScene()->AddNode( modelNode.GetPointer() );
  
  modelNode->SetName( "CoordinateModel" );
  modelNode->SetAndObservePolyData( appendPolyData->GetOutput() );
  
  vtkNew< vtkMRMLModelDisplayNode > displayNode;
  this->GetMRMLScene()->AddNode( displayNode.GetPointer() );
  
  displayNode->SetName( "CoordinateModelDisplay" );
  
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );

  return modelNode.GetPointer();
}
