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

// CollectFiducials includes
#include "vtkSlicerCollectFiducialsLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>
#include <sstream>


vtkStandardNewMacro(vtkSlicerCollectFiducialsLogic);



vtkSlicerCollectFiducialsLogic::vtkSlicerCollectFiducialsLogic()
{
  this->ProbeTransformNode = NULL;
  this->MarkupsFiducialNode = NULL;
  
  this->Counter = 0;
}



vtkSlicerCollectFiducialsLogic::~vtkSlicerCollectFiducialsLogic()
{
  if ( this->ProbeTransformNode != NULL )
  {
    this->ProbeTransformNode->Delete();
    this->ProbeTransformNode = NULL;
  }
  
  if ( this->MarkupsFiducialNode != NULL )
  {
    this->MarkupsFiducialNode->Delete();
    this->MarkupsFiducialNode = NULL;
  }
}



void vtkSlicerCollectFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "ProbeTransformNode: "
     << ( this->ProbeTransformNode ? this->ProbeTransformNode->GetNodeTagName() : "(none)" ) << std::endl;
}



void vtkSlicerCollectFiducialsLogic
::AddFiducial( std::string NameBase )
{
  if ( this->ProbeTransformNode == NULL || this->MarkupsFiducialNode == NULL )
  {
    return;
  }
  
  vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
  this->ProbeTransformNode->GetMatrixTransformToWorld( transformToWorld );
  
  double coord[ 3 ] = { transformToWorld->GetElement( 0, 3 ), transformToWorld->GetElement( 1, 3 ), transformToWorld->GetElement( 2, 3 ) };
  
  this->Counter++;
  int n = this->MarkupsFiducialNode->AddFiducialFromArray( coord );
  
  if ( NameBase.size() > 0 )
  {
    std::stringstream ss;
    ss << NameBase << "_" << this->Counter;
    this->MarkupsFiducialNode->SetNthFiducialLabel( n, ss.str().c_str() );
  }  
  
  //TODO: Add ability to change glyph scale when feature is added to Markups module
  //this->MarkupsFiducialNode-> ->SetGlyphScale( glyphScale );

  transformToWorld->Delete();
}



void vtkSlicerCollectFiducialsLogic
::SetProbeTransformNode( vtkMRMLLinearTransformNode *node )
{
  vtkSetMRMLNodeMacro( this->ProbeTransformNode, node );
  this->Modified();
}



void vtkSlicerCollectFiducialsLogic
::SetMarkupsFiducialNode( vtkMRMLMarkupsFiducialNode *node )
{
  vtkSetMRMLNodeMacro( this->MarkupsFiducialNode, node );
  this->Modified();
}



void vtkSlicerCollectFiducialsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerCollectFiducialsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerCollectFiducialsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerCollectFiducialsLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}



void vtkSlicerCollectFiducialsLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

