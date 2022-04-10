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

// .NAME vtkSlicerUltrasoundSnapshotsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerUltrasoundSnapshotsLogic_h
#define __vtkSlicerUltrasoundSnapshotsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerUltrasoundSnapshotsModuleLogicExport.h"


/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_ULTRASOUNDSNAPSHOTS_MODULE_LOGIC_EXPORT vtkSlicerUltrasoundSnapshotsLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerUltrasoundSnapshotsLogic *New();
  vtkTypeMacro(vtkSlicerUltrasoundSnapshotsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  
  void SetInputVolumeNode( vtkMRMLScalarVolumeNode* InputNode );
  vtkMRMLScalarVolumeNode* GetInputVolumeNode();
  void AddSnapshot( vtkMRMLScalarVolumeNode* InputNode, bool preserveWindowLevel );
  void ClearSnapshots();
  
  
protected:
  vtkSlicerUltrasoundSnapshotsLogic();
  virtual ~vtkSlicerUltrasoundSnapshotsLogic();
  
  int snapshotCounter; // This is only used to ensure unique MRML node names.

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes() override;
  virtual void UpdateFromMRMLScene() override;
  virtual void OnMRMLSceneEndImport() override;
  virtual void OnMRMLSceneStartClose() override;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerUltrasoundSnapshotsLogic(const vtkSlicerUltrasoundSnapshotsLogic&); // Not implemented
  void operator=(const vtkSlicerUltrasoundSnapshotsLogic&);               // Not implemented
};

#endif
