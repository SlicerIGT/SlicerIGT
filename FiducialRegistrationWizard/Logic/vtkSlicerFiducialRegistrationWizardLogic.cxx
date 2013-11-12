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

// FiducialRegistrationWizard includes
#include "vtkSlicerFiducialRegistrationWizardLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include <vtkMRMLMarkupsFiducialNode.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// STD includes
#include <cassert>
#include <sstream>


vtkStandardNewMacro(vtkSlicerFiducialRegistrationWizardLogic);



vtkSlicerFiducialRegistrationWizardLogic::vtkSlicerFiducialRegistrationWizardLogic()
{
  this->ProbeTransformNode = NULL;
  this->MarkupsFiducialNode = NULL;
  
  this->Counter = 0;
}



vtkSlicerFiducialRegistrationWizardLogic::~vtkSlicerFiducialRegistrationWizardLogic()
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



void vtkSlicerFiducialRegistrationWizardLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "ProbeTransformNode: "
     << ( this->ProbeTransformNode ? this->ProbeTransformNode->GetNodeTagName() : "(none)" ) << std::endl;
}



void vtkSlicerFiducialRegistrationWizardLogic
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



void vtkSlicerFiducialRegistrationWizardLogic
::SetProbeTransformNode( vtkMRMLLinearTransformNode *node )
{
  vtkSetMRMLNodeMacro( this->ProbeTransformNode, node );
  this->Modified();
}



void vtkSlicerFiducialRegistrationWizardLogic
::SetMarkupsFiducialNode( vtkMRMLMarkupsFiducialNode *node )
{
  vtkSetMRMLNodeMacro( this->MarkupsFiducialNode, node );
  this->Modified();
}



void vtkSlicerFiducialRegistrationWizardLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerFiducialRegistrationWizardLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerFiducialRegistrationWizardLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerFiducialRegistrationWizardLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}



void vtkSlicerFiducialRegistrationWizardLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

