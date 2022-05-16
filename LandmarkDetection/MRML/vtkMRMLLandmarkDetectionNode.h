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

#ifndef __vtkMRMLLandmarkDetectionNode_h
#define __vtkMRMLLandmarkDetectionNode_h

// MRML includes
#include <vtkMRMLNode.h>

// vtkAddon includes
#include <vtkAddonSetGet.h>

// Landmark Detection includes
#include "vtkSlicerLandmarkDetectionModuleMRMLExport.h"

class vtkMRMLMarkupsFiducialNode;
class vtkMRMLTransformNode;

/// \ingroup LandmarkDetection
/// \brief Parameter set node for landmark detection
///
class VTK_SLICER_LANDMARKDETECTION_MODULE_MRML_EXPORT vtkMRMLLandmarkDetectionNode : public vtkMRMLNode
{
public:

  static vtkMRMLLandmarkDetectionNode* New();
  vtkTypeMacro(vtkMRMLLandmarkDetectionNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Standard MRML node methods
  virtual vtkMRMLNode* CreateNodeInstance() override;
  virtual void ReadXMLAttributes(const char** atts) override;
  virtual void WriteXML(ostream& of, int indent) override;
  virtual void Copy(vtkMRMLNode* node) override;
  virtual const char* GetNodeTagName() override { return "LandmarkDetection"; }
  void ProcessMRMLEvents(vtkObject* caller, unsigned long eventID, void* callData) override;

protected:
  vtkMRMLLandmarkDetectionNode();
  virtual ~vtkMRMLLandmarkDetectionNode();
  vtkMRMLLandmarkDetectionNode(const vtkMRMLLandmarkDetectionNode&);
  void operator=(const vtkMRMLLandmarkDetectionNode&);

public:

  enum
  {
    LandmarkDetectionStartedEvent = 67000,
    LandmarkDetectionFinishedEvent,
    InputTransformModifiedEvent,
    LandmarkDetectedEvent,
    ResetDetectionEvent,
  };

  //@{
  /// InputTransformNode is the transform node that is used for landmark detection
  const char* GetInputTransformNodeReferenceRole() { return "inputTransformNode"; };
  const char* GetInputTransformNodeReferenceMRMLAttributeName() { return "inputTransformNodeRef"; };
  vtkMRMLTransformNode* GetInputTransformNode();
  virtual void SetAndObserveInputTransformNode(vtkMRMLTransformNode* transformNode);
  //@}

  //@{
  /// OutputMarkupsNode is the transform node that is used for landmark detection
  const char* GetOutputMarkupsNodeReferenceRole() { return "outputMarkupsNode"; };
  const char* GetOutputMarkupsNodeReferenceMRMLAttributeName() { return "outputMarkupsNodeRef"; };
  vtkMRMLMarkupsFiducialNode* GetOutputMarkupsNode();
  virtual void SetAndObserveOutputMarkupsNode(vtkMRMLMarkupsFiducialNode* transformNode);
  //@}

  //@{
  /// Output coordinate system transform node that is used for landmark detection
  const char* GetOutputCoordinateTransformNodeReferenceRole() { return "outputCoordinateNode"; };
  const char* GetOutputCoordinateTransformNodeReferenceMRMLAttributeName() { return "outputCoordinateNodeRef"; };
  vtkMRMLTransformNode* GetOutputCoordinateTransformNode();
  virtual void SetAndObserveOutputCoordinateTransformNode(vtkMRMLTransformNode* transformNode);
  //@}

  /// Reset the detected landmark points and collected samples.
  virtual void ResetDetection();

  //@{
  /// The state of landmark detection.
  /// True if landmark detection is currently in progress, false otherwise.
  vtkSetMacro(LandmarkDetectionInProgress, bool);
  vtkGetMacro(LandmarkDetectionInProgress, bool);
  vtkBooleanMacro(LandmarkDetectionInProgress, bool);
  //@}

  //@{
  /// Flag to control whether the markups parent transform should be used as the output landmark coordinate frame.
  /// On by default.
  vtkSetMacro(UseMarkupsCoordinatesForOutput, bool);
  vtkGetMacro(UseMarkupsCoordinatesForOutput, bool);
  vtkBooleanMacro(UseMarkupsCoordinatesForOutput, bool);
  //@}
  

  //@{
  /// Device acquisition rate([samples/sec]).
  /// This is required to determine the minimum number of stylus tip poses required for detecting a point.
  /// Default value is 20[samples/sec].
  vtkSetMacro(AcquisitionRateHz, double);
  vtkGetMacro(AcquisitionRateHz, double);
  //@}

  //@{
  /// Device acquisition rate([samples / sec]).
  /// size of the time window used for sample averaging(default 0.2[s]).
  /// Higher value makes the algorithm more robust(by tolerating more outliers) but may become less accurate.
  /// Default value is 0.2[s].
  vtkSetMacro(FilterWindowTimeSec, double);
  vtkGetMacro(FilterWindowTimeSec, double);
  //@}

  //@{
  /// The minimum time the stylus tip has to be at a fixed position to detect a landmark point(default 1[s]).
  /// Default value is 1.0[s].
  vtkSetMacro(DetectionTimeSec, double);
  vtkGetMacro(DetectionTimeSec, double);
  //@}

  //@{
  /// Set the stylus shaft(a point 10 cm above the stylus tip) threshold is used to decide if the stylus is pivoting or static.
  /// During the detection time a point in the stylus shaft updates a bounding box.
  /// The stylus position is consider for landmark detection if the box's lengths norm is bigger than StylusShaftMinimumDisplacementThresholdMm.
  /// Default value is 30.0[mm].
  vtkSetMacro(StylusShaftMinimumDisplacementThresholdMm, double);
  vtkGetMacro(StylusShaftMinimumDisplacementThresholdMm, double);
  //@}

  //@{
  /// A landmark position will be detected if the filtered stylus tip position moves during detection time inside a bounding box
  /// with lengths norm smaller than StylusTipMaximumMotionThresholdMm.
  /// Default value is 1.5[mm].
  vtkSetMacro(StylusTipMaximumDisplacementThresholdMm, double);
  vtkGetMacro(StylusTipMaximumDisplacementThresholdMm, double);
  //@}

  //@{
  /// The minimum distance in between any two defined landmarks.
  /// New landmarks will only be detected if it is further away from any other already detected landmark.
  /// Default value is 15.0[mm].
  vtkSetMacro(MinimumDistanceBetweenLandmarksMm, double);
  vtkGetMacro(MinimumDistanceBetweenLandmarksMm, double);
  //@}

  //@{
  /// The id of the next landmark that should be placed
  vtkSetStringMacro(NextLandmarkLabel);
  vtkGetStringMacro(NextLandmarkLabel);
  //@}

protected:
  bool LandmarkDetectionInProgress{ false };

  bool UseMarkupsCoordinatesForOutput{ true };

  double AcquisitionRateHz{ 20.0 };
  double FilterWindowTimeSec{ 0.2 };
  double DetectionTimeSec{ 1.0 };
  double StylusShaftMinimumDisplacementThresholdMm{ 30.0 };
  double StylusTipMaximumDisplacementThresholdMm{ 1.5 };
  double MinimumDistanceBetweenLandmarksMm{ 15.0 };

  char* NextLandmarkLabel{ nullptr };
};

#endif // __vtkMRMLLandmarkDetectionNode_h
