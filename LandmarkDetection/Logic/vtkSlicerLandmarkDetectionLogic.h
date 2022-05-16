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

// .NAME vtkSlicerLandmarkDetectionLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerLandmarkDetectionLogic_h
#define __vtkSlicerLandmarkDetectionLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerLandmarkDetectionModuleLogicExport.h"

class vtkMRMLLandmarkDetectionNode;
class vtkMRMLTransformNode;
class vtkMRMLMarkupsFiducialNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_LANDMARKDETECTION_MODULE_LOGIC_EXPORT vtkSlicerLandmarkDetectionLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerLandmarkDetectionLogic *New();
  vtkTypeMacro(vtkSlicerLandmarkDetectionLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

public:

  ///  TODO: Maybe delete?
  vtkMRMLLandmarkDetectionNode* AddLandmarkDetectionNode(vtkMRMLTransformNode* inputTransformNode, vtkMRMLMarkupsFiducialNode* outputFiducialNode);

  /// TODO: Kyle
  void ResetDetectionForNode(vtkMRMLLandmarkDetectionNode* landmarkDetectionNode);

protected:
  vtkSlicerLandmarkDetectionLogic();
  ~vtkSlicerLandmarkDetectionLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  void UpdateLandmarkDetectionAlgo(vtkMRMLLandmarkDetectionNode* landmarkDetectionNode);
  void AddTransformToAlgo(vtkMRMLLandmarkDetectionNode* landmarkDetectionNode);
  void UpdateDetectedLandmarks(vtkMRMLLandmarkDetectionNode* landmarkDetectionNode);

private:

  vtkSlicerLandmarkDetectionLogic(const vtkSlicerLandmarkDetectionLogic&); // Not implemented
  void operator=(const vtkSlicerLandmarkDetectionLogic&); // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
