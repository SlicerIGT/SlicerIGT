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
#include <vtkMath.h>

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// STD includes
#include <cstdlib>

// VNL includes
#include "vnl/vnl_matrix.h"

#include "vtkSlicerPivotCalibrationModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
/// Module for calibrating a tracked pointer/stylus device.
/// Includes pivot calibration for determining tip position and spin calibration for determining orientation.
/// Shaft direction is determined from both spin calibration and pivot calibration.
/// Pivot and spin calibrations may be performed in arbitrary order.
class VTK_SLICER_PIVOTCALIBRATION_MODULE_LOGIC_EXPORT vtkSlicerPivotCalibrationLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerPivotCalibrationLogic *New();
  vtkTypeMacro(vtkSlicerPivotCalibrationLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Clears all previously acquired tool transforms.
  // Call this before start adding transforms.
  void ClearToolToReferenceMatrices();

  // Add a tool transforms automatically by observing transform changes
  vtkGetMacro(RecordingState, bool);
  vtkSetMacro(RecordingState, bool);
  void SetAndObserveTransformNode( vtkMRMLLinearTransformNode* );

  // Add a single tool transform manually
  void AddToolToReferenceMatrix( vtkMatrix4x4* );

  // Computes calibration results.
  // Returns with false on failure
  bool ComputePivotCalibration();

  // Computes calibration results.
  // Returns with false on failure
  bool ComputeSpinCalibration( bool snapRotation = false ); // Note: The neede orientation protocol assumes that the shaft of the tool lies along the negative z-axis

  // Flip the direction of the shaft axis
  void FlipShaftDirection();

  // Get calibration results
  void GetToolTipToToolTranslation( vtkMatrix4x4* );
  void GetToolTipToToolRotation( vtkMatrix4x4* );
  void GetToolTipToToolMatrix( vtkMatrix4x4* );
  void SetToolTipToToolMatrix( vtkMatrix4x4* );
  vtkGetMacro(PivotRMSE, double);
  vtkGetMacro(SpinRMSE, double);

  // Returns human-readable description of the error occurred (non-empty if ComputePivotCalibration returns with failure)
  vtkGetMacro(ErrorText, std::string);
  
protected:
  vtkSlicerPivotCalibrationLogic();
  virtual ~vtkSlicerPivotCalibrationLogic();
  
  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );

  // Returns the orientation difference in degrees between two 4x4 homogeneous transformation matrix, in degrees.
  double GetOrientationDifferenceDeg(vtkMatrix4x4* aMatrix, vtkMatrix4x4* bMatrix);
  
  // Computes the maximum orientation difference in degrees between the first tool transformation
  // and all the others. Used for determining if there was enough variation in the input data.
  double GetMaximumToolOrientationDifferenceDeg();

  // Verify whether the tool's shaft is in the same direction as the ToolTip to Tool vector.
  // Rotate the ToolTip coordinate frame by 180 degrees about the secondary axis to make the 
  // shaft in the same direction as the ToolTip to Tool vector, if this is not already the case.
  void UpdateShaftDirection();

  // Helper method to compute the secondary axis, given a shaft axis
  static vnl_vector< double > ComputeSecondaryAxis( vnl_vector< double > shaftAxis_ToolTip );

  
private:

  vtkSlicerPivotCalibrationLogic(const vtkSlicerPivotCalibrationLogic&); // Not implemented
  void operator=(const vtkSlicerPivotCalibrationLogic&);               // Not implemented

  // Calibration inputs
  double MinimumOrientationDifferenceDeg;
  std::vector< vtkMatrix4x4* > ToolToReferenceMatrices;
  vtkMRMLLinearTransformNode* ObservedTransformNode;
  bool RecordingState;

  // Calibration results
  vtkMatrix4x4* ToolTipToToolMatrix;
  double PivotRMSE;
  double SpinRMSE; 
  std::string ErrorText;
};

#endif
