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

// TransformFusion includes
#include "vtkSlicerTransformFusionLogic.h"
#include "vtkMRMLTransformFusionNode.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>
#include <sstream>

vtkStandardNewMacro(vtkSlicerTransformFusionLogic);

//-----------------------------------------------------------------------------
vtkSlicerTransformFusionLogic::vtkSlicerTransformFusionLogic()
{

}

//-----------------------------------------------------------------------------
vtkSlicerTransformFusionLogic::~vtkSlicerTransformFusionLogic()
{

}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == NULL)
  {
    vtkErrorMacro("Null scene");
    return;
  }
  
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLTransformFusionNode>::New());
}

//----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::SetAndObserveTransformFusionNode(vtkMRMLTransformFusionNode *node)
{
  vtkSetAndObserveMRMLNodeMacro(this->TransformFusionNode, node);
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::fuseInputTransforms()
{


}




