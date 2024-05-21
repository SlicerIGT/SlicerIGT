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

// PivotCalibration Logic includes
#include "vtkSlicerPivotCalibrationLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include "vtkMRMLScene.h"

// vtkIGSIOCalibration includes
#include <vtkIGSIOPivotCalibrationAlgo.h>
#include <vtkIGSIOSpinCalibrationAlgo.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>

//----------------------------------------------------------------------------
class vtkSlicerPivotCalibrationLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerPivotCalibrationLogic* external)
  {
    this->External = external;
  }
  ~vtkInternal() = default;

  vtkSlicerPivotCalibrationLogic* External;
  vtkNew<vtkIGSIOPivotCalibrationAlgo> PivotCalibrationAlgo;
  vtkNew<vtkIGSIOSpinCalibrationAlgo> SpinCalibrationAlgo;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPivotCalibrationLogic);

const int DEFAULT_NUMBER_OF_POSE_BUCKETS = 10;
const int DEFAULT_POSE_BUCKET_SIZE = 10;

const double DEFAULT_PIVOT_POSE_BUCKET_ERROR_MM = 3.0;
const double DEFAULT_PIVOT_INPUT_ORIENTATION_THRESHOLD_DEGREES = 5.0;
const double DEFAULT_PIVOT_INPUT_POSITION_THRESHOLD_MM = 3.0;

const double DEFAULT_SPIN_POSE_BUCKET_ERROR_MM = 0.05;
const double DEFAULT_SPIN_INPUT_ORIENTATION_THRESHOLD_DEGREES = 5.0;
const double DEFAULT_SPIN_INPUT_POSITION_THRESHOLD_MM = 0.0; // No default position threshold for spin. Origin may be on stylus shaft.

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::vtkSlicerPivotCalibrationLogic()
{
  this->Internal = new vtkInternal(this);
  this->ToolTipToToolMatrix = vtkMatrix4x4::New();

  this->Internal->PivotCalibrationAlgo->SetMaximumNumberOfPoseBuckets(DEFAULT_NUMBER_OF_POSE_BUCKETS);
  this->Internal->PivotCalibrationAlgo->SetPoseBucketSize(DEFAULT_POSE_BUCKET_SIZE);
  this->Internal->PivotCalibrationAlgo->SetMaximumPoseBucketError(DEFAULT_PIVOT_POSE_BUCKET_ERROR_MM);
  this->Internal->PivotCalibrationAlgo->SetMaximumCalibrationErrorMm(-1.0);
  this->Internal->PivotCalibrationAlgo->SetOrientationDifferenceThresholdDegrees(DEFAULT_PIVOT_INPUT_ORIENTATION_THRESHOLD_DEGREES);
  this->Internal->PivotCalibrationAlgo->SetPositionDifferenceThresholdMm(DEFAULT_PIVOT_INPUT_POSITION_THRESHOLD_MM);

  this->Internal->SpinCalibrationAlgo->SetMaximumNumberOfPoseBuckets(DEFAULT_NUMBER_OF_POSE_BUCKETS);
  this->Internal->SpinCalibrationAlgo->SetPoseBucketSize(DEFAULT_POSE_BUCKET_SIZE);
  this->Internal->SpinCalibrationAlgo->SetMaximumPoseBucketError(DEFAULT_SPIN_POSE_BUCKET_ERROR_MM);
  this->Internal->SpinCalibrationAlgo->SetMaximumCalibrationErrorMm(-1.0);
  this->Internal->SpinCalibrationAlgo->SetOrientationDifferenceThresholdDegrees(DEFAULT_SPIN_INPUT_ORIENTATION_THRESHOLD_DEGREES);
  this->Internal->SpinCalibrationAlgo->SetPositionDifferenceThresholdMm(DEFAULT_SPIN_INPUT_POSITION_THRESHOLD_MM);

  this->UpdateMaximumCalibrationError();
}

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::~vtkSlicerPivotCalibrationLogic()
{
  delete this->Internal;
  this->ToolTipToToolMatrix->Delete();
  this->SetAndObserveTransformNode(NULL); // Remove the observer
}

//----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  if (caller != NULL)
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
    if (event == vtkMRMLLinearTransformNode::TransformModifiedEvent && this->RecordingState == true && strcmp(transformNode->GetID(), this->ObservedTransformNode->GetID()) == 0)
    {
      vtkNew<vtkMatrix4x4> toolToReferenceMatrix;
      transformNode->GetMatrixTransformToParent(toolToReferenceMatrix);
      this->AddToolToReferenceMatrix(toolToReferenceMatrix);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetAndObserveTransformNode(vtkMRMLLinearTransformNode* transformNode)
{
  if (this->ObservedTransformNode == transformNode)
  {
    return;
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->ObservedTransformNode, transformNode, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::AddToolToReferenceMatrix(vtkMatrix4x4* transformMatrix)
{
  if (!transformMatrix)
  {
    vtkErrorMacro("vtkSlicerPivotCalibrationLogic::AddToolToReferenceMatrix failed: invalid transformMatrix");
    return;
  }

  if (this->PivotCalibrationEnabled)
  {
    this->Internal->PivotCalibrationAlgo->InsertNextCalibrationPoint(transformMatrix);
    this->InvokeEvent(PivotInputTransformAdded);
    if (this->PivotAutoCalibrationEnabled && this->GetPivotNumberOfPoses() >= this->PivotAutoCalibrationTargetNumberOfPoints)
    {
      if (this->ComputePivotCalibration() && this->PivotRMSE <= this->PivotAutoCalibrationTargetError)
      {
        if (this->PivotAutoCalibrationStopWhenComplete)
        {
          // Calibration is complete. Disable pivot calibration.
          this->SetPivotCalibrationEnabled(false);
          if (!this->SpinCalibrationEnabled)
          {
            // If spin calibration is not running, disable recording entirely.
            this->SetRecordingState(false);
          }
        }

        // Calibration completed succesfully.
        this->InvokeEvent(vtkSlicerPivotCalibrationLogic::PivotCalibrationCompleteEvent);
      }
    }
  }

  if (this->SpinCalibrationEnabled)
  {
    this->Internal->SpinCalibrationAlgo->InsertNextCalibrationPoint(transformMatrix);
    this->InvokeEvent(vtkSlicerPivotCalibrationLogic::SpinInputTransformAdded);
    if (this->SpinAutoCalibrationEnabled && this->GetSpinNumberOfPoses() >= this->SpinAutoCalibrationTargetNumberOfPoints)
    {
      if (this->ComputeSpinCalibration() && this->SpinRMSE <= this->SpinAutoCalibrationTargetError)
      {
        if (this->SpinAutoCalibrationStopWhenComplete)
        {
          // Calibration is complete. Disable spin calibration.
          this->SetSpinCalibrationEnabled(false);
          // Calibration is complete. Disable pivot calibration.
          if (!this->PivotCalibrationEnabled)
          {
            // If pivot calibration is not running, disable recording entirely.
            this->SetRecordingState(false);
          }
        }

        // Calibration completed succesfully.
        this->InvokeEvent(vtkSlicerPivotCalibrationLogic::SpinCalibrationCompleteEvent);
      }
    }
  }

  this->InvokeEvent(vtkSlicerPivotCalibrationLogic::InputTransformAdded);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ClearToolToReferenceMatrices()
{
  this->Internal->PivotCalibrationAlgo->RemoveAllCalibrationPoints();
  this->Internal->SpinCalibrationAlgo->RemoveAllCalibrationPoints();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ClearPivotToolToReferenceMatrices()
{
  this->Internal->PivotCalibrationAlgo->RemoveAllCalibrationPoints();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ClearSpinToolToReferenceMatrices()
{
  this->Internal->SpinCalibrationAlgo->RemoveAllCalibrationPoints();
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetPivotErrorCode()
{
  return this->Internal->PivotCalibrationAlgo->GetErrorCode();
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetSpinErrorCode()
{
  return this->Internal->SpinCalibrationAlgo->GetErrorCode();
}

//---------------------------------------------------------------------------
std::string vtkSlicerPivotCalibrationLogic::GetErrorCodeAsString(int errorCode)
{
  std::string errorText;
  switch (errorCode)
  {
  case vtkIGSIOAbstractStylusCalibrationAlgo::CALIBRATION_NO_ERROR:
  case vtkIGSIOAbstractStylusCalibrationAlgo::CALIBRATION_NOT_STARTED:
    break;
  case vtkIGSIOAbstractStylusCalibrationAlgo::CALIBRATION_NOT_ENOUGH_VARIATION:
    errorText = "Couldn't perform calibration; not enough variation";
    break;
  case vtkIGSIOAbstractStylusCalibrationAlgo::CALIBRATION_NOT_ENOUGH_POINTS:
    errorText = "Couldn't perform calibration; not enough points";
    break;
  case vtkIGSIOAbstractStylusCalibrationAlgo::CALIBRATION_HIGH_ERROR:
    errorText = "Couldn't perform calibration; error is too high";
    break;
  default:
  case vtkIGSIOAbstractStylusCalibrationAlgo::CALIBRATION_FAIL:
    errorText = "Couldn't perform calibration";
    break;
  }
  return errorText;
}

//---------------------------------------------------------------------------
bool vtkSlicerPivotCalibrationLogic::ComputePivotCalibration(bool autoOrient /*=true*/)
{
  vtkNew<vtkMatrix4x4> toolTipToToolMatrix;
  toolTipToToolMatrix->DeepCopy(this->ToolTipToToolMatrix);
  this->Internal->PivotCalibrationAlgo->SetPivotPointToMarkerTransformMatrix(toolTipToToolMatrix);

  bool success = this->Internal->PivotCalibrationAlgo->DoPivotCalibration(nullptr, autoOrient) == IGSIO_SUCCESS;
  this->ErrorText = this->GetErrorCodeAsString(this->GetPivotErrorCode());

  if (!success)
  {
    this->SetPivotRMSE(-1.0);
    vtkErrorMacro("ComputePivotCalibration: " << this->GetErrorText());
    return false;
  }

  //set the RMSE
  this->SetPivotRMSE(this->Internal->PivotCalibrationAlgo->GetPivotCalibrationErrorMm());

  //set the transformation
  this->ToolTipToToolMatrix->DeepCopy(
    this->Internal->PivotCalibrationAlgo->GetPivotPointToMarkerTransformMatrix());

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerPivotCalibrationLogic::ComputeSpinCalibration(bool snapRotation /*=false*/, bool autoOrient /*=true*/)
{
  vtkNew<vtkMatrix4x4> toolTipToToolMatrix;
  toolTipToToolMatrix->DeepCopy(this->ToolTipToToolMatrix);
  this->Internal->SpinCalibrationAlgo->SetPivotPointToMarkerTransformMatrix(toolTipToToolMatrix);

  bool success = this->Internal->SpinCalibrationAlgo->DoSpinCalibration(nullptr, snapRotation, autoOrient) == IGSIO_SUCCESS;
  this->ErrorText = this->GetErrorCodeAsString(this->GetSpinErrorCode());

  if (!success)
  {
    this->SetSpinRMSE(-1.0);
    vtkErrorMacro("ComputeSpinCalibration: " << this->GetErrorText());
    return false;
  }

  //set the RMSE
  this->SetSpinRMSE(this->Internal->SpinCalibrationAlgo->GetSpinCalibrationErrorMm());

  //set the transformation
  this->ToolTipToToolMatrix->DeepCopy(
    this->Internal->SpinCalibrationAlgo->GetPivotPointToMarkerTransformMatrix());

  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::GetToolTipToToolTranslation(vtkMatrix4x4* translationMatrix)
{
  translationMatrix->Identity();
  translationMatrix->SetElement(0, 3, this->ToolTipToToolMatrix->GetElement(0, 3));
  translationMatrix->SetElement(1, 3, this->ToolTipToToolMatrix->GetElement(1, 3));
  translationMatrix->SetElement(2, 3, this->ToolTipToToolMatrix->GetElement(2, 3));
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::GetToolTipToToolRotation(vtkMatrix4x4* rotationMatrix)
{
  rotationMatrix->Identity();
  rotationMatrix->SetElement(0, 0, this->ToolTipToToolMatrix->GetElement(0, 0));
  rotationMatrix->SetElement(0, 1, this->ToolTipToToolMatrix->GetElement(0, 1));
  rotationMatrix->SetElement(0, 2, this->ToolTipToToolMatrix->GetElement(0, 2));
  rotationMatrix->SetElement(1, 0, this->ToolTipToToolMatrix->GetElement(1, 0));
  rotationMatrix->SetElement(1, 1, this->ToolTipToToolMatrix->GetElement(1, 1));
  rotationMatrix->SetElement(1, 2, this->ToolTipToToolMatrix->GetElement(1, 2));
  rotationMatrix->SetElement(2, 0, this->ToolTipToToolMatrix->GetElement(2, 0));
  rotationMatrix->SetElement(2, 1, this->ToolTipToToolMatrix->GetElement(2, 1));
  rotationMatrix->SetElement(2, 2, this->ToolTipToToolMatrix->GetElement(2, 2));
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::GetToolTipToToolMatrix(vtkMatrix4x4* matrix)
{
  matrix->DeepCopy(this->ToolTipToToolMatrix);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetToolTipToToolMatrix(vtkMatrix4x4* matrix)
{
  this->ToolTipToToolMatrix->DeepCopy(matrix);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::FlipShaftDirection()
{
  vtkIGSIOAbstractStylusCalibrationAlgo::FlipShaftDirection(this->ToolTipToToolMatrix);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotAutoCalibrationEnabled(bool enabled)
{
  if (this->PivotAutoCalibrationEnabled == enabled)
  {
    return;
  }
  this->PivotAutoCalibrationEnabled = enabled;
  this->Internal->PivotCalibrationAlgo->SetValidateInputBufferEnabled(enabled);
  this->UpdateMaximumCalibrationError();
  this->Modified();
  return;
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinAutoCalibrationEnabled(bool enabled)
{
  if (this->SpinAutoCalibrationEnabled == enabled)
  {
    return;
  }
  this->SpinAutoCalibrationEnabled = enabled;
  this->Internal->SpinCalibrationAlgo->SetValidateInputBufferEnabled(enabled);
  this->UpdateMaximumCalibrationError();
  this->Modified();
  return;
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetPivotNumberOfPoses()
{
  return this->Internal->PivotCalibrationAlgo->GetNumberOfCalibrationPoints();
}

//---------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetPivotMinimumOrientationDifferenceDegrees()
{
  return this->Internal->PivotCalibrationAlgo->GetMinimumOrientationDifferenceDegrees();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotMinimumOrientationDifferenceDegrees(double minimumOrientationDifferenceDegrees)
{
  this->Internal->PivotCalibrationAlgo->SetMinimumOrientationDifferenceDegrees(minimumOrientationDifferenceDegrees);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinAutoCalibrationTargetError(double spinTargetError)
{
  if (this->SpinAutoCalibrationTargetError == spinTargetError)
  {
    return;
  }
  this->SpinAutoCalibrationTargetError = spinTargetError;
  this->UpdateMaximumCalibrationError();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotAutoCalibrationTargetError(double pivotTargetError)
{
  if (this->PivotAutoCalibrationTargetError == pivotTargetError)
  {
    return;
  }
  this->PivotAutoCalibrationTargetError = pivotTargetError;
  this->UpdateMaximumCalibrationError();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::UpdateMaximumCalibrationError()
{
  this->Internal->PivotCalibrationAlgo->SetMaximumCalibrationErrorMm(this->PivotAutoCalibrationEnabled ? this->PivotAutoCalibrationTargetError : -1.0);
  this->Internal->SpinCalibrationAlgo->SetMaximumCalibrationErrorMm(this->SpinAutoCalibrationEnabled ? this->SpinAutoCalibrationTargetError : -1.0);
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetPivotPoseBucketSize()
{
  return this->Internal->PivotCalibrationAlgo->GetPoseBucketSize();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotPoseBucketSize(int bucketSize)
{
  this->Internal->PivotCalibrationAlgo->SetPoseBucketSize(bucketSize);
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetPivotMaximumNumberOfPoseBuckets()
{
  return this->Internal->PivotCalibrationAlgo->GetMaximumNumberOfPoseBuckets();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotMaximumNumberOfPoseBuckets(int bucketSize)
{
  this->Internal->PivotCalibrationAlgo->SetMaximumNumberOfPoseBuckets(bucketSize);
}

//---------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetPivotMaximumPoseBucketError()
{
  return this->Internal->PivotCalibrationAlgo->GetMaximumPoseBucketError();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotMaximumPoseBucketError(double maximumBucketError)
{
  this->Internal->PivotCalibrationAlgo->SetMaximumPoseBucketError(maximumBucketError);
}

//-----------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetPivotPositionDifferenceThresholdMm()
{
  return this->Internal->PivotCalibrationAlgo->GetPositionDifferenceThresholdMm();
}

//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotPositionDifferenceThresholdMm(double thresholdMM)
{
  this->Internal->PivotCalibrationAlgo->SetPositionDifferenceThresholdMm(thresholdMM);
}

//-----------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetPivotOrientationDifferenceThresholdDegrees()
{
  return this->Internal->PivotCalibrationAlgo->GetOrientationDifferenceThresholdDegrees();
}

//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetPivotOrientationDifferenceThresholdDegrees(double thresholdDegrees)
{
  this->Internal->PivotCalibrationAlgo->SetOrientationDifferenceThresholdDegrees(thresholdDegrees);
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetSpinNumberOfPoses()
{
  return this->Internal->SpinCalibrationAlgo->GetNumberOfCalibrationPoints();
}

//---------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetSpinMinimumOrientationDifferenceDegrees()
{
  return this->Internal->SpinCalibrationAlgo->GetMinimumOrientationDifferenceDegrees();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinMinimumOrientationDifferenceDegrees(double minimumOrientationDifferenceDegrees)
{
  this->Internal->SpinCalibrationAlgo->SetMinimumOrientationDifferenceDegrees(minimumOrientationDifferenceDegrees);
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetSpinPoseBucketSize()
{
  return this->Internal->SpinCalibrationAlgo->GetPoseBucketSize();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinPoseBucketSize(int bucketSize)
{
  this->Internal->SpinCalibrationAlgo->SetPoseBucketSize(bucketSize);
}

//---------------------------------------------------------------------------
int vtkSlicerPivotCalibrationLogic::GetSpinMaximumNumberOfPoseBuckets()
{
  return this->Internal->SpinCalibrationAlgo->GetMaximumNumberOfPoseBuckets();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinMaximumNumberOfPoseBuckets(int bucketSize)
{
  this->Internal->SpinCalibrationAlgo->SetMaximumNumberOfPoseBuckets(bucketSize);
}

//---------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetSpinMaximumPoseBucketError()
{
  return this->Internal->SpinCalibrationAlgo->GetMaximumPoseBucketError();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinMaximumPoseBucketError(double maximumBucketError)
{
  this->Internal->SpinCalibrationAlgo->SetMaximumPoseBucketError(maximumBucketError);
}

//-----------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetSpinPositionDifferenceThresholdMm()
{
  return this->Internal->SpinCalibrationAlgo->GetPositionDifferenceThresholdMm();
}

//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinPositionDifferenceThresholdMm(double thresholdMM)
{
  this->Internal->SpinCalibrationAlgo->SetPositionDifferenceThresholdMm(thresholdMM);
}

//-----------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetSpinOrientationDifferenceThresholdDegrees()
{
  return this->Internal->SpinCalibrationAlgo->GetOrientationDifferenceThresholdDegrees();
}

//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetSpinOrientationDifferenceThresholdDegrees(double thresholdDegrees)
{
  this->Internal->SpinCalibrationAlgo->SetOrientationDifferenceThresholdDegrees(thresholdDegrees);
}
