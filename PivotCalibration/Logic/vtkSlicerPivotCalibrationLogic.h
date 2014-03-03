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

==============================================================================*/

// .NAME vtkSlicerPivotCalibrationLogic
// .SECTION Description
#ifndef __vtkSlicerPivotCalibrationLogic_h
#define __vtkSlicerPivotCalibrationLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// VTK includes
#include <vtkMatrix4x4.h>

// MRML includes
#include "vtkMRMLSliceNode.h"
#include "qMRMLSliceWidget.h"

// STD includes
#include <cstdlib>

// VNL includes
#include "vnl/vnl_matrix.h"

#include "vtkSlicerPivotCalibrationModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_PIVOTCALIBRATION_MODULE_LOGIC_EXPORT vtkSlicerPivotCalibrationLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPivotCalibrationLogic *New();
  vtkTypeMacro(vtkSlicerPivotCalibrationLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void initializeViewer(qMRMLSliceWidget* widget);
  void InitializeObserver(vtkMRMLNode*);
  void AddSample(vtkMatrix4x4*);
  void ComputePivotCalibration();
  void ComputeSpinCalibration();
  
  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );
  
  //Move to protected, add accessors
  double PivotPosition[3];
  double Translation[3];
  double RMSE;
  vnl_matrix<double> Rotation;
  
  void setRecordingState(bool);
  
  void ClearSamples();
  
  
protected:
  vtkSlicerPivotCalibrationLogic();
  virtual ~vtkSlicerPivotCalibrationLogic();

  std::vector<vtkMatrix4x4*> transforms;
  
  const char* transformId;
  
  bool recordingState;
  
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  /*
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneStartClose();
  virtual void OnMRMLSceneEndClose();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  */
  
private:

  vtkSlicerPivotCalibrationLogic(const vtkSlicerPivotCalibrationLogic&); // Not implemented
  void operator=(const vtkSlicerPivotCalibrationLogic&);               // Not implemented
};

#endif
