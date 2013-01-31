
#ifndef __vtkMRMLRemoteExecNode_h
#define __vtkMRMLRemoteExecNode_h


#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerOpenIGTLinkRemoteModuleMRMLExport.h"

#include <string>
#include <queue>

class vtkCallbackCommand;

//BTX
struct RemoteExecCommand
{
  enum StatusType {
    WAIT_FOR_SENDING = 0,
    WAIT_FOR_COMPLETION = 1,
    COMPLETED = 2,
    WAIT_FOR_DELETE = 3
  };

  std::string  CommandString;
  std::string  ReplyString;
  int          Id;
  StatusType   Status;
};
//ETX


class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_MRML_EXPORT vtkMRMLRemoteExecNode : public vtkMRMLNode
{
 public:
  static vtkMRMLRemoteExecNode *New();
  vtkTypeMacro(vtkMRMLRemoteExecNode,vtkMRMLNode);

  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "RemoteExec";};
  
  int GetNextCommandWaitForSending();
  
  
 protected:
  vtkMRMLRemoteExecNode();
  ~vtkMRMLRemoteExecNode(){};
  vtkMRMLRemoteExecNode(const vtkMRMLRemoteExecNode&);
  void operator=(const vtkMRMLRemoteExecNode&);
  

 public:
  
  int PushOutgoingCommand( const char* name );
  void SetCommandReply( int commandId, const char* reply );
  
 protected:
 
  RemoteExecCommand* GetCommandById( int id );
  
  //BTX
  std::deque< RemoteExecCommand > CommandQueue;
  //ETX
  
  int LastCommandId;
};

#endif
