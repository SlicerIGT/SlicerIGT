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
class vtkSlicerOpenIGTLinkCommand;
class vtkSlicerOpenIGTLinkIFLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_LOGIC_EXPORT vtkSlicerOpenIGTLinkRemoteLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerOpenIGTLinkRemoteLogic *New();
  vtkTypeMacro(vtkSlicerOpenIGTLinkRemoteLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetIFLogic( vtkSlicerOpenIGTLinkIFLogic* ifLogic );

  /// Send an OpenIGTLink command
  /// OpenIGTLink STRING command query nodes are automatically created and associated
  /// with the vtkSlicerOpenIGTLinkCommand. If the query node is responded then
  /// the vtkSlicerOpenIGTLinkCommand is updated with the response.
  /// For performance improvement reason, the query nodes are not removed from the scene
  /// but reused for sending the next command. The query nodes are invisible and not saved
  /// with the scene.
  /// If the command is already in progress then the moethod returns with an error (the command
  /// that was already in progress is not changed).
  ///
  /// Example usage from Python:
  ///     cmd = slicer.modulelogic.vtkSlicerOpenIGTLinkCommand()
  ///     cmd.SetCommandName('RequestChannelIds')
  ///     slicer.modules.openigtlinkremote.logic().SendCommand(cmd, 'vtkMRMLIGTLConnectorNode1')
  ///   To get notification about command completion run these before SendCommand:
  ///     def notificationMethod(command,q):
  ///       print "Command completed: ", command.StatusToString(command.GetStatus())
  ///     cmd.AddObserver(slicer.modulelogic.vtkSlicerOpenIGTLinkCommand.CommandCompletedEvent, notificationMethod)
  bool SendCommand(vtkSlicerOpenIGTLinkCommand* command, const char* connectorNodeId);

  /// Cancel a command: removes from the OpenIGTLink connector's query queue, removes the
  /// association with the query node (so that it is reusable for sending another command),
  /// and sets the command state to cancelled.
  bool CancelCommand(vtkSlicerOpenIGTLinkCommand* command);

protected:
  vtkSlicerOpenIGTLinkRemoteLogic();
  virtual ~vtkSlicerOpenIGTLinkRemoteLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  /// Receives all the events fired by the nodes.
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void * callData);

  vtkMRMLIGTLQueryNode* GetCommandQueryNode(vtkSlicerOpenIGTLinkCommand* command);
  void ReleaseCommandQueryNode(vtkMRMLIGTLQueryNode* commandQueryNode);

  /// Creates a command query node and corresponding response node.
  /// It is recommended to reuse the same query node for multiple commands
  /// to avoid the overhead of creating and deleting nodes in the scene at each
  /// command execution.
  vtkMRMLIGTLQueryNode* CreateCommandQueryNode();
  
  /// Deletes a command query node and corresponding response node
  void DeleteCommandQueryNode(vtkMRMLIGTLQueryNode* commandQueryNode);

private:

  vtkSlicerOpenIGTLinkRemoteLogic(const vtkSlicerOpenIGTLinkRemoteLogic&); // Not implemented
  void operator=(const vtkSlicerOpenIGTLinkRemoteLogic&);               // Not implemented
  
  class vtkInternal;
  vtkInternal* Internal;

  // Counter that will be used for generation of unique command IDs
  static int CommandCounter;
};

#endif
