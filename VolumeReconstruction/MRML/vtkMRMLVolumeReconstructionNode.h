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

#ifndef __vtkMRMLVolumeReconstructionNode_h
#define __vtkMRMLVolumeReconstructionNode_h

// MRML includes
#include <vtkMRMLNode.h>

// vtkAddon includes  
#include <vtkAddonSetGet.h>

// Volume Reconstruction includes
#include "vtkSlicerVolumeReconstructionModuleMRMLExport.h"

// IGSIO includes
#include <vtkIGSIOPasteSliceIntoVolume.h>

class vtkMRMLAnnotationROINode;
class vtkMRMLMarkupsROINode;
class vtkMRMLSequenceBrowserNode;
class vtkMRMLVolumeNode;

/// \ingroup VolumeReconstruction
/// \brief Parameter set node for volume reconstruction
///
class VTK_SLICER_VOLUMERECONSTRUCTION_MODULE_MRML_EXPORT vtkMRMLVolumeReconstructionNode : public vtkMRMLNode
{

public:

  static vtkMRMLVolumeReconstructionNode* New();
  vtkTypeMacro(vtkMRMLVolumeReconstructionNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Standard MRML node methods
  virtual vtkMRMLNode* CreateNodeInstance() override;
  virtual void ReadXMLAttributes(const char** atts) override;
  virtual void WriteXML(ostream& of, int indent) override;
  virtual void Copy(vtkMRMLNode* node) override;
  virtual const char* GetNodeTagName() override { return "VolumeReconstruction"; }
  void ProcessMRMLEvents(vtkObject* caller, unsigned long eventID, void* callData) override;

protected:
  vtkMRMLVolumeReconstructionNode();
  virtual ~vtkMRMLVolumeReconstructionNode();
  vtkMRMLVolumeReconstructionNode(const vtkMRMLVolumeReconstructionNode&);
  void operator=(const vtkMRMLVolumeReconstructionNode&);

public:

  enum
  {
    VolumeReconstructionStarted = 56000,
    VolumeAddedToReconstruction,
    VolumeReconstructionFinished,
    InputVolumeModified,
  };

  enum InterpolationType
  {
    NEAREST_NEIGHBOR_INTERPOLATION = vtkIGSIOPasteSliceIntoVolume::NEAREST_NEIGHBOR_INTERPOLATION,
    LINEAR_INTERPOLATION = vtkIGSIOPasteSliceIntoVolume::LINEAR_INTERPOLATION,
    INTERPOLATION_LAST
  };

  enum OptimizationType
  {
    NO_OPTIMIZATION = vtkIGSIOPasteSliceIntoVolume::NO_OPTIMIZATION,
    PARTIAL_OPTIMIZATION = vtkIGSIOPasteSliceIntoVolume::PARTIAL_OPTIMIZATION,
    FULL_OPTIMIZATION = vtkIGSIOPasteSliceIntoVolume::FULL_OPTIMIZATION,
    GPU_ACCELERATION_OPENCL = vtkIGSIOPasteSliceIntoVolume::GPU_ACCELERATION_OPENCL,
    OPTIMIZATION_LAST
  };

  enum CompoundingType
  {
    UNDEFINED_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::UNDEFINED_COMPOUNDING_MODE,
    LATEST_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::LATEST_COMPOUNDING_MODE,
    MAXIMUM_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::MAXIMUM_COMPOUNDING_MODE,
    MEAN_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::MEAN_COMPOUNDING_MODE,
    IMPORTANCE_MASK_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::IMPORTANCE_MASK_COMPOUNDING_MODE,
    COMPOUNDING_MODE_LAST
  };

  /*!
  InputSequenceBrowserNode is used for reconstructing an image volume from a sequence.
  The InputSequenceBrowserNode should contain the sequences for the InputVolumeNode, as well as all recorded transforms.
  */
  const char* GetInputSequenceBrowserNodeReferenceRole() { return "inputSequenceBrowserNode"; };
  const char* GetInputSequenceBrowserNodeReferenceMRMLAttributeName() { return "inputSequenceBrowserNodeRef"; };
  vtkMRMLSequenceBrowserNode* GetInputSequenceBrowserNode();
  virtual void SetAndObserveInputSequenceBrowserNode(vtkMRMLSequenceBrowserNode* sequenceBrowserNode);

  /*!
  InputVolumeNode is the volume node that is used for both live and recorded volume reconstruction.
  */
  const char* GetInputVolumeNodeReferenceRole() { return "inputVolumeNode"; };
  const char* GetInputVolumeNodeReferenceMRMLAttributeName() { return "inputVolumeNodeRef"; };
  vtkMRMLVolumeNode* GetInputVolumeNode();
  virtual void SetAndObserveInputVolumeNode(vtkMRMLVolumeNode* volumeNode);

  /*!
  InputROINode is the region of interest that will be reconstructed. The size of the ROI dictates the
  dimensions of the output volume.
  */
  const char* GetInputROINodeReferenceRole() { return "inputROINode"; };
  const char* GetInputROINodeReferenceMRMLAttributeName() { return "inputROINodeRef"; };
  vtkMRMLNode* GetInputROINode();
  virtual void SetAndObserveInputROINode(vtkMRMLAnnotationROINode* roiNode);
  virtual void SetAndObserveInputROINode(vtkMRMLMarkupsROINode* roiNode);

  /*!
  OutputVolumeNode is the volume node that the reconstruction results will be read into.
  */
  const char* GetOutputVolumeNodeReferenceRole() { return "outputVolumeNode"; };
  const char* GetOutputVolumeNodeReferenceMRMLAttributeName() { return "outputVolumeNodeRef"; };
  vtkMRMLVolumeNode* GetOutputVolumeNode();
  virtual void SetAndObserveOutputVolumeNode(vtkMRMLVolumeNode* volumeNode);

  /*!
  LiveVolumeReconstruction is true if the node is intended for live volume reconstruction.
  */
  vtkSetMacro(LiveVolumeReconstruction, bool);
  vtkGetMacro(LiveVolumeReconstruction, bool);

  /*!
  During live update, LiveUpdateIntervalSeconds controls the frequency that the output is read from the volume reconstructor.
  */
  vtkSetMacro(LiveUpdateIntervalSeconds, double);
  vtkGetMacro(LiveUpdateIntervalSeconds, double);

  /*!
  The clip rectangle origin to apply to the image in pixel coordinates.
  Pixels outside the clip rectangle will not be pasted into the volume.
  The origin of the rectangle is at its corner that is closest to the image origin.
  */
  vtkSetVector2Macro(ClipRectangleOrigin, int);
  vtkGetVector2Macro(ClipRectangleOrigin, int);

  /*!
  The clip rectangle size in pixels
  */
  vtkSetVector2Macro(ClipRectangleSize, int);
  vtkGetVector2Macro(ClipRectangleSize, int);

  /*!
  Set spacing of the output data in Reference coordinate system.
  This is required to be set, otherwise the reconstructed volume will be empty.
  */
  vtkSetVector3Macro(OutputSpacing, double);
  vtkGetVector3Macro(OutputSpacing, double);

  /*!
  Set the interpolation mode
  LINEAR:           Each pixel is distributed into the surrounding eight voxels using trilinear interpolation weights.
                    Resists noise, but is slower and may introduce blurring.
  NEAREST_NEIGHBOR: Each pixel is inserted only into the spatially nearest voxel.
                    Faster, but is susceptible to noise. (default)
  */
  vtkSetMacro(InterpolationMode, int);
  vtkGetMacro(InterpolationMode, int);
  const char* GetInterpolationModeAsString(int interpolationMode);
  int GetInterpolationModeFromString(const char* interpolationMode);

  /*!
  Optimization method (turn off optimization only if it is not stable
  on your architecture).
  NO_OPTIMIZATION: means no optimization (almost never used)
  PARTIAL_OPTIMIZATION: break transformation into x, y and z components, and
    don't do bounds checking for nearest-neighbor interpolation
  FULL_OPTIMIZATION: fixed-point (i.e. integer) math is used instead of float math,
    it is only useful with NEAREST_NEIGHBOR interpolation
    (when used with LINEAR interpolation then it is slower than NO_OPTIMIZATION)
  GPU_ACCELERATION_OPENCL: OpenCL-based GPU reconstruction implementation.
  */
  vtkSetMacro(OptimizationMode, int);
  vtkGetMacro(OptimizationMode, int);
  const char* GetOptimizationModeAsString(int optimizationMode);
  int GetOptimizationModeFromString(const char* optimizationMode);

  /*!
  The compounding mode
  MEAN:            For each voxel, use an average of all inserted pixel values. Used on single or multiple sweeps
                   from the same angle (regardless of intersection). Resistant to noise, but slower than other
                   compounding methods.
  IMPORTANCE_MASK: Similar to MEAN, but pixels in a frame are weighted by their importance, which is supplied by a mask.
  LATEST:          For each voxel, use only the latest inserted pixel value. Used on single or multiple sweeps
                   from the same angle (regardless of intersection). Fast, but susceptible to noise.
  MAXIMUM:         For each voxel, use only the pixel value with the highest intensity. Used when multiple slices
                   from different angles are expected to intersect. Fast, but susceptible to noise.
  */
  vtkSetMacro(CompoundingMode, int);
  vtkGetMacro(CompoundingMode, int);
  const char* GetCompoundingModeAsString(int compoundingMode);
  int GetCompoundingModeFromString(const char* compoundingMode);

  /*!
  The output reconstructed volume may contain holes(empty voxels between images slices).
  If FillHoles is enabeld, The vtkIGSIOFillHolesInVolume filter will be used for post - processing to fill holes with
  values similar to nearby voxels.
  */
  vtkSetMacro(FillHoles, bool);
  vtkGetMacro(FillHoles, bool);

  /*!
  Number of threads used for processing the data.
  The reconstruction result is slightly different if more than one thread is used
  because due to interpolation and rounding errors is influenced by the order the pixels
  are processed.
  Choose 0 (this is the default) for maximum speed, in this case the default number of
  used threads equals the number of processors. Choose 1 for reproducible results.
  */
  vtkSetMacro(NumberOfThreads, int);
  vtkGetMacro(NumberOfThreads, int);

  /*!
  The number of individual volumes that have been added to the reconstruction.
  */
  void SetNumberOfVolumesAddedToReconstruction(int numberOfVolumesAddedToReconstruction);
  vtkGetMacro(NumberOfVolumesAddedToReconstruction, int);

  /*!
  The state of live volume reconstrction.
  True if a reconstruction is currently in progress, false otherwise.
  */
  vtkSetMacro(LiveVolumeReconstructionInProgress, bool);
  vtkGetMacro(LiveVolumeReconstructionInProgress, bool);
  vtkBooleanMacro(LiveVolumeReconstructionInProgress, bool);

protected:
  bool LiveVolumeReconstruction;
  double LiveUpdateIntervalSeconds;
  int ClipRectangleOrigin[2];
  int ClipRectangleSize[2];
  double OutputSpacing[3];
  int InterpolationMode;
  int OptimizationMode;
  int CompoundingMode;
  bool FillHoles;
  int NumberOfThreads;
  int NumberOfVolumesAddedToReconstruction;
  bool LiveVolumeReconstructionInProgress;
};

#endif // __vtkMRMLVolumeReconstructionNode_h
