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
  
  This file was originally developed by Franklin King, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkMRMLTransformFusionNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

#include <sstream>


//----------------------------------------------------------------------------
std::string vtkMRMLTransformFusionNode::OutputTransformReferenceRole = std::string("outputTransform");
std::string vtkMRMLTransformFusionNode::InputTransformsReferenceRole = std::string("inputTransforms");

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformFusionNode);

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::vtkMRMLTransformFusionNode()
{
  //Parameters
  this->UpdatesPerSecond = 60;
}

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::~vtkMRMLTransformFusionNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::ReadXMLAttributes(const char** atts)
{
  std::cerr << "Reading TransformFusion parameter node" << std::endl;
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL){
    attName = *(atts++);
    attValue = *(atts++);
    
    if (!strcmp(attName,"UpdatesPerSecond")){
      std::stringstream ss;
      ss << attValue;
      ss >> this->UpdatesPerSecond;
      continue;
    }    
  }

  this->WriteXML(std::cout,1);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
  
  of << indent << " UpdatesPerSecond=\""<< this->UpdatesPerSecond << "\"";    
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTransformFusionNode *node = vtkMRMLTransformFusionNode::SafeDownCast(anode);
  this->DisableModifiedEventOn();
  
  this->UpdatesPerSecond = node->UpdatesPerSecond;

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetOutputTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(vtkMRMLTransformFusionNode::OutputTransformReferenceRole.c_str()));
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveOutputTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetNthNodeReferenceID(vtkMRMLTransformFusionNode::OutputTransformReferenceRole.c_str(),0,node->GetID());
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetInputTransformNode(int n)
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNthNodeReference(vtkMRMLTransformFusionNode::InputTransformsReferenceRole.c_str(), n));
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::AddAndObserveInputTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->AddNodeReferenceID(vtkMRMLTransformFusionNode::InputTransformsReferenceRole.c_str(),node->GetID());
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::RemoveInputTransformNode(int n)
{
  this->RemoveNthNodeReferenceID(vtkMRMLTransformFusionNode::InputTransformsReferenceRole.c_str(),n);
}

//----------------------------------------------------------------------------
int vtkMRMLTransformFusionNode::GetNumberOfInputTransformNodes()
{
  return this->GetNumberOfNodeReferences(vtkMRMLTransformFusionNode::InputTransformsReferenceRole.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << " UpdatesPerSecond = "<< this->UpdatesPerSecond << "\n";   
}

