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

// VolumeResliceDriver includes
#include "vtkSlicerVolumeResliceDriverLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLAnnotationRulerNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>



vtkStandardNewMacro(vtkSlicerVolumeResliceDriverLogic);



vtkSlicerVolumeResliceDriverLogic
::vtkSlicerVolumeResliceDriverLogic()
{
}



vtkSlicerVolumeResliceDriverLogic
::~vtkSlicerVolumeResliceDriverLogic()
{
  this->ClearObservedNodes();
}



void vtkSlicerVolumeResliceDriverLogic
::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "Number of observed nodes: " << this->ObservedNodes.size() << std::endl;
  os << indent << "Observed nodes:";
  
  for ( unsigned int i = 0; i < this->ObservedNodes.size(); ++ i )
  {
    os << " " << this->ObservedNodes[ i ]->GetID();
  }
  
  os << std::endl;
}



void vtkSlicerVolumeResliceDriverLogic
::SetDriverForSlice( std::string nodeID, vtkMRMLSliceNode* sliceNode )
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID( nodeID );
  if ( node == NULL )
    {
    sliceNode->RemoveAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE );
    return;
    }
  
  vtkMRMLTransformableNode* tnode = vtkMRMLTransformableNode::SafeDownCast( node );
  if ( tnode == NULL )
    {
    sliceNode->RemoveAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE );
    return;
    }
  
  sliceNode->SetAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE, nodeID.c_str() );
  this->AddObservedNode( tnode );
  
  this->UpdateSliceIfObserved( sliceNode );
}


void vtkSlicerVolumeResliceDriverLogic
::SetModeForSlice( int mode, vtkMRMLSliceNode* sliceNode )
{
  if ( sliceNode == NULL )
    {
    return;
    }
  
  std::stringstream modeSS;
  modeSS << mode;
  sliceNode->SetAttribute( VOLUMERESLICEDRIVER_MODE_ATTRIBUTE, modeSS.str().c_str() );
  
  this->UpdateSliceIfObserved( sliceNode );
}


void vtkSlicerVolumeResliceDriverLogic
::AddObservedNode( vtkMRMLTransformableNode* node )
{
  for ( unsigned int i = 0; i < this->ObservedNodes.size(); ++ i )
    {
    if ( node == this->ObservedNodes[ i ] )
      {
      return;
      }
    }
  
  int wasModifying = this->StartModify();
  
  vtkMRMLTransformableNode* newNode = NULL;
  
  vtkSmartPointer< vtkIntArray > events = vtkSmartPointer< vtkIntArray >::New();
  events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent );
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLVolumeNode::ImageDataModifiedEvent );
  vtkSetAndObserveMRMLNodeEventsMacro( newNode, node, events );
  this->ObservedNodes.push_back( newNode );
  
  this->EndModify( wasModifying );
}



void vtkSlicerVolumeResliceDriverLogic
::ClearObservedNodes()
{
  for ( unsigned int i = 0; i < this->ObservedNodes.size(); ++ i )
    {
    vtkSetAndObserveMRMLNodeMacro( this->ObservedNodes[ i ], 0 );
    }
  
  this->ObservedNodes.clear();
}



void vtkSlicerVolumeResliceDriverLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerVolumeResliceDriverLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}



/// This method is called by Slicer after each significant MRML scene event (import, load, etc.)
void vtkSlicerVolumeResliceDriverLogic
::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
  
  // Check if any of the slice nodes contain driver transoforms that need to be observed.
  
  vtkCollection* sliceNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLSliceNode" );
  vtkCollectionIterator* sliceIt = vtkCollectionIterator::New();
  sliceIt->SetCollection( sliceNodes );
  for ( sliceIt->InitTraversal(); ! sliceIt->IsDoneWithTraversal(); sliceIt->GoToNextItem() )
    {
    vtkMRMLSliceNode* slice = vtkMRMLSliceNode::SafeDownCast( sliceIt->GetCurrentObject() );
    if ( slice == NULL )
      {
      continue;
      }
    const char* driverCC = slice->GetAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE );
    if ( driverCC == NULL )
      {
      continue;
      }
    vtkMRMLNode* driverNode = this->GetMRMLScene()->GetNodeByID( driverCC );
    if ( driverNode == NULL )
      {
      continue;
      }
    vtkMRMLTransformableNode* driverTransformable = vtkMRMLTransformableNode::SafeDownCast( driverNode );
    if ( driverTransformable == NULL )
      {
      continue;
      }
    this->AddObservedNode( driverTransformable );
    }
  sliceIt->Delete();
  sliceNodes->Delete();
  
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeResliceDriverLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeResliceDriverLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}



void vtkSlicerVolumeResliceDriverLogic
::OnMRMLNodeModified( vtkMRMLNode* vtkNotUsed(node) )
{
  std::cout << "Observed node modified." << std::endl;
}



void vtkSlicerVolumeResliceDriverLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void * callData )
{
  if ( caller == NULL )
    {
    return;
    }
  
  if (    event != vtkMRMLTransformableNode::TransformModifiedEvent
       && event != vtkCommand::ModifiedEvent
       && event != vtkMRMLVolumeNode::ImageDataModifiedEvent )
    {
    this->Superclass::ProcessMRMLNodesEvents( caller, event, callData );
    }
  
  vtkMRMLTransformableNode* callerNode = vtkMRMLTransformableNode::SafeDownCast( caller );
  if ( callerNode == NULL )
    {
    return;
    }
  
  std::string callerNodeID( callerNode->GetID() );
  
  //vtkMRMLNode* node = 0;
  std::vector< vtkMRMLSliceNode* > SlicesToDrive;
  
  vtkCollection* sliceNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLSliceNode" );
  vtkCollectionIterator* sliceIt = vtkCollectionIterator::New();
  sliceIt->SetCollection( sliceNodes );
  sliceIt->InitTraversal();
  for ( int i = 0; i < sliceNodes->GetNumberOfItems(); ++ i )
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast( sliceIt->GetCurrentObject() );
    sliceIt->GoToNextItem();
    const char* driverCC = sliceNode->GetAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE );
    if (    sliceNode != NULL
         && driverCC != NULL
         && callerNodeID.compare( std::string( driverCC ) ) == 0 )
      {
      SlicesToDrive.push_back( sliceNode );
      }
    }
  sliceIt->Delete();
  sliceNodes->Delete();
  
  for ( unsigned int i = 0; i < SlicesToDrive.size(); ++ i )
    {
    this->UpdateSliceByTransformableNode( callerNode, SlicesToDrive[ i ] );
    }
}



void vtkSlicerVolumeResliceDriverLogic
::UpdateSliceByTransformableNode( vtkMRMLTransformableNode* tnode, vtkMRMLSliceNode* sliceNode )
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( tnode );
  if ( transformNode != NULL )
    {
    this->UpdateSliceByTransformNode( transformNode, sliceNode );
    }
  
  vtkMRMLScalarVolumeNode* imageNode = vtkMRMLScalarVolumeNode::SafeDownCast( tnode );
  if ( imageNode != NULL )
    {
    this->UpdateSliceByImageNode( imageNode, sliceNode );
    }

  vtkMRMLAnnotationRulerNode* rulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast( tnode );
  if ( rulerNode != NULL )
    {
    this->UpdateSliceByRulerNode( rulerNode, sliceNode );
    }
}



void vtkSlicerVolumeResliceDriverLogic
::UpdateSliceByTransformNode( vtkMRMLLinearTransformNode* tnode, vtkMRMLSliceNode* sliceNode )
{
  if ( ! tnode)
    {
    return;
    }
  
  vtkSmartPointer< vtkMatrix4x4 > transform = vtkSmartPointer< vtkMatrix4x4 >::New();
  transform->Identity();
  int getTransf = tnode->GetMatrixTransformToWorld( transform );
  if( getTransf != 0 )
    {
    this->UpdateSlice( transform, sliceNode );
    }
}


void vtkSlicerVolumeResliceDriverLogic
::UpdateSliceByImageNode( vtkMRMLScalarVolumeNode* inode, vtkMRMLSliceNode* sliceNode )
{
  vtkMRMLVolumeNode* volumeNode = inode;

  if (volumeNode == NULL)
    {
    return;
    }

  vtkSmartPointer<vtkMatrix4x4> rtimgTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix(rtimgTransform);

  float tx = rtimgTransform->GetElement(0, 0);
  float ty = rtimgTransform->GetElement(1, 0);
  float tz = rtimgTransform->GetElement(2, 0);
  float sx = rtimgTransform->GetElement(0, 1);
  float sy = rtimgTransform->GetElement(1, 1);
  float sz = rtimgTransform->GetElement(2, 1);
  float nx = rtimgTransform->GetElement(0, 2);
  float ny = rtimgTransform->GetElement(1, 2);
  float nz = rtimgTransform->GetElement(2, 2);
  float px = rtimgTransform->GetElement(0, 3);
  float py = rtimgTransform->GetElement(1, 3);
  float pz = rtimgTransform->GetElement(2, 3);

  vtkImageData* imageData;
  imageData = volumeNode->GetImageData();
  int size[3];
  imageData->GetDimensions(size);

  // normalize
  float psi = sqrt(tx*tx + ty*ty + tz*tz);
  float psj = sqrt(sx*sx + sy*sy + sz*sz);
  float psk = sqrt(nx*nx + ny*ny + nz*nz);
  float ntx = tx / psi;
  float nty = ty / psi;
  float ntz = tz / psi;
  float nsx = sx / psj;
  float nsy = sy / psj;
  float nsz = sz / psj;
  float nnx = nx / psk;
  float nny = ny / psk;
  float nnz = nz / psk;

  // Shift the center
  // NOTE: The center of the image should be shifted due to different
  // definitions of image origin between VTK (Slicer) and OpenIGTLink;
  // OpenIGTLink image has its origin at the center, while VTK image
  // has one at the corner.

  float hfovi = psi * size[0] / 2.0;
  float hfovj = psj * size[1] / 2.0;
  //float hfovk = psk * imgheader->size[2] / 2.0;
  float hfovk = 0;

  float cx = ntx * hfovi + nsx * hfovj + nnx * hfovk;
  float cy = nty * hfovi + nsy * hfovj + nny * hfovk;
  float cz = ntz * hfovi + nsz * hfovj + nnz * hfovk;

  rtimgTransform->SetElement(0, 0, ntx);
  rtimgTransform->SetElement(1, 0, nty);
  rtimgTransform->SetElement(2, 0, ntz);
  rtimgTransform->SetElement(0, 1, nsx);
  rtimgTransform->SetElement(1, 1, nsy);
  rtimgTransform->SetElement(2, 1, nsz);
  rtimgTransform->SetElement(0, 2, nnx);
  rtimgTransform->SetElement(1, 2, nny);
  rtimgTransform->SetElement(2, 2, nnz);
  rtimgTransform->SetElement(0, 3, px + cx);
  rtimgTransform->SetElement(1, 3, py + cy);
  rtimgTransform->SetElement(2, 3, pz + cz);

  vtkMRMLLinearTransformNode* parentNode =
    vtkMRMLLinearTransformNode::SafeDownCast(volumeNode->GetParentTransformNode());
  if (parentNode)
    {
    vtkSmartPointer<vtkMatrix4x4> parentTransform = vtkSmartPointer<vtkMatrix4x4>::New();
    parentTransform->Identity();
    int r = parentNode->GetMatrixTransformToWorld(parentTransform);
    if (r)
      {
      vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New();
      vtkMatrix4x4::Multiply4x4(parentTransform, rtimgTransform,  transform);
      this->UpdateSlice( transform, sliceNode );
      return;
      }
    }

  this->UpdateSlice( rtimgTransform, sliceNode );

}


void Cross(double *a, double *b, double *c)
{
    a[0] = b[1]*c[2] - c[1]*b[2];
    a[1] = c[0]*b[2] - b[0]*c[2];
    a[2] = b[0]*c[1] - c[0]*b[1];
}

void vtkSlicerVolumeResliceDriverLogic
::UpdateSliceByRulerNode( vtkMRMLAnnotationRulerNode* rnode, vtkMRMLSliceNode* sliceNode )
{

  vtkSmartPointer<vtkMatrix4x4> rulerTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  double position1[4];
  double position2[4];
  double t[3];
  double s[3];
  double n[3];
  double nlen;

  rnode->GetPositionWorldCoordinates1(position1);
  rnode->GetPositionWorldCoordinates2(position2);

  // Calculate <n> and normalize it.
  n[0] = position2[0]-position1[0];
  n[1] = position2[1]-position1[1];
  n[2] = position2[2]-position1[2];
  nlen = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
  n[0] /= nlen;
  n[1] /= nlen;
  n[2] /= nlen;

  // Check if <n> is not parallel to <s>=(0.0, 1.0, 0.0)
  if (n[1] < 1.0)
    {
    s[0] = 0.0;
    s[1] = 1.0;
    s[2] = 0.0;
    Cross(t, s, n);
    Cross(s, n, t);
    }
  else
    {
    t[0] = 1.0;
    t[1] = 0.0;
    t[2] = 0.0;
    Cross(s, n, t);
    Cross(t, s, n);
    }

  // normal vectors
  double ntx = t[0];
  double nty = t[1];
  double ntz = t[2];
  double nsx = s[0];
  double nsy = s[1];
  double nsz = s[2];
  double nnx = n[0];
  double nny = n[1];
  double nnz = n[2];
  double px = position2[0];
  double py = position2[1];
  double pz = position2[2];

  rulerTransform->SetElement(0, 0, ntx);
  rulerTransform->SetElement(1, 0, nty);
  rulerTransform->SetElement(2, 0, ntz);
  rulerTransform->SetElement(0, 1, nsx);
  rulerTransform->SetElement(1, 1, nsy);
  rulerTransform->SetElement(2, 1, nsz);
  rulerTransform->SetElement(0, 2, nnx);
  rulerTransform->SetElement(1, 2, nny);
  rulerTransform->SetElement(2, 2, nnz);
  rulerTransform->SetElement(0, 3, px);
  rulerTransform->SetElement(1, 3, py);
  rulerTransform->SetElement(2, 3, pz);

  this->UpdateSlice( rulerTransform, sliceNode );

}



/**
 * Updates the SliceToRAS matrix.
 * SliceToRAS is concatenated from SliceToDriver and DriverToRAS.
 * SliceToDriver depends how we want to orient the slice relative to the
 * driver object (in-plane, transverse, etc.)
 */
void vtkSlicerVolumeResliceDriverLogic
::UpdateSlice( vtkMatrix4x4* driverToRASMatrix, vtkMRMLSliceNode* sliceNode )
{
  int mode= MODE_NONE;

  const char* modeCC = sliceNode->GetAttribute( VOLUMERESLICEDRIVER_MODE_ATTRIBUTE );
  if ( modeCC != NULL )
  {
    std::stringstream modeSS( modeCC );
    modeSS >> mode;
  }

  float px = driverToRASMatrix->Element[0][3];
  float py = driverToRASMatrix->Element[1][3];
  float pz = driverToRASMatrix->Element[2][3];
  
  
  vtkSmartPointer< vtkTransform > driverToRASTransform = vtkSmartPointer< vtkTransform >::New();
  driverToRASTransform->SetMatrix( driverToRASMatrix );
  driverToRASTransform->Update();
  
  vtkSmartPointer< vtkTransform > sliceToDriverTransform = vtkSmartPointer< vtkTransform >::New();
  sliceToDriverTransform->Identity();
  
  switch (mode)
    {
    case MODE_AXIAL:
      sliceNode->SetOrientationToAxial();
      sliceNode->JumpSlice(px, py, pz);
      sliceNode->UpdateMatrices();
      return;
      break;
    case MODE_SAGITTAL:
      sliceNode->SetOrientationToSagittal();
      sliceNode->JumpSlice(px, py, pz);
      sliceNode->UpdateMatrices();
      return;
      break;
    case MODE_CORONAL:
      sliceNode->SetOrientationToCoronal();
      sliceNode->JumpSlice(px, py, pz);
      sliceNode->UpdateMatrices();
      return;
      break;
    case MODE_INPLANE:
      sliceToDriverTransform->RotateX( -90 );
      sliceToDriverTransform->RotateY( 90 );
      break;
    case MODE_INPLANE90:
      sliceToDriverTransform->RotateX( -90 );
      break;
    case MODE_TRANSVERSE:
      break;
    case MODE_TRANSVERSE180:
      sliceToDriverTransform->RotateZ( 180 );
      break;
    default: //     case MODE_NONE:
      return;
      break;
    };
  
  vtkSmartPointer< vtkTransform > sliceToRASTransform = vtkSmartPointer< vtkTransform >::New();
  sliceToRASTransform->Identity();
  sliceToRASTransform->Concatenate( driverToRASTransform );
  sliceToRASTransform->Concatenate( sliceToDriverTransform );
  sliceToRASTransform->Update();
  
  sliceNode->SetSliceToRAS( sliceToRASTransform->GetMatrix() );
  sliceNode->UpdateMatrices();
}


void vtkSlicerVolumeResliceDriverLogic
::UpdateSliceIfObserved( vtkMRMLSliceNode* sliceNode )
{
  if ( sliceNode == NULL )
    {
    return;
    }
  
  const char* driverCC = sliceNode->GetAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE );
  if ( driverCC == NULL )
    {
    return;
    }
  
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID( driverCC );
  
  sliceNode->Modified();
  node->InvokeEvent( vtkMRMLTransformableNode::TransformModifiedEvent );
}

