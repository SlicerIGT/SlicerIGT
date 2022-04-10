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

// .NAME vtkSlicerTransformProcessorLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes
#ifndef __vtkSlicerTransformProcessorLogic_h
#define __vtkSlicerTransformProcessorLogic_h

#include <string>
#include <deque>

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

class vtkMRMLTransformProcessorNode;
class vtkMRMLLinearTransformNode;


// STD includes
#include <cstdlib>

// vtk includes
#include "vtkGeneralTransform.h"
#include "vtkTransform.h"
#include "vtkSmartPointer.h"

#include "vtkSlicerTransformProcessorModuleLogicExport.h"


/// \ingroup Slicer_QtModules_TransformProcessor
class VTK_SLICER_TRANSFORMPROCESSOR_MODULE_LOGIC_EXPORT vtkSlicerTransformProcessorLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerTransformProcessorLogic *New();
  vtkTypeMacro( vtkSlicerTransformProcessorLogic, vtkSlicerModuleLogic );
  void PrintSelf( ostream& os, vtkIndent indent ) override;
  
public:
  // Update all output transforms that are to be updated continuously.
  // This method is called regularly by the application.
  void UpdateAllOutputs();

  void UpdateOutputTransform( vtkMRMLTransformProcessorNode* );
  void QuaternionAverage( vtkMRMLTransformProcessorNode* );
  void ComputeShaftPivotTransform( vtkMRMLTransformProcessorNode* );
  void ComputeRotation( vtkMRMLTransformProcessorNode* );
  void ComputeTranslation( vtkMRMLTransformProcessorNode* );
  void ComputeFullTransform( vtkMRMLTransformProcessorNode* );
  void ComputeInverseTransform( vtkMRMLTransformProcessorNode* );
  void ComputeStabilizedTransform(vtkMRMLTransformProcessorNode*);
  bool IsTransformProcessingPossible( vtkMRMLTransformProcessorNode*, bool verbose = false );

  static void GetRotationAllAxesFromTransform ( vtkGeneralTransform*, vtkTransform* );
  static void GetRotationSingleAxisWithPivotFromTransform( vtkGeneralTransform*, const double*, vtkTransform* );
  static void GetRotationSingleAxisWithSecondaryFromTransform( vtkGeneralTransform*, const double*, const double*, vtkTransform* );
  static void GetTranslationOnlyFromTransform( vtkGeneralTransform*, const bool*, vtkTransform* );
  static void GetRotationMatrixFromAxes( const double*, const double*, const double*, vtkMatrix4x4* );
  
protected:
  vtkSlicerTransformProcessorLogic();
  ~vtkSlicerTransformProcessorLogic();

  virtual void RegisterNodes() override;
  virtual void SetMRMLSceneInternal( vtkMRMLScene * newScene ) override;
  virtual void OnMRMLSceneNodeAdded( vtkMRMLNode* node ) override;
  virtual void OnMRMLSceneNodeRemoved( vtkMRMLNode* node ) override;
  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData ) override;
  
private:
  vtkSlicerTransformProcessorLogic( const vtkSlicerTransformProcessorLogic& );// Not implemented
  void operator=( const vtkSlicerTransformProcessorLogic& );// Not implemented
  
  // these helper functions should only be used by processing modes themselves, and are therefore private
  void GetRotationOnlyFromTransform( vtkGeneralTransform*, int, int, const double*, const double*, vtkTransform* );
  void GetRotationSingleAxisFromTransform( vtkGeneralTransform*, int, const double*, const double*, vtkTransform* );

  void UpdateContinuouslyUpdatedNodesList(vtkMRMLTransformProcessorNode* paramNode);

  void Slerp(double* result, double t, double* from, double* to, bool adjustSign = true);
  void GetInterpolatedTransform(vtkMatrix4x4* itemAmatrix, vtkMatrix4x4* itemBmatrix,
    double itemAweight, double itemBweight,
    vtkMatrix4x4* interpolatedMatrix);

  std::deque< vtkWeakPointer<vtkMRMLTransformProcessorNode> > ContinuouslyUpdatedNodes;

};

#endif

