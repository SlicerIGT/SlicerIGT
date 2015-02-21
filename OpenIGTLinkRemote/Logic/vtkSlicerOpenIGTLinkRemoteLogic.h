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

// .NAME vtkSlicerOpenIGTLinkRemoteLogic - slicer logic class for remote control of PLUS server
// .SECTION Description
// This class managed the interactions with a PLUS server via an OpenIGTLinkIF connector


#ifndef __vtkSlicerOpenIGTLinkRemoteLogic_h
#define __vtkSlicerOpenIGTLinkRemoteLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerOpenIGTLinkRemoteModuleLogicExport.h"
#include <cstdlib>

class vtkMRMLIGTLQueryNode;
class vtkSlicerOpenIGTLinkIFLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_LOGIC_EXPORT vtkSlicerOpenIGTLinkRemoteLogic :
  public vtkSlicerModuleLogic
{
public:
  enum COMMAND_RESULT
  {
    COMMAND_SUCCESS,
    COMMAND_FAIL,
    COMMAND_WAITING
  };
  
  static vtkSlicerOpenIGTLinkRemoteLogic *New();
  vtkTypeMacro(vtkSlicerOpenIGTLinkRemoteLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetIFLogic( vtkSlicerOpenIGTLinkIFLogic* ifLogic );
  
  /// Creates a command query node and corresponding response node.
  /// It is recommended to reuse the same query node for multiple commands
  /// to avoid the overhead of creating and deleting nodes in the scene at each
  /// command execution.
  vtkMRMLIGTLQueryNode* CreateCommandQueryNode();
  
  /// Deletes a command query node and corresponding response node
  void DeleteCommandQueryNode(vtkMRMLIGTLQueryNode* commandQueryNode);

  /// Sends a command defined by name and set of attributes
  /// @param commandQueryNode Query node that can be used to monitor the status of the command and retrieve the command response.
  /// @param connectorNodeId Identifies the IGTL connector node that will send this command message.
  /// @param commandName Will translate to Name parameter in the Command element (root) of the command message XML text.
  /// @param attributes A string in name1="value1" name2="value2" ... format. It will be placed in the Command XML element.
  /// @returns true on success
  bool SendCommand(vtkMRMLIGTLQueryNode* commandQueryNode, const char* connectorNodeId, const char* commandName, const char* attributes);
  
  /// Sends a command defined by an XML string
  /// @param commandQueryNode Query node that can be used to monitor the status of the command and retrieve the command response.
  /// @param connectorNodeId Identifies the IGTL connector node that will send the command message to the server.
  /// @param strCommand XML string that will be sent in the command IGTL message.
  /// @returns true on success
  bool SendCommandXML(vtkMRMLIGTLQueryNode* commandQueryNode, const char* connectorNodeId, const char* commandXml);

  /// Retrieves command response
  COMMAND_RESULT GetCommandResponse(vtkMRMLIGTLQueryNode* commandQueryNode, std::string &message, std::string &attributes);

  /// Cancels command (removes command from the query queue of the association connector).
  /// If command response arrives after the command is cancelled the query node will ignore it.
  void CancelCommand(vtkMRMLIGTLQueryNode* commandQueryNode);
  
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

  // Counter that will be used for generation of unique command IDs
  static int CommandCounter;
};

#endif
