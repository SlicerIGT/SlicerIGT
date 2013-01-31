
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkIGTLToMRMLRemoteExec.h"

#include "vtkMRMLRemoteExecNode.h"



vtkMRMLRemoteExecNode* vtkMRMLRemoteExecNode
::New()
{
  vtkMRMLRemoteExecNode* ret;

  // First try to create the object from the vtkObjectFactory
  vtkObject* r = vtkObjectFactory::CreateInstance("vtkMRMLRemoteExecNode");
  if(r)
    {
    ret =  (vtkMRMLRemoteExecNode*)r;
    }
  else
    {
    // If the factory was unable to create the object, then create it here.
    ret =  new vtkMRMLRemoteExecNode;
    }
  
  return ret;
}



vtkMRMLNode* vtkMRMLRemoteExecNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLRemoteExecNode");
  if(ret)
    {
    return (vtkMRMLRemoteExecNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLRemoteExecNode;
}



vtkMRMLRemoteExecNode
::vtkMRMLRemoteExecNode()
{
  this->LastCommandId = 0;
}



void vtkMRMLRemoteExecNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}



/**
 * @returns Id of the first command that is waiting to be sent. 0 if none found.
 */
int vtkMRMLRemoteExecNode
::GetNextCommandWaitForSending()
{
  for ( std::deque< RemoteExecCommand >::iterator cIt = this->CommandQueue.begin(); cIt != this->CommandQueue.end(); ++ cIt )
  {
    if ( (*cIt).Status == RemoteExecCommand::WAIT_FOR_SENDING )
    {
      return (*cIt).Id;
    }
  }
  
  return 0;
}



/**
 * @returns Id of the command for later follow-up.
 */
int vtkMRMLRemoteExecNode
::PushOutgoingCommand( const char* name )
{
  RemoteExecCommand cmd;
  cmd.Status = RemoteExecCommand::WAIT_FOR_SENDING;
  cmd.CommandString = std::string( name );
  ++ this->LastCommandId; // Generate a unique Id for each command.
  cmd.Id = this->LastCommandId;
  this->CommandQueue.push_back( cmd );
  return this->LastCommandId;
}



void vtkMRMLRemoteExecNode
::SetCommandReply( int commandId, const char* reply )
{
  RemoteExecCommand* cmd = this->GetCommandById( commandId );
  
  if ( cmd == NULL )
  {
    vtkErrorMacro( "Failed to find command: " << commandId );
    return;
  }
  
  cmd->ReplyString = std::string( reply );
  cmd->Status = RemoteExecCommand::COMPLETED;
}



RemoteExecCommand* vtkMRMLRemoteExecNode
::GetCommandById( int id )
{
  for ( std::deque< RemoteExecCommand >::iterator cIt = this->CommandQueue.begin(); cIt != this->CommandQueue.end(); ++ cIt )
  {
    if ( (*cIt).Status == RemoteExecCommand::WAIT_FOR_SENDING )
    {
      return &(*cIt);
    }
  }
  
  return 0;
}
