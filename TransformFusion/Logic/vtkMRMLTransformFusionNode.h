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

#include <vtkCommand.h>

#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include "vtkSlicerTransformFusionModuleLogicExport.h"

/// \ingroup Slicer_QtModules_TransformFusion
class VTK_SLICER_TRANSFORMFUSION_MODULE_LOGIC_EXPORT vtkMRMLTransformFusionNode : 
  public vtkMRMLNode
{
public:   

  enum Events
  {
    /// The node stores both inputs (e.g., transforms, etc.) and parameters.
    /// InputDataModifiedEvent is only invoked when inputs are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 777 is just a random value that is very unlikely to be used for anything else in this class

    // TODO: Call Modified Event on output node modified?
    InputDataModifiedEvent = vtkCommand::UserEvent + 777
  };

  enum
  {
    UPDATE_MODE_MANUAL = 0,
    UPDATE_MODE_AUTO,
    UPDATE_MODE_TIMED,
    UPDATE_MODE_LAST // do not set to this one
  };

  enum
  {
    FUSION_MODE_QUATERNION_AVERAGE = 0,
    FUSION_MODE_CONSTRAIN_SHAFT_ROTATION,
    FUSION_MODE_LAST // do not set to this one
  };

  static vtkMRMLTransformFusionNode *New();
  vtkTypeMacro( vtkMRMLTransformFusionNode, vtkMRMLNode );
  void PrintSelf( ostream& os, vtkIndent indent );

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  virtual const char* GetNodeTagName() { return "TransformFusionParameters"; };

public:
  vtkMRMLLinearTransformNode* GetOutputTransformNode();
  void SetAndObserveOutputTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetRestingTransformNode();
  void SetAndObserveRestingTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetReferenceTransformNode();
  void SetAndObserveReferenceTransformNode( vtkMRMLLinearTransformNode* node );

  // these input functions should be used when there is only one "input" transform
  vtkMRMLLinearTransformNode* GetSingleInputTransformNode();
  void SetAndObserveSingleInputTransformNode( vtkMRMLLinearTransformNode* node );

  // these input functions should only be used when there are multiple "inputs"
  vtkMRMLLinearTransformNode* GetNthInputTransformNode( int n );
  void AddAndObserveInputTransformNode( vtkMRMLLinearTransformNode* node );
  void RemoveInputTransformNode( int n );
  int GetNumberOfInputTransformNodes();
  
  void ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData );

  vtkSetMacro( UpdatesPerSecond, int );
  vtkGetMacro( UpdatesPerSecond, int ); 

  vtkGetMacro( FusionMode, int );
  void SetFusionMode( int );

  vtkGetMacro( UpdateMode, int );
  void SetUpdateMode( int );

  static std::string GetFusionModeAsString( int );
  static int GetFusionModeFromString( std::string );

  static std::string GetUpdateModeAsString( int );
  static int GetUpdateModeFromString( std::string );

protected:
  vtkMRMLTransformFusionNode();
  ~vtkMRMLTransformFusionNode();

  vtkMRMLTransformFusionNode( const vtkMRMLTransformFusionNode& );
  void operator=( const vtkMRMLTransformFusionNode& );

//Parameters
protected:
  int UpdatesPerSecond;
  int FusionMode;
  int UpdateMode;
};

#endif
