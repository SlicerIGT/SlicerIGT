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
#ifndef __vtkMRMLTransformProcessorNode_h
#define __vtkMRMLTransformProcessorNode_h

#include <vtkCommand.h>

#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLLinearTransformNode.h>

#include "vtkSlicerTransformProcessorModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_TransformProcessor
class VTK_SLICER_TRANSFORMPROCESSOR_MODULE_MRML_EXPORT vtkMRMLTransformProcessorNode : 
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
    UPDATE_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    PROCESSING_MODE_QUATERNION_AVERAGE = 0,
    PROCESSING_MODE_COMPUTE_SHAFT_PIVOT,
    PROCESSING_MODE_COMPUTE_ROTATION,
    PROCESSING_MODE_COMPUTE_TRANSLATION,
    PROCESSING_MODE_COMPUTE_FULL_TRANSFORM,
    PROCESSING_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    ROTATION_MODE_COPY_ALL_AXES = 0,
    ROTATION_MODE_COPY_SINGLE_AXIS,
    ROTATION_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    DEPENDENT_AXES_MODE_FROM_PIVOT = 0,
    DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS,
    DEPENDENT_AXES_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    AXIS_LABEL_X = 0,
    AXIS_LABEL_Y,
    AXIS_LABEL_Z,
    AXIS_LABEL_LAST // do not set to this type, insert valid types above this line
  };

  static vtkMRMLTransformProcessorNode *New();
  vtkTypeMacro( vtkMRMLTransformProcessorNode, vtkMRMLNode );
  void PrintSelf( ostream& os, vtkIndent indent );

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  virtual const char* GetNodeTagName() { return "TransformProcessorParameters"; };

  // begin accessors and mutators
  vtkMRMLLinearTransformNode* GetNthInputCombineTransformNode( int n );
  void AddAndObserveInputCombineTransformNode( vtkMRMLLinearTransformNode* node );
  void RemoveNthInputCombineTransformNode( int n );
  int GetNumberOfInputCombineTransformNodes();

  vtkMRMLLinearTransformNode* GetInputFromTransformNode();
  void SetAndObserveInputFromTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetInputToTransformNode();
  void SetAndObserveInputToTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetInputInitialTransformNode();
  void SetAndObserveInputInitialTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetInputChangedTransformNode();
  void SetAndObserveInputChangedTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetInputAnchorTransformNode();
  void SetAndObserveInputAnchorTransformNode( vtkMRMLLinearTransformNode* node );

  vtkMRMLLinearTransformNode* GetOutputTransformNode();
  void SetAndObserveOutputTransformNode( vtkMRMLLinearTransformNode* node );
  
  void ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData );

  vtkGetMacro( UpdatesPerSecond, int ); 
  vtkSetMacro( UpdatesPerSecond, int );

  vtkGetMacro( ProcessingMode, int );
  void SetProcessingMode( int );

  vtkGetMacro( UpdateMode, int );
  void SetUpdateMode( int );
  void SetUpdateModeToAuto() { this->SetUpdateMode( UPDATE_MODE_AUTO ); }
  void SetUpdateModeToManual() { this->SetUpdateMode( UPDATE_MODE_MANUAL ); }

  const bool* GetCopyTranslationComponents();
  
  bool GetCopyTranslationX();
  void SetCopyTranslationX( bool );
  
  bool GetCopyTranslationY();
  void SetCopyTranslationY( bool );
  
  bool GetCopyTranslationZ();
  void SetCopyTranslationZ( bool );

  vtkGetMacro( RotationMode, int );
  void SetRotationMode( int );
  
  vtkGetMacro( DependentAxesMode, int );
  void SetDependentAxesMode( int );
  
  vtkGetMacro( PrimaryAxisLabel, int );
  void SetPrimaryAxisLabel( int );
  
  vtkGetMacro( SecondaryAxisLabel, int );
  void SetSecondaryAxisLabel( int );

  void CheckAndCorrectForDuplicateAxes();

  static std::string GetProcessingModeAsString( int );
  static int GetProcessingModeFromString( std::string );

  static std::string GetUpdateModeAsString( int );
  static int GetUpdateModeFromString( std::string );

  static std::string GetRotationModeAsString( int );
  static int GetRotationModeFromString( std::string );
  
  static std::string GetDependentAxesModeAsString( int );
  static int GetDependentAxesModeFromString( std::string );

  static std::string GetAxisLabelAsString( int );
  static int GetAxisLabelFromString( std::string );

private:
  // common functions internal to the class.
  // convenience functions GetInputXXXTransformNode(), etc... call these
  vtkMRMLLinearTransformNode* GetNthTransformNodeInRole( const char* role, int n );
  void AddAndObserveTransformNodeInRole( const char* role, vtkMRMLLinearTransformNode* node );
  void RemoveNthTransformNodeInRole( const char* role, int n );
  int GetNumberOfTransformNodesInRole( const char* role );

  vtkMRMLLinearTransformNode* GetTransformNodeInRole( const char* role );
  void SetAndObserveTransformNodeInRole( const char* role, vtkMRMLLinearTransformNode* node );

protected:
  vtkMRMLTransformProcessorNode();
  ~vtkMRMLTransformProcessorNode();

  vtkMRMLTransformProcessorNode( const vtkMRMLTransformProcessorNode& );
  void operator=( const vtkMRMLTransformProcessorNode& );

//Parameters
protected:
  int  UpdatesPerSecond;
  int  ProcessingMode;
  int  UpdateMode;
  bool CopyTranslationComponents[ 3 ];
  int  RotationMode;
  int  DependentAxesMode;
  int  PrimaryAxisLabel;
  int  SecondaryAxisLabel;
};

#endif
