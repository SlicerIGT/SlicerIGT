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

// #define ENABLE_PERFORMANCE_PROFILING

// MetafileImporter Logic includes
#include "vtkSlicerMetafileImporterLogic.h"
#include "vtkSlicerSequencesLogic.h"

// MRMLSequence includes
#include "vtkMRMLLinearTransformSequenceStorageNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceStorageNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVectorVolumeNode.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

#ifdef ENABLE_PERFORMANCE_PROFILING
#include "vtkTimerLog.h"
#endif

// STD includes
#include <sstream>
#include <algorithm>

static const char IMAGE_NODE_BASE_NAME[]="Image";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMetafileImporterLogic);

//----------------------------------------------------------------------------
vtkSlicerMetafileImporterLogic
::vtkSlicerMetafileImporterLogic()
{
  this->SequencesLogic = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerMetafileImporterLogic::~vtkSlicerMetafileImporterLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic::SetSequencesLogic(vtkSlicerSequencesLogic* sequencesLogic)
{
  this->SequencesLogic=sequencesLogic;
}

//---------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic::RegisterNodes()
{
  if (this->GetMRMLScene()==NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene()==NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//----------------------------------------------------------------------------
// Read the spacing and dimentions of the image.
vtkMRMLSequenceNode* vtkSlicerMetafileImporterLogic::ReadSequenceMetafileImages(const std::string& fileName,
  const std::string &baseNodeName, std::map< int, std::string >& frameNumberToIndexValueMap)
{
#ifdef ENABLE_PERFORMANCE_PROFILING
  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
#endif
  vtkNew< vtkMetaImageReader > imageReader;
  imageReader->SetFileName( fileName.c_str() );
  imageReader->Update();
#ifdef ENABLE_PERFORMANCE_PROFILING
  timer->StopTimer();
  vtkInfoMacro("Image reading: " << timer->GetElapsedTime() << "sec\n");
#endif

  // check for loading error
  int imageExtent[6] = { 0, -1, 0, -1, 0, -1 };
  imageReader->GetDataExtent(imageExtent);
  if (imageExtent[0] == 0 && imageExtent[1] == 0
    && imageExtent[2] == 0 && imageExtent[3] == 0
    && imageExtent[4] == 0 && imageExtent[5] == 0)
  {
    // loading error
    // (if there is a loading error then all the extents are set to 0
    // although it corresponds to an 1x1x1 image size)
    return NULL;
  }
  if ( imageExtent[0]>imageExtent[1]
    || imageExtent[2]>imageExtent[3]
    || imageExtent[4]>imageExtent[5] )
  {
    // empty image
    return NULL;
  }

  // Create sequence node
  vtkSmartPointer<vtkMRMLSequenceNode> imagesSequenceNode = vtkSmartPointer<vtkMRMLSequenceNode>::New();
  this->GetMRMLScene()->AddNode(imagesSequenceNode);
  imagesSequenceNode->SetIndexName("time");
  imagesSequenceNode->SetIndexUnit("s");
  std::string imagesSequenceName = vtkMRMLSequenceStorageNode::GetSequenceNodeName(baseNodeName, IMAGE_NODE_BASE_NAME);
  imagesSequenceNode->SetName( this->GetMRMLScene()->GenerateUniqueName(imagesSequenceName).c_str() );

  int imagesSequenceNodeDisableModify = imagesSequenceNode->StartModify();

  // Grab the image data from the mha file
  vtkImageData* imageData = imageReader->GetOutput();
  vtkSmartPointer<vtkImageData> emptySliceImageData=vtkSmartPointer<vtkImageData>::New();
  int* dimensions = imageData->GetDimensions();
  emptySliceImageData->SetDimensions(dimensions[0],dimensions[1],1);
  double* spacing = imageData->GetSpacing();
  emptySliceImageData->SetSpacing(spacing[0],spacing[1],1);
  emptySliceImageData->SetOrigin(0,0,0);

  int sliceSize=imageData->GetIncrements()[2];
  for ( int frameNumber = 0; frameNumber < dimensions[2]; frameNumber++ )
  {
    // Add the image slice to scene as a volume

    vtkSmartPointer< vtkMRMLScalarVolumeNode > slice;
    if (imageData->GetNumberOfScalarComponents() > 1)
    {
      slice = vtkSmartPointer< vtkMRMLVectorVolumeNode >::New();
    }
    else
    {
      slice = vtkSmartPointer< vtkMRMLScalarVolumeNode >::New();
    }

    vtkSmartPointer<vtkImageData> sliceImageData=vtkSmartPointer<vtkImageData>::New();
    sliceImageData->DeepCopy(emptySliceImageData);

    sliceImageData->AllocateScalars(imageData->GetScalarType(), imageData->GetNumberOfScalarComponents());

    unsigned char* startPtr=(unsigned char*)imageData->GetScalarPointer(0, 0, frameNumber);
    memcpy(sliceImageData->GetScalarPointer(), startPtr, sliceSize);

    // Generating a unique name is important because that will be used to generate the filename by default
    std::ostringstream nameStr;
    nameStr << IMAGE_NODE_BASE_NAME << std::setw(4) << std::setfill('0') << frameNumber << std::ends;
    slice->SetName(nameStr.str().c_str());
    slice->SetAndObserveImageData(sliceImageData);

    std::string paramValueString = frameNumberToIndexValueMap[frameNumber];
    slice->SetHideFromEditors(false);
    imagesSequenceNode->SetDataNodeAtValue(slice, paramValueString.c_str() );
  }

  imagesSequenceNode->EndModify(imagesSequenceNodeDisableModify);
  imagesSequenceNode->Modified();
  return imagesSequenceNode;
}

//----------------------------------------------------------------------------
void vtkSlicerMetafileImporterLogic::WriteSequenceMetafileImages(const std::string& fileName, vtkMRMLSequenceNode* imageSequenceNode, vtkMRMLSequenceNode* masterSequenceNode)
{
  if ( masterSequenceNode == NULL )
  {
    return;
  }
  if ( imageSequenceNode == NULL || imageSequenceNode->GetNumberOfDataNodes() == 0 )
  {
    return; // Nothing to do if there are zero slices
  }

  // Otherwise, we can grab the slice extents from the first frame
  vtkMRMLVolumeNode* sliceNode = vtkMRMLVolumeNode::SafeDownCast( imageSequenceNode->GetNthDataNode( 0 ) ); // Must have at least one data node
  if ( sliceNode == NULL || sliceNode->GetImageData() == NULL )
  {
    return;
  }
  vtkImageData* sliceImageData = sliceNode->GetImageData();
  if ( sliceImageData->GetDimensions()[ 0 ] == 0 || sliceImageData->GetDimensions()[ 1 ] == 0 || sliceImageData->GetDimensions()[ 2 ] != 1 )
  {
    return;
  }

  // Allocate the output image data from the first slice
  vtkNew< vtkImageData > imageData;
  int* sliceDimensions = sliceImageData->GetDimensions();
  double* sliceSpacing = sliceImageData->GetSpacing();
  imageData->SetDimensions( sliceDimensions[ 0 ], sliceDimensions[ 1 ], imageSequenceNode->GetNumberOfDataNodes() );
  imageData->SetSpacing( sliceSpacing[ 0 ], sliceSpacing[ 1 ], sliceSpacing[ 2 ] );
  imageData->SetOrigin( 0, 0, 0 );

  imageData->AllocateScalars(sliceImageData->GetScalarType(), sliceImageData->GetNumberOfScalarComponents());

  int sliceSize=sliceImageData->GetIncrements()[2];
  for ( int frameNumber = 0; frameNumber < imageSequenceNode->GetNumberOfDataNodes(); frameNumber++ ) // Iterate including the first frame
  {
    std::string indexValue = masterSequenceNode->GetNthIndexValue( frameNumber );

    // Check the image slice is OK
    sliceNode = vtkMRMLVolumeNode::SafeDownCast( imageSequenceNode->GetDataNodeAtValue( indexValue.c_str() ) );
    if ( sliceNode == NULL || sliceNode->GetImageData() == NULL )
    {
      return;
    }
    sliceImageData = sliceNode->GetImageData();
    if ( sliceImageData->GetDimensions()[ 0 ] != sliceDimensions[ 0 ] || sliceImageData->GetDimensions()[ 1 ] != sliceDimensions[ 1 ] || sliceImageData->GetDimensions()[ 2 ] != 1
      || sliceImageData->GetSpacing()[ 0 ] != sliceSpacing[ 0 ] || sliceImageData->GetSpacing()[ 1 ] != sliceSpacing[ 1 ] || sliceImageData->GetSpacing()[ 2 ] != sliceSpacing[ 2 ] )
    {
      return;
    }

    unsigned char* startPtr=(unsigned char*)imageData->GetScalarPointer(0, 0, frameNumber);
    memcpy(startPtr, sliceImageData->GetScalarPointer(), sliceImageData->GetScalarSize() * sliceSize);
  }

#ifdef ENABLE_PERFORMANCE_PROFILING
  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
#endif
  vtkNew< vtkMetaImageWriter > imageWriter;
  imageWriter->SetInputData( imageData.GetPointer() );
  imageWriter->SetFileName( fileName.c_str() ); // This automatically takes care of the file extensions. Note: Saving to .mhd is way faster than saving to .mha
  imageWriter->SetCompression( false );
  imageWriter->Write();
#ifdef ENABLE_PERFORMANCE_PROFILING
  timer->StopTimer();
  vtkInfoMacro("Image writing: " << timer->GetElapsedTime() << "sec\n");
#endif
}

//----------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkSlicerMetafileImporterLogic::ReadSequenceFile(const std::string& fileName, vtkCollection* addedSequenceNodes/*=NULL*/)
{
  // Map the frame numbers to timestamps
  std::map< int, std::string > frameNumberToIndexValueMap;

  std::string fileNameName = vtksys::SystemTools::GetFilenameName(fileName);

  std::string extension = vtksys::SystemTools::GetFilenameLastExtension(fileName);
  extension = vtksys::SystemTools::LowerCase(extension);

  SequenceFileType fileType = INVALID_SEQUENCE_FILE;
  if (extension == ".mha" || extension == ".mhd")
  {
    fileType = METAIMAGE_SEQUENCE_FILE;
  }
  else if (extension == ".nrrd")
  {
    fileType = NRRD_SEQUENCE_FILE;
  }

  if (addedSequenceNodes)
  {
    addedSequenceNodes->RemoveAllItems();
  }

#ifdef ENABLE_PERFORMANCE_PROFILING
  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
#endif
  std::deque< vtkSmartPointer<vtkMRMLSequenceNode> > createdSequenceNodes; // first one is the master
  std::map< std::string, std::string > imageMetaData;
  if (vtkMRMLLinearTransformSequenceStorageNode::ReadSequenceFileTransforms(fileName, this->GetMRMLScene(), createdSequenceNodes,
    frameNumberToIndexValueMap, imageMetaData, fileType) == 0)
  {
    // error is logged in ReadTransforms
    vtkWarningMacro("No transforms read from metafile: " << fileName);
  }

  if (fileType == METAIMAGE_SEQUENCE_FILE && imageMetaData.find("NDims") != imageMetaData.end())
  {
    std::stringstream nDimSS;
    nDimSS << imageMetaData["NDims"];
    int nDims = 0;
    nDimSS >> nDims;
    if (nDims < 1 || nDims > 3)
    {
      vtkErrorMacro("Cannot load metafile with " << nDims << " dimensions. Number of dimensions must be between 1 and 3");
      return NULL;
    }
  }

#ifdef ENABLE_PERFORMANCE_PROFILING
  timer->StopTimer();
  vtkInfoMacro("ReadTransforms time: " << timer->GetElapsedTime() << "sec\n");
  timer->StartTimer();
#endif
  std::string imageBaseNodeName = vtkMRMLSequenceStorageNode::GetSequenceBaseName(fileNameName, IMAGE_NODE_BASE_NAME);
#ifdef ENABLE_PERFORMANCE_PROFILING
  timer->StopTimer();
  vtkInfoMacro("ReadImages time: " << timer->GetElapsedTime() << "sec\n");
#endif

  // Get the shortest base name for all nodes
  std::string shortestBaseNodeName;
  for (std::deque< vtkSmartPointer<vtkMRMLSequenceNode> > ::iterator synchronizedNodesIt = createdSequenceNodes.begin();
    synchronizedNodesIt != createdSequenceNodes.end(); ++synchronizedNodesIt)
  {
    const char* sourceName = (*synchronizedNodesIt)->GetAttribute("Sequences.Source");
    if (!sourceName)
    {
      continue;
    }
    std::string baseNodeName = vtkMRMLSequenceStorageNode::GetSequenceBaseName(fileNameName, sourceName);
    if (shortestBaseNodeName.empty() || baseNodeName.size() < shortestBaseNodeName.size())
    {
      shortestBaseNodeName = baseNodeName;
    }
  }

  // For the user's convenience, add a browser node and show the volume in the slice viewer.
  // If a browser node by that exact name exists already then we reuse that to browse all the nodes together.
  vtkSmartPointer<vtkMRMLSequenceBrowserNode> sequenceBrowserNode;
  vtkMRMLSequenceNode* createdImageNode = NULL;
  if (fileType == METAIMAGE_SEQUENCE_FILE)
  {
    createdImageNode = this->ReadSequenceMetafileImages(fileName, imageBaseNodeName, frameNumberToIndexValueMap);
    if (createdImageNode)
    {
      // push to front as we prefer the image to be the master node
      createdSequenceNodes.push_front(createdImageNode);
    }
    vtkSmartPointer<vtkCollection> foundSequenceBrowserNodes = vtkSmartPointer<vtkCollection>::Take(
      this->GetMRMLScene()->GetNodesByClassByName("vtkMRMLSequenceBrowserNode", shortestBaseNodeName.c_str()));
    if (foundSequenceBrowserNodes->GetNumberOfItems() > 0)
    {
      sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(foundSequenceBrowserNodes->GetItemAsObject(0));
    }
    if (sequenceBrowserNode.GetPointer() == NULL)
    {
      sequenceBrowserNode = vtkSmartPointer<vtkMRMLSequenceBrowserNode>::New();
      sequenceBrowserNode->SetName(this->GetMRMLScene()->GenerateUniqueName(shortestBaseNodeName).c_str());
      this->GetMRMLScene()->AddNode(sequenceBrowserNode);
    }
  }
  else if (fileType == NRRD_SEQUENCE_FILE)
  {
    sequenceBrowserNode = this->ReadVolumeSequence(fileName, addedSequenceNodes);
    if (sequenceBrowserNode)
    {
      createdImageNode = sequenceBrowserNode->GetMasterSequenceNode();
      if (createdImageNode)
      {
        // Update frame values for the indexes
        createdImageNode->SetIndexName("time");
        createdImageNode->SetIndexUnit("s");
        for (int frameNumber = 0; frameNumber < createdImageNode->GetNumberOfDataNodes(); frameNumber++)
        {
          std::stringstream frameNumberSS;
          frameNumberSS << frameNumber;
          createdImageNode->UpdateIndexValue(frameNumberSS.str(), frameNumberToIndexValueMap[frameNumber]);
        }
      }
    }
  }

  // Save image metadata into node attributes
  if (createdImageNode)
  {
    for (std::map< std::string, std::string >::iterator imageMetaDataIt = imageMetaData.begin(); imageMetaDataIt != imageMetaData.end(); ++imageMetaDataIt)
    {
      std::string attributeName = "Sequences.";
      attributeName += imageMetaDataIt->first;
      createdImageNode->SetAttribute(attributeName.c_str(), imageMetaDataIt->second.c_str());
    }
    createdImageNode->SetAttribute("Sequences.Source", IMAGE_NODE_BASE_NAME);
  }

  if (createdSequenceNodes.empty())
  {
    vtkWarningMacro("No readable nodes found in sequence metafile");
    return NULL;
  }

  if (sequenceBrowserNode)
  {
    for (std::deque< vtkSmartPointer<vtkMRMLSequenceNode> > ::iterator synchronizedNodesIt = createdSequenceNodes.begin();
      synchronizedNodesIt != createdSequenceNodes.end(); ++synchronizedNodesIt)
    {
      if (addedSequenceNodes)
      {
        addedSequenceNodes->AddItem(*synchronizedNodesIt);
      }
      sequenceBrowserNode->AddSynchronizedSequenceNode((*synchronizedNodesIt)->GetID());
      if (vtkMRMLVolumeNode::SafeDownCast((*synchronizedNodesIt)->GetNthDataNode(0)))
      {
        // Image
        // If save changes are allowed then proxy nodes are updated using shallow copy, which is much faster for images
        // (and images are usually not modified, so the risk of accidentally modifying data in the sequence is low).
        sequenceBrowserNode->SetSaveChanges(*synchronizedNodesIt, true);
        // Show image in slice viewers
        vtkMRMLVolumeNode* imageProxyVolumeNode = vtkMRMLVolumeNode::SafeDownCast(sequenceBrowserNode->GetProxyNode(*synchronizedNodesIt));
        if (imageProxyVolumeNode)
        {
          vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
          vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : 0;
          if (appLogic && selectionNode)
          {
            selectionNode->SetReferenceActiveVolumeID(imageProxyVolumeNode->GetID());
            appLogic->PropagateVolumeSelection();
            appLogic->FitSliceToAll();
          }
        }
      }
      else
      {
        // Prevent accidental overwriting of transforms
        sequenceBrowserNode->SetSaveChanges(*synchronizedNodesIt, false);
      }
    }
  }
  return sequenceBrowserNode.GetPointer();
}

//----------------------------------------------------------------------------
bool vtkSlicerMetafileImporterLogic::WriteSequenceMetafile(const std::string& fileName, vtkMRMLSequenceBrowserNode* browserNode)
{
  if (browserNode == NULL)
  {
    vtkWarningMacro( "vtkSlicerMetafileImporterLogic::WriteSequenceMetafile: Could not write to file, browser node not specified.");
    return false;
  }
  vtkMRMLSequenceNode* masterSequenceNode = browserNode->GetMasterSequenceNode();
  if (masterSequenceNode == NULL)
  {
    vtkWarningMacro( "vtkSlicerMetafileImporterLogic::WriteSequenceMetafile: Could not write to file, browser node has no master node.");
    return false;
  }

  vtkNew< vtkCollection > sequenceNodes;
  browserNode->GetSynchronizedSequenceNodes(sequenceNodes.GetPointer(), true); // Include the master node (since it is probably the image sequence)

  // Find the image sequence node
  vtkMRMLSequenceNode* imageNode = NULL;
  for ( int i = 0; i < sequenceNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodes->GetItemAsObject( i ) );
    if ( currSequenceNode == NULL )
    {
      continue;
    }
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast( currSequenceNode->GetNthDataNode( 0 ) );
    if ( volumeNode != NULL )
    {
      imageNode = currSequenceNode;
      break;
    }
  }

  // Find all of the transform nodes
  std::deque< vtkMRMLSequenceNode* > transformNodes;
  std::deque< std::string > transformNames;
  for (int i = 0; i < sequenceNodes->GetNumberOfItems(); i++)
  {
    vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodes->GetItemAsObject( i ) );
    if ( currSequenceNode == NULL )
    {
      continue;
    }
    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast( currSequenceNode->GetNthDataNode( 0 ) );
    if ( transformNode == NULL ) // Check if the data nodes are a subclass of vtkMRMLTransformNode. This is better than looking at just the class name, because we can't tell inheritance that way.
    {
      continue;
    }
    transformNodes.push_back( currSequenceNode );
    vtkMRMLNode* proxyNode = browserNode->GetProxyNode(currSequenceNode);
    if (proxyNode)
    {
      const char* transformName = currSequenceNode->GetAttribute("Sequences.Source");
      if (transformName == NULL)
      {
        transformName = proxyNode->GetAttribute("Sequences.BaseName");
      }
      if (transformName == NULL)
      {
        transformName = proxyNode->GetName();
      }
      transformNames.push_back(transformName ? transformName : "");
    }
    else
    {
      transformNames.push_back(currSequenceNode->GetName());
    }

  }

  // Need to write the images first so the header file is generated by the vtkMetaImageWriter
  // Then, we can append the transforms to the header
  this->WriteSequenceMetafileImages(fileName, imageNode, masterSequenceNode);
  vtkMRMLLinearTransformSequenceStorageNode::WriteSequenceMetafileTransforms(fileName, transformNodes, transformNames, masterSequenceNode, imageNode);

  return true;
}


//----------------------------------------------------------------------------
vtkMRMLSequenceBrowserNode* vtkSlicerMetafileImporterLogic::ReadVolumeSequence(const std::string& fileName, vtkCollection* addedSequenceNodes/*=NULL*/)
{
  if (addedSequenceNodes)
  {
    addedSequenceNodes->RemoveAllItems();
  }

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == NULL)
  {
    vtkErrorMacro("vtkSlicerMetafileImporterLogic::ReadVolumeSequence failed: scene is invalid");
    return NULL;
  }

  vtkNew<vtkMRMLVolumeSequenceStorageNode> storageNode;

  vtkNew<vtkMRMLSequenceNode> volumeSequenceNode;
  std::string volumeName = storageNode->GetFileNameWithoutExtension(fileName.c_str());
  volumeSequenceNode->SetName(scene->GenerateUniqueName(volumeName).c_str());
  scene->AddNode(volumeSequenceNode.GetPointer());
  if (addedSequenceNodes)
  {
    addedSequenceNodes->AddItem(volumeSequenceNode.GetPointer());
  }

  //storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  scene->AddNode(storageNode.GetPointer());
  volumeSequenceNode->SetAndObserveStorageNodeID(storageNode->GetID());

  if (scene->GetCacheManager() && this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(fileName.c_str()))
    {
    vtkDebugMacro("AddArchetypeVolume: input filename '" << fileName << "' is a URI");
    // need to set the scene on the storage node so that it can look for file handlers
    storageNode->SetURI(fileName.c_str());
    //storageNode->SetScene(this->GetMRMLScene());
    }
  else
    {
      storageNode->SetFileName(fileName.c_str());
    }

  bool success = storageNode->ReadData(volumeSequenceNode.GetPointer());

  if (!success)
  {
    scene->RemoveNode(storageNode.GetPointer());
    scene->RemoveNode(volumeSequenceNode.GetPointer());
    return NULL;
  }


  // For the user's convenience, add a browser node and show the volume in the slice viewer.
  // If a browser node by that exact name exists already then we reuse that to browse all the nodes together.
  std::string fileNameName = vtksys::SystemTools::GetFilenameName(fileName);
  std::string baseNodeName = vtkMRMLSequenceStorageNode::GetSequenceBaseName(fileNameName, IMAGE_NODE_BASE_NAME);
  vtkSmartPointer<vtkCollection> foundSequenceBrowserNodes = vtkSmartPointer<vtkCollection>::Take(
    this->GetMRMLScene()->GetNodesByClassByName("vtkMRMLSequenceBrowserNode", baseNodeName.c_str()));
  vtkSmartPointer<vtkMRMLSequenceBrowserNode> sequenceBrowserNode;
  if (foundSequenceBrowserNodes->GetNumberOfItems() > 0)
  {
    // there are already sequence nodes (and a browser node) in the scene from the same acquisition
    sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(foundSequenceBrowserNodes->GetItemAsObject(0));
  }
  if (sequenceBrowserNode.GetPointer() == NULL)
  {
    sequenceBrowserNode = vtkSmartPointer<vtkMRMLSequenceBrowserNode>::New();
    if (volumeName == baseNodeName)
    {
      // Make the volume name distinguishable from the sequence node name
      baseNodeName += " browser";
    }
    sequenceBrowserNode->SetName(this->GetMRMLScene()->GenerateUniqueName(baseNodeName).c_str());
    this->GetMRMLScene()->AddNode(sequenceBrowserNode);
  }

  sequenceBrowserNode->AddSynchronizedSequenceNodeID(volumeSequenceNode->GetID());

  // If save changes are allowed then proxy nodes are updated using shallow copy, which is much faster for images
  sequenceBrowserNode->SetSaveChanges(volumeSequenceNode.GetPointer(), true);

  // Show output volume in the slice viewer
  vtkMRMLVolumeNode* volumeProxyNode = vtkMRMLVolumeNode::SafeDownCast(sequenceBrowserNode->GetProxyNode(volumeSequenceNode.GetPointer()));
  if (volumeProxyNode)
  {
    vtkSlicerApplicationLogic* appLogic = this->GetApplicationLogic();
    vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : 0;
    if (selectionNode)
    {
      selectionNode->SetReferenceActiveVolumeID(volumeProxyNode->GetID());
      if (appLogic)
      {
        appLogic->PropagateVolumeSelection();
        appLogic->FitSliceToAll();
      }
    }
  }

  return sequenceBrowserNode.GetPointer();
}
