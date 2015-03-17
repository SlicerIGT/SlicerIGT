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
#include <vtkMRMLScene.h>
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
//#include <vtkQuaternionInterpolator.h>

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
void vtkSlicerTransformFusionLogic::fuseInputTransforms(int fusionTechnique) //enum techniqueTypes
{
  switch (fusionTechnique){
    case MODE_QUATERNION_AVERAGE:
    {
      this->QuaternionAverageFusion();
      break;
    }
  } 
}


//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::QuaternionAverageFusion()
{
  vtkMRMLLinearTransformNode* outputNode = this->TransformFusionNode->GetOutputTransformNode();
  if (outputNode == NULL)
  {
    return;
  }

  // Average quaternion
  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  int numberOfInputs = this->TransformFusionNode->GetNumberOfInputTransformNodes();
#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
  vtkSmartPointer<vtkMatrix4x4> matrix4x4Pointer = vtkSmartPointer<vtkMatrix4x4>::New();
#else
  vtkMatrix4x4* matrix4x4Pointer = NULL;
#endif

  float rotationMatrix[3][3] = {{0}};
  float averageRotationMatrix[3][3] = {{0}};
  float singleQuaternion[4] = {0};
  float averageQuaternion[4] = {0};

  for (int i = 0; i < numberOfInputs; i++)
  {

#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
    this->TransformFusionNode->GetInputTransformNode(i)->GetMatrixTransformToParent(matrix4x4Pointer);
#else
    matrix4x4Pointer = this->TransformFusionNode->GetInputTransformNode(i)->GetMatrixTransformToParent();
#endif

    for (int row = 0; row < 3; row++)
    {
      for (int column = 0; column < 3; column++)
      { 
        rotationMatrix[row][column] = matrix4x4Pointer->GetElement(row,column);
      }
    }

    vtkMath::Matrix3x3ToQuaternion(rotationMatrix, singleQuaternion);

    averageQuaternion[0] = averageQuaternion[0] + singleQuaternion[0];
    averageQuaternion[1] = averageQuaternion[1] + singleQuaternion[1];
    averageQuaternion[2] = averageQuaternion[2] + singleQuaternion[2];
    averageQuaternion[3] = averageQuaternion[3] + singleQuaternion[3];
  }

  averageQuaternion[0] = averageQuaternion[0] / numberOfInputs;
  averageQuaternion[1] = averageQuaternion[1] / numberOfInputs;
  averageQuaternion[2] = averageQuaternion[2] / numberOfInputs;
  averageQuaternion[3] = averageQuaternion[3] / numberOfInputs;

  float magnitude = sqrt(averageQuaternion[0]*averageQuaternion[0] + averageQuaternion[1]*averageQuaternion[1] + averageQuaternion[2]*averageQuaternion[2] + averageQuaternion[3]*averageQuaternion[3]);
  averageQuaternion[0] = averageQuaternion[0]/magnitude;
  averageQuaternion[1] = averageQuaternion[1]/magnitude;
  averageQuaternion[2] = averageQuaternion[2]/magnitude;
  averageQuaternion[3] = averageQuaternion[3]/magnitude;

  vtkMath::QuaternionToMatrix3x3(averageQuaternion,averageRotationMatrix);
  
  for (int row = 0; row < 3; row++)
  {
    for (int column = 0; column < 3; column++)
    { 
      outputMatrix->SetElement(row,column,averageRotationMatrix[row][column]);
    }
  }


  // Average linear elements
  double value = 0;
  for (int row = 0; row < 4; row++)
  {
    for (int i = 0; i < numberOfInputs; i++)
    {
#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
      this->TransformFusionNode->GetInputTransformNode(i)->GetMatrixTransformToParent(matrix4x4Pointer);
      value += matrix4x4Pointer->GetElement(row,3);
#else
      value += this->TransformFusionNode->GetInputTransformNode(i)->GetMatrixTransformToParent()->GetElement(row,3);
#endif
    }
    value = value/numberOfInputs;
    outputMatrix->SetElement(row,3,value);
    value = 0;
  }

#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
  outputNode->SetMatrixTransformToParent(outputMatrix);
#else
  outputNode->SetAndObserveMatrixTransformToParent(outputMatrix);
#endif
}