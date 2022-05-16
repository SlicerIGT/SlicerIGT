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
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// SlicerIGT includes
#include <vtkSlicerLandmarkDetectionLogic.h>

// Slicer MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>

#include <vtkMRMLLandmarkDetectionNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>

// VTK includes
#include <vtkTransform.h>

int NUMBER_OF_POINTS = 100;
double epsilon = 1.0e-6;


//----------------------------------------------------------------------------
int TestLandmarkDetection(vtkMRMLLandmarkDetectionNode* landmarkDetectionNode, vtkMRMLLinearTransformNode* markerToReferenceTransform, double landmarkPosition_RAS[3])
{
  for (int i = 0; i < NUMBER_OF_POINTS; ++i)
  {
    vtkNew<vtkTransform> stylusPosition;
    stylusPosition->PostMultiply();
    stylusPosition->RotateX(vtkMath::Random() * 360.0);
    stylusPosition->RotateY(vtkMath::Random() * 360.0);
    stylusPosition->RotateZ(vtkMath::Random() * 360.0);
    stylusPosition->Translate(landmarkPosition_RAS);
    markerToReferenceTransform->SetMatrixTransformToParent(stylusPosition->GetMatrix());
  }  
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestControlPointPosition(vtkMRMLMarkupsFiducialNode* landmarkNode, int pointIndex, double expectedPosition_RAS[3])
{
  double actualPosition_RAS[3] = { 0.0, 0.0, 0.0 };
  landmarkNode->GetNthControlPointPositionWorld(pointIndex, actualPosition_RAS);
  for (int i = 0; i < 3; ++i)
  {
    CHECK_DOUBLE_TOLERANCE(actualPosition_RAS[i], expectedPosition_RAS[i], epsilon);
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int vtkLandmarkDetectionTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkNew<vtkMRMLMarkupsFiducialNode>());

  vtkNew<vtkSlicerLandmarkDetectionLogic> logic;
  logic->SetMRMLScene(scene);

  {
    vtkMRMLLinearTransformNode* markerToReferenceTransform = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
    vtkMRMLMarkupsFiducialNode* landmarkNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
    vtkMRMLLandmarkDetectionNode* landmarkDetectionNode = vtkMRMLLandmarkDetectionNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLandmarkDetectionNode"));
    landmarkDetectionNode->SetAndObserveInputTransformNode(markerToReferenceTransform);
    landmarkDetectionNode->SetAndObserveOutputMarkupsNode(landmarkNode);
    landmarkDetectionNode->LandmarkDetectionInProgressOn();

    // Test initial landmark placement
    double landmark0_RAS[3] = { 100.0, 50.0, 20.0 };
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, landmark0_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), 1);
    CHECK_EXIT_SUCCESS(TestControlPointPosition(landmarkNode, 0, landmark0_RAS));

    // Test invalid landmark placement due to proximity to previous position
    landmarkDetectionNode->SetMinimumDistanceBetweenLandmarksMm(13.0);
    double landmark1_RAS[3] = { 104.0, 50.0, 20.0 };
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, landmark1_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), 1);

    // Test valid landmark placement with reduced proximity threshold
    landmarkDetectionNode->SetMinimumDistanceBetweenLandmarksMm(4.0);
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, landmark1_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), 2);
    CHECK_EXIT_SUCCESS(TestControlPointPosition(landmarkNode, 1, landmark1_RAS));

    // Test invalid landmark placement due to maximum number of landmarks placed
    landmarkNode->SetMaximumNumberOfControlPoints(2);
    double landmark2_RAS[3] = { 100.0, 250.0, 30.0 };
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, landmark2_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), 2);
    landmarkDetectionNode->LandmarkDetectionInProgressOff();
  }

  {
    vtkMRMLLinearTransformNode* markerToReferenceTransform = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
    vtkMRMLMarkupsFiducialNode* landmarkNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
    vtkMRMLLandmarkDetectionNode* landmarkDetectionNode = vtkMRMLLandmarkDetectionNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLandmarkDetectionNode"));
    landmarkDetectionNode->SetAndObserveInputTransformNode(markerToReferenceTransform);
    landmarkDetectionNode->SetAndObserveOutputMarkupsNode(landmarkNode);
    landmarkDetectionNode->LandmarkDetectionInProgressOn();

    std::string point0Label = "LandmarkPoint0";
    std::string point1Label = "LandmarkPoint1";
    std::string point2Label = "LandmarkPoint2";
    std::string point3Label = "LandmarkPoint3";

    std::vector<std::string> pointLabels;
    pointLabels.push_back(point0Label);
    pointLabels.push_back(point1Label);
    pointLabels.push_back(point2Label);
    pointLabels.push_back(point3Label);

    double point0_RAS[3] = {   0.0,   0.0,   0.0 };
    double point1_RAS[3] = { 100.0,   0.0,   0.0 };
    double point2_RAS[3] = {   0.0, 100.0,   0.0 };
    double point3_RAS[3] = {   0.0,   0.0, 100.0 };

    std::map<std::string, vtkVector3d> labelledPoints_RAS;
    labelledPoints_RAS["LandmarkPoint0"] = vtkVector3d(point0_RAS);
    labelledPoints_RAS["LandmarkPoint1"] = vtkVector3d(point1_RAS);
    labelledPoints_RAS["LandmarkPoint2"] = vtkVector3d(point2_RAS);
    labelledPoints_RAS["LandmarkPoint3"] = vtkVector3d(point3_RAS);

    for (int i = 0; i < pointLabels.size(); ++i)
    {
      // Initialize unplaced control points at center
      landmarkNode->AddControlPoint(0, 0, 0, pointLabels[i]);
      landmarkNode->SetNthControlPointPosition(i, 0.0, 0.0, 0.0, vtkMRMLMarkupsFiducialNode::PositionUndefined);
    }
    landmarkNode->SetMaximumNumberOfControlPoints(4);
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), pointLabels.size());

    // Test valid placement of landmark out of order (3rd first)
    landmarkDetectionNode->SetNextLandmarkLabel(point3Label.c_str());
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, point3_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), pointLabels.size());
    CHECK_INT(landmarkNode->GetNumberOfDefinedControlPoints(), 1);
    CHECK_EXIT_SUCCESS(TestControlPointPosition(landmarkNode, 3, point3_RAS));

    // Test valid placement of landmark out of order (0th second)
    landmarkDetectionNode->SetNextLandmarkLabel(point0Label.c_str());
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, point0_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), pointLabels.size());
    CHECK_INT(landmarkNode->GetNumberOfDefinedControlPoints(), 2);
    CHECK_EXIT_SUCCESS(TestControlPointPosition(landmarkNode, 0, point0_RAS));

    // Test valid placement of landmark out of order (1st third)
    landmarkDetectionNode->SetNextLandmarkLabel(point1Label.c_str());
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, point1_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), pointLabels.size());
    CHECK_INT(landmarkNode->GetNumberOfDefinedControlPoints(), 3);
    CHECK_EXIT_SUCCESS(TestControlPointPosition(landmarkNode, 1, point1_RAS));

    // Test valid placement of landmark out of order (2nd fourth)
    landmarkDetectionNode->SetNextLandmarkLabel(point2Label.c_str());
    CHECK_EXIT_SUCCESS(TestLandmarkDetection(landmarkDetectionNode, markerToReferenceTransform, point2_RAS));
    CHECK_INT(landmarkNode->GetNumberOfControlPoints(), pointLabels.size());
    CHECK_INT(landmarkNode->GetNumberOfDefinedControlPoints(), 4);
    CHECK_EXIT_SUCCESS(TestControlPointPosition(landmarkNode, 2, point2_RAS));
  }

  return EXIT_SUCCESS;
}
