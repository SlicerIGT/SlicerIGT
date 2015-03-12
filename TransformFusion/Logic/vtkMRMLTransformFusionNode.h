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

#pragma once
#ifndef __vtkMRMLTransformFusionNode_h
#define __vtkMRMLTransformFusionNode_h

#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include "vtkSlicerTransformFusionModuleLogicExport.h"

/// \ingroup Slicer_QtModules_TransformFusion
class VTK_SLICER_TRANSFORMFUSION_MODULE_LOGIC_EXPORT vtkMRMLTransformFusionNode : 
  public vtkMRMLNode
{
public:   

  static vtkMRMLTransformFusionNode *New();
  vtkTypeMacro(vtkMRMLTransformFusionNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  static std::string OutputTransformReferenceRole;
  static std::string InputTransformsReferenceRole;

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual void ReadXMLAttributes( const char** atts);
  virtual void WriteXML(ostream& of, int indent);
  virtual void Copy(vtkMRMLNode *node);
  virtual const char* GetNodeTagName() {return "TransformFusionParameters";};

public:
  vtkMRMLLinearTransformNode* GetOutputTransformNode();
  void SetAndObserveOutputTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetInputTransformNode(int n);
  void AddAndObserveInputTransformNode(vtkMRMLLinearTransformNode* node);
  void RemoveInputTransformNode(int n);
  int GetNumberOfInputTransformNodes();

  vtkSetMacro(UpdatesPerSecond, int);
  vtkGetMacro(UpdatesPerSecond, int); 
  
protected:
  vtkMRMLTransformFusionNode();
  ~vtkMRMLTransformFusionNode();

  vtkMRMLTransformFusionNode(const vtkMRMLTransformFusionNode&);
  void operator=(const vtkMRMLTransformFusionNode&);

//Parameters
protected:
  //std::vector<vtkMRMLLinearTransformNode*> InputTransforms;
  int UpdatesPerSecond;
};

#endif
