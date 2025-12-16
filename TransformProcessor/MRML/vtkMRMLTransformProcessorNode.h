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
#include <vtkMRMLTransformNode.h>

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
    PROCESSING_MODE_COMPUTE_INVERSE,
    PROCESSING_MODE_STABILIZE,
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
  void PrintSelf( ostream& os, vtkIndent indent ) override;

  virtual vtkMRMLNode* CreateNodeInstance() override;
  virtual void ReadXMLAttributes( const char** atts ) override;
  virtual void WriteXML( ostream& of, int indent ) override;
  virtual void Copy( vtkMRMLNode *node ) override;
  virtual const char* GetNodeTagName() override { return "TransformProcessorParameters"; };

  // begin accessors and mutators
  vtkMRMLTransformNode* GetNthInputCombineTransformNode( int n );
  void AddAndObserveInputCombineTransformNode( vtkMRMLTransformNode* node );
  void RemoveNthInputCombineTransformNode( int n );
  int GetNumberOfInputCombineTransformNodes();

  vtkMRMLTransformNode* GetInputFromTransformNode();
  void SetAndObserveInputFromTransformNode( vtkMRMLTransformNode* node );

  vtkMRMLTransformNode* GetInputToTransformNode();
  void SetAndObserveInputToTransformNode( vtkMRMLTransformNode* node );

  vtkMRMLTransformNode* GetInputInitialTransformNode();
  void SetAndObserveInputInitialTransformNode( vtkMRMLTransformNode* node );

  vtkMRMLTransformNode* GetInputChangedTransformNode();
  void SetAndObserveInputChangedTransformNode( vtkMRMLTransformNode* node );

  vtkMRMLTransformNode* GetInputAnchorTransformNode();
  void SetAndObserveInputAnchorTransformNode( vtkMRMLTransformNode* node );

  vtkMRMLTransformNode* GetInputForwardTransformNode();
  void SetAndObserveInputForwardTransformNode( vtkMRMLTransformNode* node );

  vtkMRMLTransformNode* GetInputUnstabilizedTransformNode();
  void SetAndObserveInputUnstabilizedTransformNode(vtkMRMLTransformNode* node);

  vtkMRMLTransformNode* GetOutputTransformNode();
  void SetAndObserveOutputTransformNode( vtkMRMLTransformNode* node );
  
  void ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData ) override;

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

  vtkGetMacro(StabilizationCutOffFrequency, double);
  void SetStabilizationCutOffFrequency(double);

  vtkGetMacro(StabilizationEnabled, bool);
  void SetStabilizationEnabled(bool);

  void CheckAndCorrectForDuplicateAxes();

  static const char* GetProcessingModeAsString( int );
  static int GetProcessingModeFromString( std::string );

  static const char* GetUpdateModeAsString( int );
  static int GetUpdateModeFromString( std::string );

  static const char* GetRotationModeAsString( int );
  static int GetRotationModeFromString( std::string );
  
  static const char* GetDependentAxesModeAsString( int );
  static int GetDependentAxesModeFromString( std::string );

  static const char* GetAxisLabelAsString( int );
  static int GetAxisLabelFromString( std::string );

  static const char* GetPrimaryAxisLabelAsString(int);
  static int GetPrimaryAxisLabelFromString(std::string);

  static const char* GetSecondaryAxisLabelAsString(int);
  static int GetSecondaryAxisLabelFromString(std::string);

private:
  // common functions internal to the class.
  // convenience functions GetInputXXXTransformNode(), etc... call these
  vtkMRMLTransformNode* GetNthTransformNodeInRole( const char* role, int n );
  void AddAndObserveTransformNodeInRole( const char* role, vtkMRMLTransformNode* node );
  void RemoveNthTransformNodeInRole( const char* role, int n );
  int GetNumberOfTransformNodesInRole( const char* role );

  vtkMRMLTransformNode* GetTransformNodeInRole( const char* role );
  void SetAndObserveTransformNodeInRole( const char* role, vtkMRMLTransformNode* node );

protected:
  vtkMRMLTransformProcessorNode();
  ~vtkMRMLTransformProcessorNode();

  vtkMRMLTransformProcessorNode( const vtkMRMLTransformProcessorNode& );
  void operator=( const vtkMRMLTransformProcessorNode& );

//Parameters
protected:
  int ProcessingMode;
  int UpdateMode;
  bool CopyTranslationComponents[ 3 ];
  int RotationMode;
  int DependentAxesMode;
  int PrimaryAxisLabel;
  int SecondaryAxisLabel;
  double StabilizationCutOffFrequency;
  bool StabilizationEnabled;
};

#endif
