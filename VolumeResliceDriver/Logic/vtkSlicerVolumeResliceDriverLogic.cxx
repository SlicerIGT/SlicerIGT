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
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVolumeResliceDriverLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumeResliceDriverLogic::vtkSlicerVolumeResliceDriverLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerVolumeResliceDriverLogic::~vtkSlicerVolumeResliceDriverLogic()
{
  this->ClearObservedNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeResliceDriverLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerVolumeResliceDriverLogic
::SetDriverForSlice( std::string nodeID, vtkMRMLSliceNode* sliceNode )
{
  if ( nodeID.size() < 1 )
  {
    sliceNode->RemoveAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE );
    return;
  }
  
  sliceNode->SetAttribute( VOLUMERESLICEDRIVER_DRIVER_ATTRIBUTE, nodeID.c_str() );
}



void vtkSlicerVolumeResliceDriverLogic
::SetMethodForSlice( int method, vtkMRMLSliceNode* sliceNode )
{
  if ( method == this->METHOD_DEFAULT )
  {
    method = this->METHOD_POSITION;
  }
  
  std::stringstream methodSS;
  methodSS << method;
  sliceNode->SetAttribute( VOLUMERESLICEDRIVER_METHOD_ATTRIBUTE, methodSS.str().c_str() );
}



void vtkSlicerVolumeResliceDriverLogic
::SetOrientationForSlice( int orientation, vtkMRMLSliceNode* sliceNode )
{
  if ( orientation == this->ORIENTATION_DEFAULT )
  {
    orientation = this->ORIENTATION_INPLANE;
  }
  
  std::stringstream orientationSS;
  orientationSS << orientation;
  sliceNode->SetAttribute( VOLUMERESLICEDRIVER_ORIENTATION_ATTRIBUTE, orientationSS.str().c_str() );
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

//---------------------------------------------------------------------------
void vtkSlicerVolumeResliceDriverLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
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
::OnMRMLNodeModified( vtkMRMLNode* node )
{
  std::cout << "Observed node modified." << std::endl;
}



void vtkSlicerVolumeResliceDriverLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void * callData )
{
  switch ( event )
  {
    case vtkMRMLTransformableNode::TransformModifiedEvent:
      std::cout << "Oberved node transform changed." << std::endl;
      break;
    default:
      this->Superclass::ProcessMRMLNodesEvents( caller, event, callData );
      break;
  }
}

