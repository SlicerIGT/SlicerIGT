
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
  
}



void vtkMRMLRemoteExecNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}



void vtkMRMLRemoteExecNode
::PushOutgoingCommand( const char* name )
{
  this->OutCommand = name;
}



const char* vtkMRMLRemoteExecNode
::PopOutgoingCommand()
{
  return this->OutCommand.c_str();
}



void vtkMRMLRemoteExecNode
::PushIncomingCommand( const char* name )
{
  this->InCommand = name;
}



const char* vtkMRMLRemoteExecNode
::PopIncomingCommand()
{
  return this->InCommand.c_str();
}

