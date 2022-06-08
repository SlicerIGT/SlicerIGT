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

/// Manages the logic associated with encoded video input/output.
#ifndef __vtkSlicerVolumeReconstructionLogic_h
#define __vtkSlicerVolumeReconstructionLogic_h

#include "vtkSlicerVolumeReconstructionModuleLogicExport.h"


// Slicer includes
#include <vtkSlicerBaseLogic.h>
#include <vtkSlicerModuleLogic.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>

// Sequences MRML includes
#include <vtkMRMLSequenceBrowserNode.h>

// MRML includes
#include <vtkMRMLVolumeNode.h>

class vtkMRMLAnnotationROINode;
class vtkMRMLIGTLConnectorNode;
class vtkMRMLMarkupsROINode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumeReconstructionNode;


/// \ingroup Slicer_QtModules_VolumeReconstruction
class VTK_SLICER_VOLUMERECONSTRUCTION_MODULE_LOGIC_EXPORT vtkSlicerVolumeReconstructionLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerVolumeReconstructionLogic* New();
  vtkTypeMacro(vtkSlicerVolumeReconstructionLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream&, vtkIndent) override;
  void RegisterNodes() override;

  void UpdateLiveVolumeReconstruction();

  void StartVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);
  void StartLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);
  void ResumeLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);
  void StopLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);
  bool AddVolumeNodeToReconstructedVolume(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode, bool isFirst, bool isLast);
  void GetReconstructedVolume(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);

  void ReconstructVolumeFromSequence(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);

  void CalculateROIFromVolumeSequence(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
    vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLAnnotationROINode* outputROINodeRAS);
  void CalculateROIFromVolumeSequence(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
    vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLMarkupsROINode* outputROINodeRAS);
  void CalculateROIFromVolumeSequenceInternal(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
    vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLNode* outputROINodeRAS);

  vtkMRMLVolumeNode* GetOrAddOutputVolumeNode(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode);

  static bool IsGpuAccelerationSupported();


protected:
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);

  //----------------------------------------------------------------
  // Constructor, destructor etc.
  //----------------------------------------------------------------

  vtkSlicerVolumeReconstructionLogic();
  virtual ~vtkSlicerVolumeReconstructionLogic();

private:
  class vtkInternal;
  vtkInternal* Internal;

private:
  vtkSlicerVolumeReconstructionLogic(const vtkSlicerVolumeReconstructionLogic&); // Not implemented
  void operator=(const vtkSlicerVolumeReconstructionLogic&);               // Not implemented
};

#endif
