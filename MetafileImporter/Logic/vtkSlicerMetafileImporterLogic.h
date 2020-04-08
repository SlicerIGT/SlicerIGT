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

// .NAME vtkSlicerMetafileImporterLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerMetafileImporterLogic_h
#define __vtkSlicerMetafileImporterLogic_h

// MRML Sequence includes
#include "vtkMRMLLinearTransformSequenceStorageNode.h"

// STD includes
#include <cstdlib>
#include <deque>

// VTK includes
#include "vtkMatrix4x4.h"
#include "vtkMetaImageReader.h"
#include "vtkMetaImageWriter.h"

// ITK includes

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MetfileImporter includes
#include "vtkSlicerMetafileImporterModuleLogicExport.h"
#include "vtkSlicerSequencesLogic.h"

class vtkMRMLSequenceNode;
class vtkMRMLSequenceBrowserNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_METAFILEIMPORTER_MODULE_LOGIC_EXPORT vtkSlicerMetafileImporterLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerMetafileImporterLogic *New();
  vtkTypeMacro(vtkSlicerMetafileImporterLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void SetSequencesLogic(vtkSlicerSequencesLogic* sequencesLogic);

protected:
  vtkSlicerMetafileImporterLogic();
  virtual ~vtkSlicerMetafileImporterLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes() override;
  virtual void UpdateFromMRMLScene() override;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerMetafileImporterLogic(const vtkSlicerMetafileImporterLogic&); // Not implemented
  void operator=(const vtkSlicerMetafileImporterLogic&);               // Not implemented

public:

  /*!
    Read sequence file contents into the object.
    \param addedNodes if not NULL then returns sequence nodes that are added to the scene.
    Returns the created browser node on success, NULL on failure.
  */
  vtkMRMLSequenceBrowserNode* ReadSequenceFile(const std::string& fileName, vtkCollection* addedSequenceNodes=NULL);

  /*! Write sequence metafile contents to the file */
  bool WriteSequenceMetafile(const std::string& fileName, vtkMRMLSequenceBrowserNode* browserNode);

  /*!
    Read volume sequence from NRRD file.
    \param addedNodes if not NULL then returns sequence nodes that are added to the scene.
    Returns the created browser node on success, NULL on failure.
  */
  vtkMRMLSequenceBrowserNode* ReadVolumeSequence(const std::string& fileName, vtkCollection* addedSequenceNodes=NULL);


protected:

  /*! Read pixel data from the metaimage. Returns the pointer to the created image sequence. */
  vtkMRMLSequenceNode* ReadSequenceMetafileImages(const std::string& fileName, const std::string &baseNodeName, std::map< int, std::string >& frameNumberToIndexValueMap );

  /*! Write pixel data to the metaimage. Returns the pointer to the created image sequence. */
  void WriteSequenceMetafileImages(const std::string& fileName, vtkMRMLSequenceNode* imageNode, vtkMRMLSequenceNode* masterNode);

  /*! Generate a node name that contains the hierarchy name and index value */
  std::string GenerateDataNodeName(const std::string &dataItemName, const std::string& indexValue);

  /*! Logic for Sequence hierarchy to manipulate nodes */
  vtkSlicerSequencesLogic* SequencesLogic;

};

#endif
