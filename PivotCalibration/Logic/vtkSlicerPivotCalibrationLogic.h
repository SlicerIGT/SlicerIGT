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
#include "vtkMRMLLinearTransformNode.h"

// Pivot calibration includes
#include "vtkSlicerPivotCalibrationModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
/// Module for calibrating a tracked pointer/stylus device.
/// Includes pivot calibration for determining tip position and spin calibration for determining orientation.
/// Shaft direction (i.e. flipping) is automatically determined from both spin calibration and pivot calibration (otherwise the flipping is arbitrary).
/// Shaft direction (i.e. flipping) assumes that the tool marker/sensor is on the same side as the tool base.
/// Pivot and spin calibrations may be performed in arbitrary order.
class VTK_SLICER_PIVOTCALIBRATION_MODULE_LOGIC_EXPORT vtkSlicerPivotCalibrationLogic :
  public vtkSlicerModuleLogic
{
public:

  enum Events
  {
    InputTransformAdded = vtkCommand::UserEvent + 173,
    PivotInputTransformAdded,
    SpinInputTransformAdded,
    PivotCalibrationCompleteEvent,
    SpinCalibrationCompleteEvent
  };

  enum CalibrationErrorCodes
  {
    CALIBRATION_FAIL,
    CALIBRATION_SUCCESS,
    CALIBRATION_NOT_STARTED,
    CALIBRATION_NOT_ENOUGH_POINTS,
    CALIBRATION_NOT_ENOUGH_VARIATION,
    CALIBRATION_HIGH_ERROR,
  };

  static vtkSlicerPivotCalibrationLogic* New();
  vtkTypeMacro(vtkSlicerPivotCalibrationLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Clears all previously acquired tool transforms.
  // Call this before start adding transforms.
  void ClearToolToReferenceMatrices();
  void ClearPivotToolToReferenceMatrices();
  void ClearSpinToolToReferenceMatrices();

  // Add a tool transforms automatically by observing transform changes
  vtkGetMacro(RecordingState, bool);
  vtkSetMacro(RecordingState, bool);
  void SetAndObserveTransformNode(vtkMRMLLinearTransformNode*);

  // Add a single tool transform manually
  void AddToolToReferenceMatrix(vtkMatrix4x4*);

  // Computes calibration results.
  // By default, automatically flips the shaft direction to be consistent with the needle orientation protocol.
  // Returns with false on failure
  bool ComputePivotCalibration(bool autoOrient = true);

  // Computes calibration results.
  // By default, automatically flips the shaft direction to be consistent with the needle orientation protocol.
  // Optionally, snaps the rotation to be a 90 degree rotation about one of the coordinate axes.
  // Returns with false on failure
  bool ComputeSpinCalibration(bool snapRotation = false, bool autoOrient = true); // Note: The neede orientation protocol assumes that the shaft of the tool lies along the negative z-axis

  // Flip the direction of the shaft axis
  void FlipShaftDirection();

  // Get calibration results
  void GetToolTipToToolTranslation(vtkMatrix4x4*);
  void GetToolTipToToolRotation(vtkMatrix4x4*);
  void GetToolTipToToolMatrix(vtkMatrix4x4*);
  void SetToolTipToToolMatrix(vtkMatrix4x4*);

  vtkGetMacro(PivotRMSE, double);
  vtkGetMacro(SpinRMSE, double);

  // Returns human-readable description of the error occurred (non-empty if ComputePivotCalibration returns with failure)
  vtkGetMacro(ErrorText, std::string);

  //@{
  /// Returns the number of poses currently cached in the calibration algorithm
  int GetPivotNumberOfPoses();
  int GetSpinNumberOfPoses();
  //@}

  //@{
  /// Returns the current error code in the calibration algorithm
  int GetPivotErrorCode();
  int GetSpinErrorCode();
  //@}

  /// Returns a human-readable string representing the specified error code
  static std::string GetErrorCodeAsString(int code);

  //@{
  /// Flag that specifies if calibration should be automatically performed one the required number of poses has been reached.
  /// If enough poses have been gathered and the error is below the threshold, then PivotCalibrationCompleteEvent or SpinCalibrationCompleteEvent will be invoked.
  vtkGetMacro(PivotAutoCalibrationEnabled, bool);
  void SetPivotAutoCalibrationEnabled(bool);
  vtkBooleanMacro(PivotAutoCalibrationEnabled, bool);
  vtkGetMacro(SpinAutoCalibrationEnabled, bool);
  void SetSpinAutoCalibrationEnabled(bool);
  vtkBooleanMacro(SpinAutoCalibrationEnabled, bool);
  //@}

  //@{
  /// Flag that specifies if calibration is enabled for the specified type.
  /// If on, poses will be added to the calibration algorithm as the transform is modified.
  /// If off, poses will not be added to the calibration algorithm.
  /// Option is on by default.
  vtkGetMacro(PivotCalibrationEnabled, bool);
  vtkSetMacro(PivotCalibrationEnabled, bool);
  vtkBooleanMacro(PivotCalibrationEnabled, bool);
  vtkGetMacro(SpinCalibrationEnabled, bool);
  vtkSetMacro(SpinCalibrationEnabled, bool);
  vtkBooleanMacro(SpinCalibrationEnabled, bool);
  //@}

  //@{
  /// Flag that will specify if recording should be disabled when the desired threshold is reached.
  /// Off by default.
  vtkGetMacro(PivotAutoCalibrationStopWhenComplete, bool);
  vtkSetMacro(PivotAutoCalibrationStopWhenComplete, bool);
  vtkBooleanMacro(PivotAutoCalibrationStopWhenComplete, bool);
  vtkGetMacro(SpinAutoCalibrationStopWhenComplete, bool);
  vtkSetMacro(SpinAutoCalibrationStopWhenComplete, bool);
  vtkBooleanMacro(SpinAutoCalibrationStopWhenComplete, bool);
  //@}

  //@{
  /// The number of points required for automatic calibration.
  vtkGetMacro(PivotAutoCalibrationTargetNumberOfPoints, int);
  vtkSetMacro(PivotAutoCalibrationTargetNumberOfPoints, int);
  vtkGetMacro(SpinAutoCalibrationTargetNumberOfPoints, int);
  vtkSetMacro(SpinAutoCalibrationTargetNumberOfPoints, int);
  //@}

  //@{
  /// The desired target error threshold for automatic calibration.
  vtkGetMacro(PivotAutoCalibrationTargetError, double);
  void SetPivotAutoCalibrationTargetError(double);
  vtkGetMacro(SpinAutoCalibrationTargetError, double);
  void SetSpinAutoCalibrationTargetError(double);
  //@}

  //@{
  /// The number of poses that should be stored in each bucket.
  /// When a bucket is filled, the algorithm will discard all saved poses if the error in the bucket is too high.
  /// This indicates the fact that the tool is not performing the required motion.
  int  GetPivotPoseBucketSize();
  void SetPivotPoseBucketSize(int bucketSize);
  int  GetSpinPoseBucketSize();
  void SetSpinPoseBucketSize(int bucketSize);
  //@}

  //@{
  /// The minimum required pose orientation difference among all input poses.
  /// If the input variation is less than the threshold, then calibration will fail.
  double GetPivotMinimumOrientationDifferenceDegrees();
  void   SetPivotMinimumOrientationDifferenceDegrees(double);
  double GetSpinMinimumOrientationDifferenceDegrees();
  void   SetSpinMinimumOrientationDifferenceDegrees(double);
  //@}

  //@{
  /// The maxmimum number of poses to maintain in the buffer.
  /// If the number of buckets exceeds the maximum, then the oldest will be discarded.
  int  GetPivotMaximumNumberOfPoseBuckets();
  void SetPivotMaximumNumberOfPoseBuckets(int);
  int  GetSpinMaximumNumberOfPoseBuckets();
  void SetSpinMaximumNumberOfPoseBuckets(int);
  //@}

  //@{
  /// The maximum amount of error that is allowed in a pose bucket.
  /// When a bucket is filled, the algorithm will discard all saved poses if the error in the bucket is too high.
  /// This indicates the fact that the tool is not performing the required motion.
  double GetPivotMaximumPoseBucketError();
  void   SetPivotMaximumPoseBucketError(double);
  double GetSpinMaximumPoseBucketError();
  void   SetSpinMaximumPoseBucketError(double);
  //@}

  //@{
  /// The minimum required amount of translation of the input transform from the previous pose.
  /// If the translation is less than the specified amount, then the pose will not be added to the buffer.
  double GetPivotPositionDifferenceThresholdMm();
  void   SetPivotPositionDifferenceThresholdMm(double);
  double GetSpinPositionDifferenceThresholdMm();
  void   SetSpinPositionDifferenceThresholdMm(double);
  //@}

  //@{
  /// The minimum required amount of rotation of the input transform from the previous pose.
  /// If the rotation is less than the specified amount, then the pose will not be added to the buffer.
  double GetPivotOrientationDifferenceThresholdDegrees();
  void   SetPivotOrientationDifferenceThresholdDegrees(double);
  double GetSpinOrientationDifferenceThresholdDegrees();
  void   SetSpinOrientationDifferenceThresholdDegrees(double);
  //@}

protected:
  vtkSlicerPivotCalibrationLogic();
  virtual ~vtkSlicerPivotCalibrationLogic();

  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  vtkSetMacro(PivotRMSE, double);
  vtkSetMacro(SpinRMSE, double);

  class vtkInternal;
  vtkInternal* Internal;

private:

  vtkSlicerPivotCalibrationLogic(const vtkSlicerPivotCalibrationLogic&); // Not implemented
  void operator=(const vtkSlicerPivotCalibrationLogic&);               // Not implemented

  // Calibration inputs
  vtkMRMLLinearTransformNode* ObservedTransformNode{ nullptr };
  bool RecordingState{ false };

  // Calibration results
  vtkMatrix4x4* ToolTipToToolMatrix;
  double PivotRMSE{ -1.0 };
  double SpinRMSE{ -1.0 };
  std::string ErrorText;

  // Pivot/spin enabled flags
  bool   PivotCalibrationEnabled{ true };
  bool   SpinCalibrationEnabled{ true };

  // Pivot auto-calibration settings
  bool   PivotAutoCalibrationEnabled{ false };
  double PivotAutoCalibrationTargetError{ 3.0 };
  int    PivotAutoCalibrationTargetNumberOfPoints{ 100 };
  bool   PivotAutoCalibrationStopWhenComplete{ false };

  // Spin auto-calibration settings
  bool   SpinAutoCalibrationEnabled{ false };
  double SpinAutoCalibrationTargetError{ 0.01 };
  int    SpinAutoCalibrationTargetNumberOfPoints{ 100 };
  bool   SpinAutoCalibrationStopWhenComplete{ false };
};

#endif
