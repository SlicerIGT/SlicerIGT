
// UltrasoundSnapshots Logic includes
#include "vtkSlicerUltrasoundSnapshotsLogic.h"

// MRML includes
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerUltrasoundSnapshotsLogic);

//----------------------------------------------------------------------------
vtkSlicerUltrasoundSnapshotsLogic::vtkSlicerUltrasoundSnapshotsLogic()
{
  this->snapshotCounter = 1;
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
::SetInputVolumeNode( vtkMRMLScalarVolumeNode* InputNode )
{
  vtkSmartPointer< vtkCollection > volumeNodes = vtkSmartPointer< vtkCollection >::Take( this->GetMRMLScene()->GetNodesByClass( "vtkMRMLScalarVolumeNode" ) );
  vtkNew< vtkCollectionIterator > volumeIt;
  volumeIt->SetCollection( volumeNodes );
  
  for ( volumeIt->InitTraversal(); ! volumeIt->IsDoneWithTraversal(); volumeIt->GoToNextItem() )
  {
    vtkMRMLScalarVolumeNode* volume = vtkMRMLScalarVolumeNode::SafeDownCast( volumeIt->GetCurrentObject() );
    
    if ( volume == NULL )
    {
      continue;
    }
    
    if ( volume->GetAttribute( "UltrasoundSnapshotsInput" )
         && std::string( volume->GetAttribute( "UltrasoundSnapshotsInput" ) ).compare( "true" ) == 0 )
    {
      if ( std::string( volume->GetID() ).compare( InputNode->GetID() ) != 0 )
      {
        volume->SetAttribute( "UltrasoundSnapshotsInput", NULL );
      }
    }
    else
    {
      if ( std::string( volume->GetID() ).compare( InputNode->GetID() ) == 0 )
      {
        volume->SetAttribute( "UltrasoundSnapshotsInput", "true" );
      }
    }
  }
}



vtkMRMLScalarVolumeNode*
vtkSlicerUltrasoundSnapshotsLogic
::GetInputVolumeNode()
{
  vtkSmartPointer< vtkCollection > volumeNodes = vtkSmartPointer< vtkCollection >::Take( this->GetMRMLScene()->GetNodesByClass( "vtkMRMLScalarVolumeNode" ) );
  vtkNew< vtkCollectionIterator > volumeIt;
  volumeIt->SetCollection( volumeNodes );
  
  for ( volumeIt->InitTraversal(); ! volumeIt->IsDoneWithTraversal(); volumeIt->GoToNextItem() )
  {
    vtkMRMLScalarVolumeNode* volume = vtkMRMLScalarVolumeNode::SafeDownCast( volumeIt->GetCurrentObject() );
    if ( volume == NULL )
    {
      continue;
    }
    if ( volume->GetAttribute( "UltrasoundSnapshotsInput" )
         && std::string( volume->GetAttribute( "UltrasoundSnapshotsInput" ) ).compare( "true" ) == 0 )
    {
      return volume;
    }
  }
  
  return NULL;
}

  

void
vtkSlicerUltrasoundSnapshotsLogic
::AddSnapshot( vtkMRMLScalarVolumeNode* InputNode, bool preserveWindowLevel )
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
  snapshotDisp->SetSaveWithScene( 1 );
  snapshotDisp->SetDisableModifiedEvent( 0 );
  
  std::stringstream nameStream;
  nameStream << "UltrasoundSnapshots_Snapshot_";
  nameStream << this->snapshotCounter;
  
  vtkSmartPointer< vtkMRMLModelNode > snapshotModel = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( snapshotModel );
  snapshotModel->SetName( nameStream.str().c_str() );
  snapshotModel->SetDescription( "Live Ultrasound Snapshot" );
  snapshotModel->SetScene( this->GetMRMLScene() );
  snapshotModel->SetAndObserveDisplayNodeID( snapshotDisp->GetID() );
  snapshotModel->SetHideFromEditors( 0 );
  snapshotModel->SetSaveWithScene( 1 );
    
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
  
  
  // If the image is placed on a parent transform, get a copy of that transform.
  
  vtkSmartPointer< vtkTransform > ParentTransform = vtkSmartPointer< vtkTransform >::New();
  ParentTransform->Identity();
  if ( InputNode->GetParentTransformNode() != NULL )
  {
    vtkSmartPointer< vtkMatrix4x4 > ParentMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    InputNode->GetParentTransformNode()->GetMatrixTransformToWorld( ParentMatrix );
    ParentTransform->GetMatrix()->DeepCopy( ParentMatrix );
    ParentTransform->Update();
  }
  
  // Get the image transform from the image node. This actually only contains the
  // Image-to-Parent transform.

  vtkSmartPointer< vtkTransform > ImageToParentTransform = vtkSmartPointer< vtkTransform >::New();
  ImageToParentTransform->Identity();
  InputNode->GetIJKToRASMatrix( ImageToParentTransform->GetMatrix() );
  
  
  vtkSmartPointer< vtkTransform > tImageToRAS = vtkSmartPointer< vtkTransform >::New();
  tImageToRAS->Identity();
  tImageToRAS->Concatenate( ParentTransform );
  tImageToRAS->Concatenate( ImageToParentTransform );
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
  image->DeepCopy(InputNode->GetImageData());

  if ( preserveWindowLevel == true )
  {
    vtkSmartPointer< vtkImageMapToWindowLevelColors > mapToWindowLevelColors = vtkSmartPointer< vtkImageMapToWindowLevelColors >::New();
    
#if (VTK_MAJOR_VERSION <= 5)
	mapToWindowLevelColors->SetInput( image );
#else
    mapToWindowLevelColors->SetInputData( image );
#endif

    mapToWindowLevelColors->SetOutputFormatToLuminance();
    mapToWindowLevelColors->SetWindow( InputNode->GetScalarVolumeDisplayNode()->GetWindow() );
    mapToWindowLevelColors->SetLevel( InputNode->GetScalarVolumeDisplayNode()->GetLevel() );
    mapToWindowLevelColors->Update();
    image->DeepCopy( mapToWindowLevelColors->GetOutput() );
  }
  
  std::stringstream textureNameStream;
  textureNameStream << "UltrasoundSnapshots_Texture_";
  textureNameStream << this->snapshotCounter;
  
  vtkSmartPointer< vtkMRMLScalarVolumeNode > snapshotTexture = vtkSmartPointer< vtkMRMLScalarVolumeNode >::New();
  this->GetMRMLScene()->AddNode( snapshotTexture );
  snapshotTexture->SetName( textureNameStream.str().c_str() );
  snapshotTexture->SetDescription( "Live Ultrasound Snapshot Texture" );
  snapshotTexture->SetAndObserveImageData( image );
  snapshotTexture->CopyOrientation( InputNode );
  snapshotTexture->SetSpacing( InputNode->GetSpacing() );
  snapshotTexture->SetOrigin( InputNode->GetOrigin() );
  snapshotTexture->SetIJKToRASMatrix( tImageToRAS->GetMatrix() );
  
  std::stringstream textureDisplayNameStream;
  textureDisplayNameStream << "UltrasoundSnapshots_TextureDisplay_";
  textureDisplayNameStream << this->snapshotCounter;  
  
  vtkSmartPointer< vtkMRMLScalarVolumeDisplayNode > snapshotTextureDisplay = vtkSmartPointer< vtkMRMLScalarVolumeDisplayNode >::New();
  this->GetMRMLScene()->AddNode( snapshotTextureDisplay );
  snapshotTextureDisplay->SetName( textureDisplayNameStream.str().c_str() );
  snapshotTextureDisplay->SetAutoWindowLevel(0);
  snapshotTextureDisplay->SetWindow(256);
  snapshotTextureDisplay->SetLevel(128);
  snapshotTextureDisplay->SetDefaultColorMap();
  
  snapshotTexture->AddAndObserveDisplayNodeID( snapshotTextureDisplay->GetID() );   
  
  snapshotModel->SetAttribute( "TextureNodeID", snapshotTexture->GetID() );
  // snapshotModel->GetModelDisplayNode()->SetAndObserveTextureImageData( snapshotTexture->GetImageData() );
  snapshotModel->GetDisplayNode()->SetTextureImageDataConnection( snapshotTexture->GetImageDataConnection() );
  
  this->snapshotCounter++;  
}



void
vtkSlicerUltrasoundSnapshotsLogic
::ClearSnapshots()
{
  vtkSmartPointer< vtkCollection > modelNodes = vtkSmartPointer< vtkCollection >::Take( this->GetMRMLScene()->GetNodesByClass( "vtkMRMLModelNode" ) );
  vtkNew< vtkCollectionIterator > modelIt;
  modelIt->SetCollection( modelNodes );
  
  for ( modelIt->InitTraversal(); ! modelIt->IsDoneWithTraversal(); modelIt->GoToNextItem() )
  {
    vtkMRMLModelNode* snapshotModel = vtkMRMLModelNode::SafeDownCast( modelIt->GetCurrentObject() );
    
    if ( snapshotModel != NULL && std::string(snapshotModel->GetName()).find( "UltrasoundSnapshots_Snapshot_" ) != std::string::npos )
    {
      vtkMRMLScalarVolumeNode* snapshotTexture = vtkMRMLScalarVolumeNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( snapshotModel->GetAttribute("TextureNodeID") ) );    
      if ( snapshotTexture != NULL )
      {
        this->GetMRMLScene()->RemoveNode( snapshotTexture );
        snapshotTexture = NULL;
      }
      this->GetMRMLScene()->RemoveNode( snapshotModel );
      this->snapshotCounter--;
    }
  }
}



//---------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartCloseEvent);
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
void vtkSlicerUltrasoundSnapshotsLogic::OnMRMLSceneEndImport()
{
  assert(this->GetMRMLScene() != 0);

  vtkCollection* snapshotNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLModelNode" );
  vtkCollectionIterator* snapshotIt = vtkCollectionIterator::New();
  snapshotIt->SetCollection( snapshotNodes );
  
  for ( snapshotIt->InitTraversal(); ! snapshotIt->IsDoneWithTraversal(); snapshotIt->GoToNextItem() )
  {
    vtkMRMLModelNode* snapshotModel = vtkMRMLModelNode::SafeDownCast( snapshotIt->GetCurrentObject() );
    if ( snapshotModel != NULL && std::string(snapshotModel->GetName()).find( "UltrasoundSnapshots_Snapshot_" ) != std::string::npos )
    {
      vtkMRMLScalarVolumeNode* snapshotTexture = vtkMRMLScalarVolumeNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( snapshotModel->GetAttribute("TextureNodeID") ) );
      if ( snapshotTexture != NULL )
      {
        // snapshotModel->GetModelDisplayNode()->SetAndObserveTextureImageData( snapshotTexture->GetImageData() );
		  snapshotModel->GetDisplayNode()->SetTextureImageDataConnection( snapshotTexture->GetImageDataConnection() );
        this->snapshotCounter++;
      }        
    }
  }
  
  snapshotIt->Delete();
  snapshotNodes->Delete();
  
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerUltrasoundSnapshotsLogic::OnMRMLSceneStartClose()
{
  assert(this->GetMRMLScene() != 0);

  vtkCollection* snapshotNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLModelNode" );
  vtkCollectionIterator* snapshotIt = vtkCollectionIterator::New();
  snapshotIt->SetCollection( snapshotNodes );
  
  for ( snapshotIt->InitTraversal(); ! snapshotIt->IsDoneWithTraversal(); snapshotIt->GoToNextItem() )
  {
    vtkMRMLModelNode* snapshotModel = vtkMRMLModelNode::SafeDownCast( snapshotIt->GetCurrentObject() );
    if ( snapshotModel != NULL && std::string(snapshotModel->GetName()).find( "UltrasoundSnapshots_Snapshot_" ) != std::string::npos )
    {
      this->snapshotCounter--;
    }
  }
  
  snapshotIt->Delete();
  snapshotNodes->Delete();
  
  this->Modified();
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

