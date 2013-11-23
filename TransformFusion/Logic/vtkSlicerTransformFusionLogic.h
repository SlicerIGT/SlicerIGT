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

// .NAME vtkSlicerTransformFusionLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes
#ifndef __vtkSlicerTransformFusionLogic_h
#define __vtkSlicerTransformFusionLogic_h

#include <string>

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

class vtkMRMLTransformFusionNode;
class vtkMRMLLinearTransformNode;


// STD includes
#include <cstdlib>

#include "vtkSlicerTransformFusionModuleLogicExport.h"


/// \ingroup Slicer_QtModules_TransformFusion
class VTK_SLICER_TRANSFORMFUSION_MODULE_LOGIC_EXPORT vtkSlicerTransformFusionLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerTransformFusionLogic *New();
  vtkTypeMacro(vtkSlicerTransformFusionLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
public:
  void fuseInputTransforms();

  void SetAndObserveTransformFusionNode(vtkMRMLTransformFusionNode *node);
  vtkGetObjectMacro(TransformFusionNode, vtkMRMLTransformFusionNode);
  
protected:
  vtkSlicerTransformFusionLogic();
  ~vtkSlicerTransformFusionLogic();

  virtual void RegisterNodes();
  
private:
  vtkSlicerTransformFusionLogic(const vtkSlicerTransformFusionLogic&);// Not implemented
  void operator=(const vtkSlicerTransformFusionLogic&);// Not implemented

protected:
  /// Parameter set MRML node
  vtkMRMLTransformFusionNode* TransformFusionNode;

};

#endif

