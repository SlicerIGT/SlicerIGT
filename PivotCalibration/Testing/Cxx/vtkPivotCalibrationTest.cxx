/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// SlicerIGT includes
#include <vtkSlicerPivotCalibrationLogic.h>

// Slicer MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkTransform.h>

int NUMBER_OF_POINTS = 100;
double epsilon = 1.0e-6;

//----------------------------------------------------------------------------
bool TestPivotCalibration(vtkSlicerPivotCalibrationLogic* logic, vtkMRMLTransformNode* markerToReferenceTransform, double positionErrorMm=0.0)
{
  std::cout << "=================================================================" << std::endl;
  std::cout << "Starting pivot calibration test..." << std::endl;
  std::cout << "Position error: " << positionErrorMm << " mm" << std::endl;

  logic->ClearToolToReferenceMatrices();

  double expectedToolTipPosition_Marker[3] = { 5.0, 12.6, 3.3 };

  vtkNew<vtkTransform> startTransform;
  startTransform->Translate(-expectedToolTipPosition_Marker[0], -expectedToolTipPosition_Marker[1], -expectedToolTipPosition_Marker[2]);
  markerToReferenceTransform->SetAndObserveTransformToParent(startTransform);

  vtkNew<vtkMinimalStandardRandomSequence> randomSequence;
  randomSequence->SetSeed(12345);

  logic->SetRecordingState(true);
  for (int i = 0; i < NUMBER_OF_POINTS; ++i)
  {
    double errorOffset[3] = { 0.0, 0.0, 0.0 };
    if (positionErrorMm > 0.0)
    {
      errorOffset[0] = (2.0 * randomSequence->GetNextValue() - 1.0) * positionErrorMm;
      errorOffset[1] = (2.0 * randomSequence->GetNextValue() - 1.0) * positionErrorMm;
      errorOffset[2] = (2.0 * randomSequence->GetNextValue() - 1.0) * positionErrorMm;
    }

    vtkNew<vtkTransform> transform;
    transform->DeepCopy(startTransform);
    transform->Translate(expectedToolTipPosition_Marker);
    transform->RotateX(double(i) / NUMBER_OF_POINTS * 90.0);
    transform->RotateY(double(i) / NUMBER_OF_POINTS * 90.0);
    transform->RotateZ(double(i) / NUMBER_OF_POINTS * 90.0);
    transform->Translate(errorOffset[0] - expectedToolTipPosition_Marker[0], errorOffset[1] - expectedToolTipPosition_Marker[1], errorOffset[2] - expectedToolTipPosition_Marker[2]);
    markerToReferenceTransform->SetAndObserveTransformToParent(transform);
  }
  logic->SetRecordingState(false);

  if (!logic->ComputePivotCalibration())
  {
    std::cerr << "Could not compute pivot calibration: " << logic->GetErrorText() << std::endl;
    return false;
  }

  if (logic->GetPivotRMSE() >= epsilon && positionErrorMm == 0.0)
  {
    std::cerr << "Pivot calibration error is too large: " << logic->GetPivotRMSE() << std::endl;
    return false;
  }

  vtkNew<vtkMatrix4x4> toolTipToToolMatrix;
  logic->GetToolTipToToolMatrix(toolTipToToolMatrix);

  vtkNew<vtkTransform> toolTipToToolTransform;
  toolTipToToolTransform->Concatenate(toolTipToToolMatrix);

  double* actualToolTipPosition_Marker = toolTipToToolTransform->TransformPoint(0.0, 0.0, 0.0);
  double distanceBetweenActualAndExpectedToolTipPosition =
    std::sqrt(vtkMath::Distance2BetweenPoints(actualToolTipPosition_Marker, expectedToolTipPosition_Marker));

  std::cerr << "Expected: { " << expectedToolTipPosition_Marker[0] << ", " << expectedToolTipPosition_Marker[1] << ", " << expectedToolTipPosition_Marker[1] << " }" << std::endl;
  std::cerr << "Actual: { " << actualToolTipPosition_Marker[0] << ", " << actualToolTipPosition_Marker[1] << ", " << actualToolTipPosition_Marker[1] << " }" << std::endl;
  std::cout << "Position error: " << distanceBetweenActualAndExpectedToolTipPosition << " mm" << std::endl;

  if (distanceBetweenActualAndExpectedToolTipPosition >= epsilon && positionErrorMm == 0.0)
  {
    std::cerr << "Tool tip position error is larger than expected" << std::endl;
    return false;
  }

  std::cout << "Pivot calibration completed successfully." << std::endl;
  return true;
}

//----------------------------------------------------------------------------
bool TestSpinCalibration(vtkSlicerPivotCalibrationLogic* logic, vtkMRMLTransformNode* markerToReferenceTransform)
{
  std::cout << "=================================================================" << std::endl;
  std::cout << "Starting spin calibration test..." << std::endl;

  logic->ClearToolToReferenceMatrices();

  double expectedToolTipPosition_Marker[3] = { 5.0, 12.6, 3.3 };
  double expectedToolShaftDirection_Marker[3] = { 1.0, 2.5, 4.1 };
  vtkMath::Normalize(expectedToolShaftDirection_Marker);

  vtkNew<vtkTransform> startTransform;
  startTransform->Translate(-expectedToolTipPosition_Marker[0], -expectedToolTipPosition_Marker[1], -expectedToolTipPosition_Marker[2]);
  markerToReferenceTransform->SetAndObserveTransformToParent(startTransform);

  logic->SetRecordingState(true);
  double angleStepSize = 360.0 / NUMBER_OF_POINTS;
  for (int i = 0; i < NUMBER_OF_POINTS; ++i)
  {
    vtkNew<vtkTransform> transform;
    transform->DeepCopy(markerToReferenceTransform->GetTransformToParent());
    transform->Translate(expectedToolTipPosition_Marker);
    transform->RotateWXYZ(angleStepSize, expectedToolShaftDirection_Marker);
    transform->Translate(-expectedToolTipPosition_Marker[0], -expectedToolTipPosition_Marker[1], -expectedToolTipPosition_Marker[2]);
    markerToReferenceTransform->SetAndObserveTransformToParent(transform);
  }
  logic->SetRecordingState(false);

  if (!logic->ComputeSpinCalibration())
  {
    std::cerr << "Could not compute spin calibration: " << logic->GetErrorText() << std::endl;
    return false;
  }

  if (logic->GetSpinRMSE() >= epsilon)
  {
    std::cerr << "Spin calibration error is too large: " << logic->GetSpinRMSE() << std::endl;
    return false;
  }

  vtkNew<vtkMatrix4x4> toolTipToToolMatrix;
  logic->GetToolTipToToolMatrix(toolTipToToolMatrix);

  vtkNew<vtkTransform> toolTipToToolTransform;
  toolTipToToolTransform->Concatenate(toolTipToToolMatrix);

  double* actualToolShaftDirection_Marker = toolTipToToolTransform->TransformVector(0.0, 0.0, 1.0);
  if (vtkMath::Dot(actualToolShaftDirection_Marker, expectedToolShaftDirection_Marker) < -1.0 + epsilon)
  {
    // Flip tool shaft.
    vtkMath::MultiplyScalar(actualToolShaftDirection_Marker, -1.0);
  }

  double angleBetweenActualAndExpectedToolShaftDirection_Marker =
    vtkMath::AngleBetweenVectors(actualToolShaftDirection_Marker, expectedToolShaftDirection_Marker);

  std::cerr << "Expected: { " << expectedToolShaftDirection_Marker[0] << ", " << expectedToolShaftDirection_Marker[1] << ", " << expectedToolShaftDirection_Marker[1] << " }" << std::endl;
  std::cerr << "Actual: { " << actualToolShaftDirection_Marker[0] << ", " << actualToolShaftDirection_Marker[1] << ", " << actualToolShaftDirection_Marker[1] << " }" << std::endl;
  std::cout << "Angle error: " << angleBetweenActualAndExpectedToolShaftDirection_Marker << " degrees" << std::endl;

  if (angleBetweenActualAndExpectedToolShaftDirection_Marker >= epsilon)
  {
    std::cerr << "Tool tool shaft direction different than expected" << std::endl;
    return false;
  }

  std::cout << "Spin calibration completed successfully." << std::endl;
  return true;
}

//----------------------------------------------------------------------------
int vtkPivotCalibrationTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLTransformNode> markerToReferenceTransform;
  scene->AddNode(markerToReferenceTransform);

  vtkNew<vtkSlicerPivotCalibrationLogic> logic;
  logic->SetMRMLScene(scene);
  logic->SetAndObserveTransformNode(markerToReferenceTransform);

  double pivotErrorsMm[4] = { 0.0, 0.1, 1.0, 10.0 };
  for (double pivotErrorMm : pivotErrorsMm)
  {
    if (!TestPivotCalibration(logic, markerToReferenceTransform, pivotErrorMm))
    {
      return EXIT_FAILURE;
    }
  }

  if (!TestSpinCalibration(logic, markerToReferenceTransform))
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
