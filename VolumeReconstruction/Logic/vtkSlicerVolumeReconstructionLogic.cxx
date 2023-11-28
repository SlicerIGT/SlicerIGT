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

// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>

// Sequence MRML includes
#include <vtkMRMLSequenceNode.h>

// IGSIO Common includes
#include <vtkIGSIOTrackedFrameList.h>
#include <vtkIGSIOTransformRepository.h>
#include <igsioTrackedFrame.h>

// IGSIO VolumeReconstructor includes
#include <vtkIGSIOVolumeReconstructor.h>
#include <vtkIGSIOFillHolesInVolume.h>
#include <vtkIGSIOPasteSliceIntoVolume.h>

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

// VolumeReconstructor MRML includes
#include "vtkMRMLVolumeReconstructionNode.h"

// VolumeReconstruction Logic includes
#include "vtkSlicerVolumeReconstructionLogic.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVolumeReconstructionLogic);

struct ReconstructionInfo
{
  vtkSmartPointer<vtkIGSIOVolumeReconstructor> Reconstructor{nullptr};
  double LastUpdateTimeSeconds{0.0};
};

typedef std::map<vtkMRMLVolumeReconstructionNode*, ReconstructionInfo> VolumeReconstuctorMap;

//---------------------------------------------------------------------------
class vtkSlicerVolumeReconstructionLogic::vtkInternal
{
public:
  //---------------------------------------------------------------------------
  vtkInternal(vtkSlicerVolumeReconstructionLogic* external);
  ~vtkInternal();

  vtkSlicerVolumeReconstructionLogic* External;

  VolumeReconstuctorMap Reconstructors;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::vtkInternal::vtkInternal(vtkSlicerVolumeReconstructionLogic* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkSlicerVolumeReconstructionLogic methods

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::vtkSlicerVolumeReconstructionLogic()
  : Internal(new vtkInternal(this))
{
}

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::~vtkSlicerVolumeReconstructionLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLVolumeReconstructionNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(node);
  if (!node || !this->GetMRMLScene())
  {
    return;
  }

  ReconstructionInfo info;
  info.Reconstructor = vtkSmartPointer<vtkIGSIOVolumeReconstructor>::New();
  info.LastUpdateTimeSeconds = vtkTimerLog::GetUniversalTime();

  this->Internal->Reconstructors[volumeReconstructionNode] = info;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(node);
  if (!volumeReconstructionNode || !this->GetMRMLScene())
  {
    return;
  }

  VolumeReconstuctorMap::iterator volumeReconstructorIt = this->Internal->Reconstructors.find(volumeReconstructionNode);
  if (volumeReconstructorIt != this->Internal->Reconstructors.end())
  {
    this->Internal->Reconstructors.erase(volumeReconstructorIt);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(caller);
  if (volumeReconstructionNode && event == vtkMRMLVolumeReconstructionNode::InputVolumeModified && volumeReconstructionNode->GetLiveVolumeReconstructionInProgress())
  {
    this->AddVolumeNodeToReconstructedVolume(volumeReconstructionNode, volumeReconstructionNode->GetNumberOfVolumesAddedToReconstruction() == 0, false);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::UpdateLiveVolumeReconstruction()
{
  vtkNew<vtkTimerLog> timer;
  for (VolumeReconstuctorMap::iterator it = this->Internal->Reconstructors.begin(); it != this->Internal->Reconstructors.end(); ++it)
  {
    vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = it->first;
    ReconstructionInfo* info = &(it->second);
    vtkIGSIOVolumeReconstructor* reconstructor = info->Reconstructor;
    if (!volumeReconstructionNode || !reconstructor || !volumeReconstructionNode->GetLiveVolumeReconstructionInProgress())
    {
      continue;
    }

    double currentTime = timer->GetUniversalTime();
    if (currentTime - info->LastUpdateTimeSeconds < volumeReconstructionNode->GetLiveUpdateIntervalSeconds())
    {
      continue;
    }

    this->GetReconstructedVolume(volumeReconstructionNode, false);
    info->LastUpdateTimeSeconds = currentTime;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StartVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstruction node!");
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return;
  }

  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return;
  }

  vtkMRMLAnnotationROINode* annotationInputROINode = vtkMRMLAnnotationROINode::SafeDownCast(volumeReconstructionNode->GetInputROINode());
  vtkMRMLMarkupsROINode* markupsInputROINode = vtkMRMLMarkupsROINode::SafeDownCast(volumeReconstructionNode->GetInputROINode());
  if (!annotationInputROINode && !markupsInputROINode)
  {
    vtkErrorMacro("Invalid input ROI node!");
    return;
  }

  double bounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  if (annotationInputROINode)
  {
    annotationInputROINode->GetBounds(bounds);
  }
  else if (markupsInputROINode)
  {
    double size[3] = { 0.0, 0.0, 0.0 };
    markupsInputROINode->GetSize(size);
    bounds[0] = -size[0] / 2.0;
    bounds[1] = size[0] / 2.0;
    bounds[2] = -size[1] / 2.0;
    bounds[3] = size[1] / 2.0;
    bounds[4] = -size[2] / 2.0;
    bounds[5] = size[2] / 2.0;
  }

  double outputSpacing[3] = { 0.0, 0.0, 0.0 };
  volumeReconstructionNode->GetOutputSpacing(outputSpacing);
  int outputExtent[6] = {
    0, static_cast<int>(std::ceil((bounds[1] - bounds[0]) / outputSpacing[0])),
    0, static_cast<int>(std::ceil((bounds[3] - bounds[2]) / outputSpacing[1])),
    0, static_cast<int>(std::ceil((bounds[5] - bounds[4]) / outputSpacing[2]))
  };
  double outputOrigin[3] = { bounds[0], bounds[2], bounds[4] };

  reconstructor->SetOutputExtent(outputExtent);
  reconstructor->SetOutputOrigin(outputOrigin);
  reconstructor->SetOutputSpacing(outputSpacing);
  reconstructor->SetCompoundingMode(vtkIGSIOPasteSliceIntoVolume::CompoundingType(volumeReconstructionNode->GetCompoundingMode()));
  reconstructor->SetOptimization(vtkIGSIOPasteSliceIntoVolume::OptimizationType(volumeReconstructionNode->GetOptimizationMode()));
  reconstructor->SetInterpolation(vtkIGSIOPasteSliceIntoVolume::InterpolationType(volumeReconstructionNode->GetInterpolationMode()));
  reconstructor->SetNumberOfThreads(volumeReconstructionNode->GetNumberOfThreads());
  reconstructor->SetFillHoles(volumeReconstructionNode->GetFillHoles());
  if (volumeReconstructionNode->GetFillHoles())
  {
    vtkIGSIOFillHolesInVolume* holeFiller = reconstructor->GetHoleFiller();
    holeFiller->SetNumHFElements(1);
    holeFiller->AllocateHFElements();
    FillHolesInVolumeElement hfElement;
    hfElement.setupAsStick(9, 1);
    holeFiller->SetHFElement(0, hfElement);
  }
  reconstructor->SetImageCoordinateFrame("Image");
  reconstructor->SetReferenceCoordinateFrame("ROI");
  reconstructor->SetClipRectangleOrigin(volumeReconstructionNode->GetClipRectangleOrigin());
  reconstructor->SetClipRectangleSize(volumeReconstructionNode->GetClipRectangleSize());

  this->ResetVolumeReconstruction(volumeReconstructionNode);

  volumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(0);
  volumeReconstructionNode->InvokeEvent(vtkMRMLVolumeReconstructionNode::VolumeReconstructionStarted);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StartLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    return;
  }
  this->StartVolumeReconstruction(volumeReconstructionNode);
  this->ResumeLiveVolumeReconstruction(volumeReconstructionNode);
  this->GetReconstructedVolume(volumeReconstructionNode, true);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ResumeLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    return;
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLVolumeReconstructionNode::InputVolumeModified);
  vtkObserveMRMLNodeEventsMacro(volumeReconstructionNode, events);
  volumeReconstructionNode->LiveVolumeReconstructionInProgressOn();
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StopLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    return;
  }
  volumeReconstructionNode->LiveVolumeReconstructionInProgressOff();
  vtkUnObserveMRMLNodeMacro(volumeReconstructionNode);
  this->GetReconstructedVolume(volumeReconstructionNode, true);
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeReconstructionLogic::AddVolumeNodeToReconstructedVolume(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode, bool isFirst, bool isLast)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstruction node!");
    return false;
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return false;
  }

  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return false;
  }

  vtkMRMLAnnotationROINode* annotationInputROINode = vtkMRMLAnnotationROINode::SafeDownCast(volumeReconstructionNode->GetInputROINode());
  vtkMRMLMarkupsROINode* markupsInputROINode = vtkMRMLMarkupsROINode::SafeDownCast(volumeReconstructionNode->GetInputROINode());
  if (!annotationInputROINode && !markupsInputROINode)
  {
    vtkErrorMacro("Invalid input ROI node!");
    return false;
  }

  vtkNew<vtkTransform> imageToROITransform;
  imageToROITransform->PostMultiply();

  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  inputVolumeNode->GetIJKToRASMatrix(ijkToRASMatrix);
  imageToROITransform->Concatenate(ijkToRASMatrix);

  vtkMRMLTransformNode* imageParentTransformNode = inputVolumeNode->GetParentTransformNode();
  if (imageParentTransformNode)
  {
    vtkNew<vtkMatrix4x4> parentToWorldMatrix;
    imageParentTransformNode->GetMatrixTransformToWorld(parentToWorldMatrix);
    imageToROITransform->Concatenate(parentToWorldMatrix);
  }

  vtkMRMLTransformNode* roiParentTransformNode = vtkMRMLTransformableNode::SafeDownCast(volumeReconstructionNode->GetInputROINode())->GetParentTransformNode();
  if (roiParentTransformNode)
  {
    vtkNew<vtkMatrix4x4> worldToParentMatrix;
    roiParentTransformNode->GetMatrixTransformFromWorld(worldToParentMatrix);
    imageToROITransform->Concatenate(worldToParentMatrix);
  }

  if (markupsInputROINode)
  {
    vtkNew<vtkMatrix4x4> nodeToObjectMatrix;
    vtkMatrix4x4::Invert(markupsInputROINode->GetObjectToNodeMatrix(), nodeToObjectMatrix);
    imageToROITransform->Concatenate(nodeToObjectMatrix);
  }

  vtkNew<vtkIGSIOTransformRepository> transformRepository;
  transformRepository->SetTransform(igsioTransformName("ImageToROI"), imageToROITransform->GetMatrix());

  vtkImageData* inputImageData = inputVolumeNode->GetImageData();

  // Ensure that output scalar type matches input (only same scalar type can be added to the volume)
  if (inputImageData)
  {
    reconstructor->SetOutputScalarType(inputImageData->GetScalarType());
  }

  std::string errorDetail;
  igsioTrackedFrame trackedFrame;
  trackedFrame.GetImageData()->DeepCopyFrom(inputImageData);

  bool insertedIntoVolume = false;
  if (reconstructor->AddTrackedFrame(&trackedFrame, transformRepository, isFirst, isLast, &insertedIntoVolume) != IGSIO_SUCCESS)
  {
    return false;
  }

  int numberOfVolumesAddedToReconstruction = volumeReconstructionNode->GetNumberOfVolumesAddedToReconstruction();
  volumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(numberOfVolumesAddedToReconstruction + 1);
  volumeReconstructionNode->InvokeEvent(vtkMRMLVolumeReconstructionNode::VolumeAddedToReconstruction);
  return  true;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::GetReconstructedVolume(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode, bool deepCopy/*=true*/)
{
  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return;
  }

  vtkMRMLVolumeNode* outputVolumeNode = this->GetOrAddOutputVolumeNode(volumeReconstructionNode);
  if (!outputVolumeNode)
  {
    vtkErrorMacro("Invalid output volume node!");
    return;
  }

  MRMLNodeModifyBlocker blocker(outputVolumeNode);
  if (!outputVolumeNode->GetImageData())
  {
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    outputVolumeNode->SetAndObserveImageData(imageData);
  }

  if (reconstructor->GetReconstructedVolume(outputVolumeNode->GetImageData(), deepCopy) != IGSIO_SUCCESS)
  {
    vtkErrorMacro("Could not retrieve reconstructed image");
  }

  double spacing[3] = { 0.0, 0.0, 0.0 };
  outputVolumeNode->GetImageData()->GetSpacing(spacing);
  outputVolumeNode->GetImageData()->SetSpacing(1.0, 1.0, 1.0);
  outputVolumeNode->SetSpacing(spacing);

  double origin[3] = { 0.0, 0.0, 0.0 };
  outputVolumeNode->GetImageData()->GetOrigin(origin);
  outputVolumeNode->GetImageData()->SetOrigin(0.0, 0.0, 0.0);
  outputVolumeNode->SetOrigin(origin);

  const char* parentTransformNodeID = nullptr;
  vtkMRMLTransformableNode* inputROINode = vtkMRMLTransformableNode::SafeDownCast(volumeReconstructionNode->GetInputROINode());
  if (inputROINode && inputROINode->GetParentTransformNode())
  {
    parentTransformNodeID = inputROINode->GetParentTransformNode()->GetID();
  }
  outputVolumeNode->SetAndObserveTransformNodeID(parentTransformNodeID);

  vtkMRMLMarkupsROINode* markupsROINode = vtkMRMLMarkupsROINode::SafeDownCast(inputROINode);
  if (markupsROINode)
  {
    vtkMatrix4x4* objectToNodeMatrix = markupsROINode->GetObjectToNodeMatrix();
    outputVolumeNode->SetIJKToRASDirectionMatrix(objectToNodeMatrix);

    // Reconstructed volume origin is in ROI coordinates. Need to convert to Node
    vtkNew<vtkTransform> objectToNodeTransform;
    objectToNodeTransform->SetMatrix(objectToNodeMatrix);
    objectToNodeTransform->TransformPoint(origin, origin);
    outputVolumeNode->SetOrigin(origin);
  }

  volumeReconstructionNode->InvokeEvent(vtkMRMLVolumeReconstructionNode::VolumeReconstructionFinished);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ReconstructVolumeFromSequence(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstruction node!");
    return;
  }

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = volumeReconstructionNode->GetInputSequenceBrowserNode();
  if (!inputSequenceBrowser)
  {
    vtkErrorMacro("Invalid input sequence browser!");
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return;
  }

  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return;
  }

  vtkMRMLSequenceNode* masterSequence = inputSequenceBrowser->GetMasterSequenceNode();
  if (!masterSequence)
  {
    vtkErrorMacro("Invalid master sequence node!");
    return;
  }

  vtkSmartPointer<vtkMRMLTransformableNode> inputROINode = vtkMRMLTransformableNode::SafeDownCast(volumeReconstructionNode->GetInputROINode());
  if (!inputROINode)
  {
    if (this->GetMRMLScene())
    {
      inputROINode = vtkSmartPointer<vtkMRMLMarkupsROINode>::Take(vtkMRMLMarkupsROINode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLMarkupsROINode")));
    }
    if (!inputROINode)
    {
      inputROINode = vtkSmartPointer<vtkMRMLMarkupsROINode>::New();
    }

    if (inputVolumeNode->GetName())
    {
      std::string roiNodeName = inputVolumeNode->GetName();
      roiNodeName += "_Bounds";
      inputROINode->SetName(roiNodeName.c_str());
    }
    if (this->GetMRMLScene())
    {
      this->GetMRMLScene()->AddNode(inputROINode);
    }

    vtkSmartPointer<vtkMRMLVolumeNode> outputVolumeNode = volumeReconstructionNode->GetOutputVolumeNode();
    if (outputVolumeNode && outputVolumeNode->GetParentTransformNode())
    {
      inputROINode->SetAndObserveTransformNodeID(outputVolumeNode->GetParentTransformNode()->GetID());
    }

    if (vtkMRMLAnnotationROINode::SafeDownCast(inputROINode))
    {
      volumeReconstructionNode->SetAndObserveInputROINode(vtkMRMLAnnotationROINode::SafeDownCast(inputROINode));
    }
    else if (vtkMRMLMarkupsROINode::SafeDownCast(inputROINode))
    {
      volumeReconstructionNode->SetAndObserveInputROINode(vtkMRMLMarkupsROINode::SafeDownCast(inputROINode));
    }

    this->CalculateROIFromVolumeSequenceInternal(inputSequenceBrowser, inputVolumeNode, inputROINode);
  }

  // Begin volume reconstruction
  this->StartVolumeReconstruction(volumeReconstructionNode);

  // Save the currently selected item to restore later
  int selectedItemNumber = inputSequenceBrowser->GetSelectedItemNumber();

  const int numberOfFrames = masterSequence->GetNumberOfDataNodes();
  for (int i = 0; i < numberOfFrames; ++i)
  {
    inputSequenceBrowser->SetSelectedItemNumber(i);
    this->AddVolumeNodeToReconstructedVolume(volumeReconstructionNode, i == 0, i == numberOfFrames - 1);
  }

  this->GetReconstructedVolume(volumeReconstructionNode, true);
  inputSequenceBrowser->SetSelectedItemNumber(selectedItemNumber);
  this->GetApplicationLogic()->ResumeRender();
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::CalculateROIFromVolumeSequence(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
  vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLAnnotationROINode* outputROINodeRAS)
{
  this->CalculateROIFromVolumeSequenceInternal(inputSequenceBrowser, inputVolumeNode, outputROINodeRAS);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::CalculateROIFromVolumeSequence(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
  vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLMarkupsROINode* outputROINodeRAS)
{
  this->CalculateROIFromVolumeSequenceInternal(inputSequenceBrowser, inputVolumeNode, outputROINodeRAS);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::CalculateROIFromVolumeSequenceInternal(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
  vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLNode* outputROINodeRAS)
{
  if (!inputSequenceBrowser)
  {
    vtkErrorMacro("Invalid input sequence browser!");
    return;
  }

  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return;
  }

  vtkMRMLSequenceNode* masterSequence = inputSequenceBrowser->GetMasterSequenceNode();
  if (!masterSequence)
  {
    vtkErrorMacro("Invalid master sequence node!");
    return;
  }

  vtkNew<vtkTransform> imageToROITransform;
  imageToROITransform->PostMultiply();

  double roiBounds[6] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
                          VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
                          VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };

  const int numberOfFrames = masterSequence->GetNumberOfDataNodes();
  int selectedItemNumber = inputSequenceBrowser->GetSelectedItemNumber();
  for (int i = 0; i < numberOfFrames; ++i)
  {
    inputSequenceBrowser->SetSelectedItemNumber(i);

    imageToROITransform->Identity();

    vtkMRMLTransformNode* imageParentTransformNode = inputVolumeNode->GetParentTransformNode();
    if (imageParentTransformNode)
    {
      vtkNew<vtkMatrix4x4> parentToWorldMatrix;
      imageParentTransformNode->GetMatrixTransformToWorld(parentToWorldMatrix);
      imageToROITransform->Concatenate(parentToWorldMatrix);
    }




    vtkMRMLTransformNode* roiParentTransformNode = vtkMRMLTransformableNode::SafeDownCast(outputROINodeRAS)->GetParentTransformNode();
    if (roiParentTransformNode)
    {
      vtkNew<vtkMatrix4x4> worldToNodeMatrix;
      roiParentTransformNode->GetMatrixTransformFromWorld(worldToNodeMatrix);
      imageToROITransform->Concatenate(worldToNodeMatrix);
    }

    vtkMRMLMarkupsROINode* markupsInputROINode = vtkMRMLMarkupsROINode::SafeDownCast(outputROINodeRAS);
    if (markupsInputROINode)
    {
      vtkNew<vtkMatrix4x4> nodeToObjectMatrix;
      vtkMatrix4x4::Invert(markupsInputROINode->GetObjectToNodeMatrix(), nodeToObjectMatrix);
      imageToROITransform->Concatenate(nodeToObjectMatrix);
    }

    double selectedROIBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    inputVolumeNode->GetBounds(selectedROIBounds);

    // Transform all of the current image bounds from the local coordinates of the input image to the local coordinates of the ROI node
    double corner000[3] = { selectedROIBounds[0], selectedROIBounds[2], selectedROIBounds[4] };
    imageToROITransform->TransformPoint(corner000, corner000);
    double corner001[3] = { selectedROIBounds[0], selectedROIBounds[2], selectedROIBounds[5] };
    imageToROITransform->TransformPoint(corner001, corner001);
    double corner010[3] = { selectedROIBounds[0], selectedROIBounds[3], selectedROIBounds[4] };
    imageToROITransform->TransformPoint(corner010, corner010);
    double corner011[3] = { selectedROIBounds[0], selectedROIBounds[3], selectedROIBounds[5] };
    imageToROITransform->TransformPoint(corner011, corner011);
    double corner100[3] = { selectedROIBounds[1], selectedROIBounds[2], selectedROIBounds[4] };
    imageToROITransform->TransformPoint(corner100, corner100);
    double corner101[3] = { selectedROIBounds[1], selectedROIBounds[2], selectedROIBounds[5] };
    imageToROITransform->TransformPoint(corner101, corner101);
    double corner110[3] = { selectedROIBounds[1], selectedROIBounds[3], selectedROIBounds[4] };
    imageToROITransform->TransformPoint(corner110, corner110);
    double corner111[3] = { selectedROIBounds[1], selectedROIBounds[3], selectedROIBounds[5] };
    imageToROITransform->TransformPoint(corner111, corner111);

    for (int i = 0; i < 3; ++i)
    {
      selectedROIBounds[2 * i] = std::min({ corner000[i], corner001[i], corner010[i], corner011[i], corner100[i], corner101[i], corner110[i], corner111[i] });
      selectedROIBounds[2 * i + 1] = std::max({ corner000[i], corner001[i], corner010[i], corner011[i], corner100[i], corner101[i], corner110[i], corner111[i] });

      roiBounds[2 * i] = std::min(selectedROIBounds[2 * i], roiBounds[2 * i]);
      roiBounds[2 * i + 1] = std::max(selectedROIBounds[2 * i + 1], roiBounds[2 * i + 1]);
    }
  }
  inputSequenceBrowser->SetSelectedItemNumber(selectedItemNumber);

  double radius[3] = { 0.0, 0.0, 0.0 };
  double size[3] = { 0.0, 0.0, 0.0 };
  double center[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; ++i)
  {
    size[i] = roiBounds[2 * i + 1] - roiBounds[2 * i];
    radius[i] = 0.5 * size[i];
    center[i] = (roiBounds[2 * i + 1] + roiBounds[2 * i]) / 2.0;
  }

  vtkMRMLAnnotationROINode* outputAnnotationROINode = vtkMRMLAnnotationROINode::SafeDownCast(outputROINodeRAS);
  if (outputAnnotationROINode)
  {
    outputAnnotationROINode->SetXYZ(center);
    outputAnnotationROINode->SetRadiusXYZ(radius);
  }

  vtkMRMLMarkupsROINode* outputMarkupsROINode = vtkMRMLMarkupsROINode::SafeDownCast(outputROINodeRAS);
  if (outputMarkupsROINode)
  {
    outputMarkupsROINode->SetCenter(center);
    outputMarkupsROINode->SetSize(size);
  }
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumeReconstructionLogic::GetOrAddOutputVolumeNode(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstructor node!");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLVolumeNode> outputVolumeNode = volumeReconstructionNode->GetOutputVolumeNode();
  if (outputVolumeNode)
  {
    return outputVolumeNode;
  }

  if (this->GetMRMLScene())
  {
    outputVolumeNode = vtkSmartPointer<vtkMRMLVolumeNode>::Take(vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLScalarVolumeNode")));
  }
  if (!outputVolumeNode)
  {
    outputVolumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (inputVolumeNode && inputVolumeNode->GetName())
  {
    std::string roiNodeName = inputVolumeNode->GetName();
    roiNodeName += "_ReconstructedVolume";
    outputVolumeNode->SetName(roiNodeName.c_str());
  }
  else
  {
    outputVolumeNode->SetName("ReconstructedVolume");
  }

  if (this->GetMRMLScene())
  {
    this->GetMRMLScene()->AddNode(outputVolumeNode);
  }
  volumeReconstructionNode->SetAndObserveOutputVolumeNode(outputVolumeNode);
  return outputVolumeNode;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ResetVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("ResetVolumeReconstruction::Invalid volume reconstructor");
    return;
  }

  reconstructor->Reset();
  this->GetReconstructedVolume(volumeReconstructionNode);
  volumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(0);
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeReconstructionLogic::IsGpuAccelerationSupported()
{
  return vtkIGSIOPasteSliceIntoVolume::IsGpuAccelerationSupported();
};

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "vtkSlicerVolumeReconstructionLogic: " << this->GetClassName() << "\n";
}
