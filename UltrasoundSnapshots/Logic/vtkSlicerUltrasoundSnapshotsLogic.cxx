 
// UltrasoundSnapshots Logic includes
#include "vtkSlicerUltrasoundSnapshotsLogic.h"

// MRML includes
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerUltrasoundSnapshotsLogic);

//----------------------------------------------------------------------------
vtkSlicerUltrasoundSnapshotsLogic::vtkSlicerUltrasoundSnapshotsLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerUltrasoundSnapshotsLogic::~vtkSlicerUltrasoundSnapshotsLogic()
{
}



void
vtkSlicerUltrasoundSnapshotsLogic
::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}



void
vtkSlicerUltrasoundSnapshotsLogic
::AddSnapshot( vtkMRMLScalarVolumeNode* InputNode )
{
  if ( InputNode == NULL )
  {
    return;
  }
  
  
  vtkSmartPointer< vtkMRMLModelDisplayNode > snapshotDisp = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
  this->GetMRMLScene()->AddNode( snapshotDisp );
  snapshotDisp->SetScene( this->GetMRMLScene() );
  snapshotDisp->SetDisableModifiedEvent( 1 );
  snapshotDisp->SetOpacity( 1.0 );
  snapshotDisp->SetColor( 1.0, 1.0, 1.0 );
  snapshotDisp->SetAmbient( 1.0 );
  snapshotDisp->SetBackfaceCulling( 0 );
  snapshotDisp->SetDiffuse( 0.0 );
  snapshotDisp->SetSaveWithScene( 0 );
  snapshotDisp->SetDisableModifiedEvent( 0 );
  
  std::stringstream nameStream;  // Use stream because later we may add other info in the name, e.g. number.
  nameStream << "Snapshot";
  
  vtkSmartPointer< vtkMRMLModelNode > snapshotModel = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( snapshotModel );
  snapshotModel->SetName( nameStream.str().c_str() );
  snapshotModel->SetDescription( "Live Ultrasound Snapshot" );
  snapshotModel->SetScene( this->GetMRMLScene() );
  snapshotModel->SetAndObserveDisplayNodeID( snapshotDisp->GetID() );
  snapshotModel->SetHideFromEditors( 0 );
  snapshotModel->SetSaveWithScene( 0 );
  
  
  int dims[ 3 ] = { 0, 0, 0 };
  InputNode->GetImageData()->GetDimensions( dims );
  if ( dims[ 0 ] == 0  &&  dims[ 1 ] == 0  && dims[ 2 ] == 0 )
  {
    return;
  }
  
  vtkSmartPointer< vtkPlaneSource > plane = vtkSmartPointer< vtkPlaneSource >::New();
  plane->Update();
  snapshotModel->SetAndObservePolyData( plane->GetOutput() );
  
  vtkPolyData* slicePolyData = snapshotModel->GetPolyData();
  vtkPoints* slicePoints = slicePolyData->GetPoints();
  
  vtkSmartPointer< vtkTransform > ParentTransform = vtkSmartPointer< vtkTransform >::New();
  ParentTransform->Identity();
  if ( InputNode->GetParentTransformNode() != NULL )
  {
    vtkSmartPointer< vtkMatrix4x4 > ParentMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    InputNode->GetParentTransformNode()->GetMatrixTransformToWorld( ParentMatrix );
    ParentTransform->GetMatrix()->DeepCopy( ParentMatrix );
    ParentTransform->Update();
  }
  
  vtkSmartPointer< vtkTransform > InImageTransform = vtkSmartPointer< vtkTransform >::New();
  InImageTransform->Identity();
  InputNode->GetIJKToRASMatrix( InImageTransform->GetMatrix() );
  
  vtkSmartPointer< vtkTransform > tImageToRAS = vtkSmartPointer< vtkTransform >::New();
  tImageToRAS->Identity();
  tImageToRAS->PostMultiply();
  tImageToRAS->Concatenate( ParentTransform );
  tImageToRAS->Concatenate( InImageTransform );
  tImageToRAS->Update();
  
    // Four corners of the image in Image coordinate system.
  
  double point1Image[ 4 ] = { 0.0,       0.0,       0.0, 1.0 };
  double point2Image[ 4 ] = { dims[ 0 ], 0.0,       0.0, 1.0 };
  double point3Image[ 4 ] = { 0.0,       dims[ 1 ], 0.0, 1.0 };
  double point4Image[ 4 ] = { dims[ 0 ], dims[ 1 ], 0.0, 1.0 };
  
    // Four corners of the image in RAS coordinate system.
  
  double point1RAS[ 4 ] = { 0, 0, 0, 0 };
  double point2RAS[ 4 ] = { 0, 0, 0, 0 }; 
  double point3RAS[ 4 ] = { 0, 0, 0, 0 }; 
  double point4RAS[ 4 ] = { 0, 0, 0, 0 };
  
  tImageToRAS->MultiplyPoint( point1Image, point1RAS );
  tImageToRAS->MultiplyPoint( point2Image, point2RAS );
  tImageToRAS->MultiplyPoint( point3Image, point3RAS );
  tImageToRAS->MultiplyPoint( point4Image, point4RAS );
  
    // Position of the PolyData of the new model node.
  
  slicePoints->SetPoint( 0, point1RAS );
  slicePoints->SetPoint( 1, point2RAS );
  slicePoints->SetPoint( 2, point3RAS );
  slicePoints->SetPoint( 3, point4RAS );
  
    // Add image texture.
  
  vtkSmartPointer< vtkImageData > image = vtkSmartPointer< vtkImageData >::New();
  image->DeepCopy( InputNode->GetImageData() );
  snapshotModel->GetModelDisplayNode()->SetAndObserveTextureImageData( image );
}



void
vtkSlicerUltrasoundSnapshotsLogic
::ClearSnapshots()
{
  // TODO: Using this function causes a crash in Slicer that I could debug. So it's not used now.
  
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName( "Snapshot" );
  
  vtkMRMLModelNode* ModelNode = NULL;
  for ( int i = 0; i < collection->GetNumberOfItems(); ++ i )
  {
    ModelNode = vtkMRMLModelNode::SafeDownCast( collection->GetItemAsObject( i ) );
    if ( ModelNode != NULL )
    {
      vtkMRMLDisplayNode* ModelDisplayNode = ModelNode->GetDisplayNode();
      this->GetMRMLScene()->RemoveNode( ModelDisplayNode );
      this->GetMRMLScene()->RemoveNode( ModelNode );
      // ModelDisplayNode->RemoveAllObservers();
      // ModelNode->RemoveAllObservers();
      // ModelDisplayNode->Delete();
      // ModelNode->Delete();
      ModelNode = NULL;
    }
  }
  
  collection->Delete();  
}



//---------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
  
}

//---------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

