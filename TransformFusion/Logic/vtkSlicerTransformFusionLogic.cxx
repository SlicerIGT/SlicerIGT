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
#include <vtkQuaternionInterpolator.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <cassert>
#include <sstream>

vtkStandardNewMacro(vtkSlicerTransformFusionLogic);

//-----------------------------------------------------------------------------
vtkSlicerTransformFusionLogic::vtkSlicerTransformFusionLogic()
{
  this->TransformFusionNode = NULL;
}

//-----------------------------------------------------------------------------
vtkSlicerTransformFusionLogic::~vtkSlicerTransformFusionLogic()
{
  vtkSetAndObserveMRMLNodeMacro(this->TransformFusionNode, NULL);
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
void vtkSlicerTransformFusionLogic::fuseInputTransforms(int techniqueType) //enum techniqueTypes
{
  switch (techniqueType){
    case SIMPLE_AVERAGE:
    {
      this->SimpleAverage();
      break;
    }
    case LERP_AND_SLERP:
    {
      this->LerpAndSlerp();
      break;
    }
  } 

}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::SimpleAverage()
{
  vtkMRMLLinearTransformNode* outputNode = vtkMRMLLinearTransformNode::SafeDownCast(this->TransformFusionNode->GetOutputTransformNode());
  if (outputNode == NULL)
  {
    return;
  }
  std::vector<vtkMRMLLinearTransformNode*> inputTransforms = this->TransformFusionNode->GetInputTransforms();
  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  double value = 0;

  for (int column = 0; column < 4; column++)
  {
    for (int row = 0; row < 4; row++)
    {
      for (int i = 0; i < inputTransforms.size(); i++)
      {
        value += inputTransforms[i]->GetMatrixTransformToParent()->GetElement(row,column);
      }
      value = value/inputTransforms.size();
      outputMatrix->SetElement(row,column,value);
      value = 0;
    }
  }

  outputNode->SetAndObserveMatrixTransformToParent(outputMatrix);
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::LerpAndSlerp()
{
  //Not implemented yet
  /*
  std::vector<vtkMRMLLinearTransformNode*> inputTransforms = this->TransformFusionNode->GetInputTransforms();
  vtkSmartPointer<vtkQuaternionInterpolator> slerper = vtkSmartPointer<vtkQuaternionInterpolator>::New();
  vtkMatrix4x4* matrixPointer = NULL;

  for (int i = 0; i < inputTransforms.size(); i++)
  {
    matrixPointer = inputTransforms[i]->GetMatrixTransformToParent();
  }
  */
}