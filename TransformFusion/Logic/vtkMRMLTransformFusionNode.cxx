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

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformFusionNode);

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::vtkMRMLTransformFusionNode()
{
  //Parameters
  //this->InputTransforms;
  this->UpdatesPerSecond = 60;
}

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::~vtkMRMLTransformFusionNode()
{
  //delete[] this->InputTransformIDs;
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
    
    /*
    if (!strcmp(attName,"InputTransformIDs")){
      std::stringstream ss;
      ss << attValue;
      ss >> this->InputTransformIDs;
      continue;
    }
    */
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
  
  //of << indent << " InputTransformIDs=\""<< this->InputTransformIDs << "\"";
  of << indent << " UpdatesPerSecond=\""<< this->UpdatesPerSecond << "\"";    
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTransformFusionNode *node = vtkMRMLTransformFusionNode::SafeDownCast(anode);
  this->DisableModifiedEventOn();
  
  //this->InputTransformIDs = node->InputTransformIDs;
  this->UpdatesPerSecond = node->UpdatesPerSecond;

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTransformFusionNode::GetOutputTransformNode()
{
  return this->GetNodeReference(vtkMRMLTransformFusionNode::OutputTransformReferenceRole.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveOutputTransformNode(vtkMRMLNode* node)
{
  this->SetNthNodeReferenceID(vtkMRMLTransformFusionNode::OutputTransformReferenceRole.c_str(),0,node->GetID());
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::AddInputTransform(vtkMRMLLinearTransformNode* inputTransform)
{
  this->InputTransforms.push_back(inputTransform);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::RemoveInputTransform(int index)
{
  this->InputTransforms.erase(this->InputTransforms.begin()+index);
}

//----------------------------------------------------------------------------
std::vector<vtkMRMLLinearTransformNode*> vtkMRMLTransformFusionNode::GetInputTransforms()
{
  return this->InputTransforms;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << " InputTransformIDs = ";
  for (int i = 0;i < this->InputTransforms.size();i++)
  {
    os << this->InputTransforms[i]->GetID() << " ";
  }
  os << "\n";

  os << indent << " UpdatesPerSecond = "<< this->UpdatesPerSecond << "\n";   
}

