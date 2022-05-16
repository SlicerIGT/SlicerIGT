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

#include <vtkMRMLVolumeReconstructionNode.h>

// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>

// Sequnce MRML includes
#include <vtkMRMLSequenceBrowserNode.h>

// VTK includes
#include <vtkCommand.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumeReconstructionNode);

//----------------------------------------------------------------------------
vtkMRMLVolumeReconstructionNode::vtkMRMLVolumeReconstructionNode()
{
  this->LiveVolumeReconstruction = false;

  this->LiveUpdateIntervalSeconds = 1.0;

  this->ClipRectangleOrigin[0] = 0;
  this->ClipRectangleOrigin[1] = 0;

  this->ClipRectangleSize[0] = 0;
  this->ClipRectangleSize[1] = 0;

  this->OutputSpacing[0] = 1.0;
  this->OutputSpacing[1] = 1.0;
  this->OutputSpacing[2] = 1.0;

  this->InterpolationMode = NEAREST_NEIGHBOR_INTERPOLATION;
  this->OptimizationMode = FULL_OPTIMIZATION;
  this->CompoundingMode = MAXIMUM_COMPOUNDING_MODE;
  this->FillHoles = false;
  this->NumberOfThreads = 0;

  this->NumberOfVolumesAddedToReconstruction = 0;
  this->LiveVolumeReconstructionInProgress = false;

  this->AddNodeReferenceRole(this->GetInputSequenceBrowserNodeReferenceRole(), this->GetInputSequenceBrowserNodeReferenceMRMLAttributeName());
  this->AddNodeReferenceRole(this->GetInputROINodeReferenceRole(), this->GetInputROINodeReferenceMRMLAttributeName());
  this->AddNodeReferenceRole(this->GetOutputVolumeNodeReferenceRole(), this->GetOutputVolumeNodeReferenceMRMLAttributeName());

  vtkNew<vtkIntArray> inputVolumeEvents;
  inputVolumeEvents->InsertNextTuple1(vtkMRMLVolumeNode::ImageDataModifiedEvent);
  this->AddNodeReferenceRole(this->GetInputVolumeNodeReferenceRole(), this->GetInputVolumeNodeReferenceMRMLAttributeName(), inputVolumeEvents);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeReconstructionNode::~vtkMRMLVolumeReconstructionNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(liveVolumeReconstruction, LiveVolumeReconstruction);
  vtkMRMLWriteXMLFloatMacro(liveUpdateIntervalSeconds, LiveUpdateIntervalSeconds);
  vtkMRMLWriteXMLVectorMacro(clipRectangleOrigin, ClipRectangleOrigin, int, 2);
  vtkMRMLWriteXMLVectorMacro(clipRectangleSize, ClipRectangleSize, int, 2);
  vtkMRMLWriteXMLVectorMacro(outputSpacing, OutputSpacing, double, 3);
  vtkMRMLWriteXMLEnumMacro(interpolationMode, InterpolationMode);
  vtkMRMLWriteXMLEnumMacro(optimizationMode, OptimizationMode);
  vtkMRMLWriteXMLEnumMacro(compoundingMode, CompoundingMode);
  vtkMRMLWriteXMLBooleanMacro(fillHoles, FillHoles);
  vtkMRMLWriteXMLIntMacro(numberOfThreads, NumberOfThreads);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(liveVolumeReconstruction, LiveVolumeReconstruction);
  vtkMRMLReadXMLFloatMacro(liveUpdateIntervalSeconds, LiveUpdateIntervalSeconds);
  vtkMRMLReadXMLVectorMacro(clipRectangleOrigin, ClipRectangleOrigin, int, 2);
  vtkMRMLReadXMLVectorMacro(clipRectangleSize, ClipRectangleSize, int, 2);
  vtkMRMLReadXMLVectorMacro(outputSpacing, OutputSpacing, double, 3);
  vtkMRMLReadXMLEnumMacro(interpolationMode, InterpolationMode);
  vtkMRMLReadXMLEnumMacro(optimizationMode, OptimizationMode);
  vtkMRMLReadXMLEnumMacro(compoundingMode, CompoundingMode);
  vtkMRMLReadXMLBooleanMacro(fillHoles, FillHoles);
  vtkMRMLReadXMLIntMacro(numberOfThreads, NumberOfThreads);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::Copy(vtkMRMLNode* anode)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(LiveVolumeReconstruction);
  vtkMRMLCopyFloatMacro(LiveUpdateIntervalSeconds);
  vtkMRMLCopyVectorMacro(ClipRectangleOrigin, int, 2);
  vtkMRMLCopyVectorMacro(ClipRectangleSize, int, 2);
  vtkMRMLCopyVectorMacro(OutputSpacing, double, 3);
  vtkMRMLCopyEnumMacro(InterpolationMode);
  vtkMRMLCopyEnumMacro(OptimizationMode);
  vtkMRMLCopyEnumMacro(CompoundingMode);
  vtkMRMLCopyBooleanMacro(FillHoles);
  vtkMRMLCopyIntMacro(NumberOfThreads);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(LiveVolumeReconstruction);
  vtkMRMLPrintFloatMacro(LiveUpdateIntervalSeconds);
  vtkMRMLPrintFloatMacro(LiveUpdateIntervalSeconds);
  vtkMRMLPrintVectorMacro(ClipRectangleOrigin, int, 2);
  vtkMRMLPrintVectorMacro(ClipRectangleSize, int, 2);
  vtkMRMLPrintVectorMacro(OutputSpacing, double, 3);
  vtkMRMLPrintEnumMacro(InterpolationMode);
  vtkMRMLPrintEnumMacro(OptimizationMode);
  vtkMRMLPrintEnumMacro(CompoundingMode);
  vtkMRMLPrintBooleanMacro(FillHoles);
  vtkMRMLPrintIntMacro(NumberOfThreads);
  vtkMRMLPrintIntMacro(NumberOfVolumesAddedToReconstruction);
  vtkMRMLPrintIntMacro(LiveVolumeReconstructionInProgress);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeReconstructionNode::GetInterpolationModeAsString(int interpolationMode)
{
  return vtkIGSIOPasteSliceIntoVolume::GetInterpolationModeAsString(static_cast<vtkIGSIOPasteSliceIntoVolume::InterpolationType>(interpolationMode));
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeReconstructionNode::GetInterpolationModeFromString(const char* interpolationMode)
{
  for (int i = 0; i < INTERPOLATION_LAST; ++i)
  {
    if (strcmp(this->GetInterpolationModeAsString(i), interpolationMode) == 0)
    {
      return i;
    }
  }
  return NEAREST_NEIGHBOR_INTERPOLATION;
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeReconstructionNode::GetOptimizationModeAsString(int optimizationMode)
{
  return vtkIGSIOPasteSliceIntoVolume::GetOptimizationModeAsString(static_cast<vtkIGSIOPasteSliceIntoVolume::OptimizationType>(optimizationMode));
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeReconstructionNode::GetOptimizationModeFromString(const char* optimizationMode)
{
  for (int i = 0; i < OPTIMIZATION_LAST; ++i)
  {
    if (strcmp(this->GetOptimizationModeAsString(i), optimizationMode) == 0)
    {
      return i;
    }
  }
  return NO_OPTIMIZATION;
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeReconstructionNode::GetCompoundingModeAsString(int compoundingMode)
{
  return vtkIGSIOPasteSliceIntoVolume::GetCompoundingModeAsString(static_cast<vtkIGSIOPasteSliceIntoVolume::CompoundingType>(compoundingMode));
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeReconstructionNode::GetCompoundingModeFromString(const char* compoundingMode)
{
  for (int i = 0; i < COMPOUNDING_MODE_LAST; ++i)
  {
    if (strcmp(this->GetCompoundingModeAsString(i), compoundingMode) == 0)
    {
      return i;
    }
  }
  return MAXIMUM_COMPOUNDING_MODE;
}


//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::ProcessMRMLEvents(vtkObject* caller, unsigned long eventID, void* callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);
  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }

  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(caller);
  if (volumeNode && volumeNode == this->GetInputVolumeNode())
  {
    this->InvokeEvent(InputVolumeModified, volumeNode);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::SetNumberOfVolumesAddedToReconstruction(int numberOfVolumesAddedToReconstruction)
{
  this->NumberOfVolumesAddedToReconstruction = numberOfVolumesAddedToReconstruction;
};

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::SetAndObserveInputSequenceBrowserNode(vtkMRMLSequenceBrowserNode* node)
{
  this->SetNodeReferenceID(this->GetInputSequenceBrowserNodeReferenceRole(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::SetAndObserveInputVolumeNode(vtkMRMLVolumeNode* node)
{
  this->SetNodeReferenceID(this->GetInputVolumeNodeReferenceRole(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::SetAndObserveInputROINode(vtkMRMLAnnotationROINode* node)
{
  this->SetNodeReferenceID(this->GetInputROINodeReferenceRole(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::SetAndObserveInputROINode(vtkMRMLMarkupsROINode* node)
{
  this->SetNodeReferenceID(this->GetInputROINodeReferenceRole(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeReconstructionNode::SetAndObserveOutputVolumeNode(vtkMRMLVolumeNode* node)
{
  this->SetNodeReferenceID(this->GetOutputVolumeNodeReferenceRole(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkMRMLVolumeReconstructionNode::GetInputSequenceBrowserNode()
{
  return vtkMRMLSequenceBrowserNode::SafeDownCast(this->GetNodeReference(this->GetInputSequenceBrowserNodeReferenceRole()));
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeReconstructionNode::GetInputVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(this->GetInputVolumeNodeReferenceRole()));
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLVolumeReconstructionNode::GetInputROINode()
{
  return this->GetNodeReference(this->GetInputROINodeReferenceRole());
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVolumeReconstructionNode::GetOutputVolumeNode()
{
  return vtkMRMLVolumeNode::SafeDownCast(this->GetNodeReference(this->GetOutputVolumeNodeReferenceRole()));
}
