
#ifndef __vtkMRMLRemoteExecNode_h
#define __vtkMRMLRemoteExecNode_h


#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerOpenIGTLinkRemoteModuleMRMLExport.h"

#include <string>
#include <queue>

class vtkCallbackCommand;


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

 protected:
  vtkMRMLRemoteExecNode();
  ~vtkMRMLRemoteExecNode(){};
  vtkMRMLRemoteExecNode(const vtkMRMLRemoteExecNode&);
  void operator=(const vtkMRMLRemoteExecNode&);
  

 public:
  
  void PushOutgoingCommand(const char* name);
  const char* PopOutgoingCommand();
  void PushIncomingCommand(const char* name);
  const char* PopIncomingCommand();

 protected:
  //BTX
  std::queue<std::string> InCommandQueue;
  std::queue<std::string> OutCommandQueue;
  std::string InCommand;
  std::string OutCommand;
  //ETX

};

#endif
