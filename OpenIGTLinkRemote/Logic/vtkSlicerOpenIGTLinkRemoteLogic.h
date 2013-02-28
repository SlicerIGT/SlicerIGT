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

// .NAME vtkSlicerOpenIGTLinkRemoteLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerOpenIGTLinkRemoteLogic_h
#define __vtkSlicerOpenIGTLinkRemoteLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerOpenIGTLinkRemoteModuleLogicExport.h"

class vtkSlicerOpenIGTLinkIFLogic;
class vtkIGTLToMRMLAnnotationText;



/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_LOGIC_EXPORT vtkSlicerOpenIGTLinkRemoteLogic :
  public vtkSlicerModuleLogic
{
public:
  
  enum
  {
    REPLY_SUCCESS,
    REPLY_FAIL,
    REPLY_WAITING
  };
  
  static vtkSlicerOpenIGTLinkRemoteLogic *New();
  vtkTypeMacro(vtkSlicerOpenIGTLinkRemoteLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetIFLogic( vtkSlicerOpenIGTLinkIFLogic* ifLogic );
  
  int ExecuteCommand( const char* connectorNodeId, std::string commandName, std::string parameters );
  int GetCommandReply( int commandId, std::string &message );
  void DiscardCommand( int commandId );
  
  int SendCommand( std::string strCommand, const char* connectorNodeId );
  
  
protected:
  vtkSlicerOpenIGTLinkRemoteLogic();
  virtual ~vtkSlicerOpenIGTLinkRemoteLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerOpenIGTLinkRemoteLogic(const vtkSlicerOpenIGTLinkRemoteLogic&); // Not implemented
  void operator=(const vtkSlicerOpenIGTLinkRemoteLogic&);               // Not implemented
  
  class vtkInternal;
  vtkInternal* Internal;
  
  vtkIGTLToMRMLAnnotationText* CommandConverter;
  
  int CommandCounter;
};

#endif
