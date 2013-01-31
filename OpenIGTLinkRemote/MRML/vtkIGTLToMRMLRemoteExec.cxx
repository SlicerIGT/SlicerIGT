
#include <vtksys/SystemTools.hxx>

#include "vtkCommand.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLRemoteExecNode.h"
#include "vtkIGTLToMRMLRemoteExec.h"

vtkStandardNewMacro(vtkIGTLToMRMLRemoteExec);
vtkCxxRevisionMacro(vtkIGTLToMRMLRemoteExec, "$Revision: 1.0 $");



vtkIGTLToMRMLRemoteExec
::vtkIGTLToMRMLRemoteExec()
{
  this->IGTLNames.clear();
}



vtkIGTLToMRMLRemoteExec
::~vtkIGTLToMRMLRemoteExec()
{
}



void vtkIGTLToMRMLRemoteExec::PrintSelf( ostream& os, vtkIndent indent )
{
}



vtkMRMLNode* vtkIGTLToMRMLRemoteExec
::CreateNewNode( vtkMRMLScene* scene, const char* name )
{
  vtkMRMLRemoteExecNode* remoteExecNode;

  remoteExecNode = vtkMRMLRemoteExecNode::New();
  remoteExecNode->SetName( name );
  remoteExecNode->SetDescription( "Created by OpenIGTLinkRemote module" );

  scene->AddNode( remoteExecNode );  

  return remoteExecNode;
}



vtkIntArray* vtkIGTLToMRMLRemoteExec
::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  return events;
}



int vtkIGTLToMRMLRemoteExec
::IGTLToMRML( igtl::MessageBase::Pointer buffer, vtkMRMLNode* node )
{
  return 1;
}



int vtkIGTLToMRMLRemoteExec
::MRMLToIGTL( unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg )
{
  if ( mrmlNode == NULL || event != vtkCommand::ModifiedEvent )
  {
    return 0;
  }
  
  vtkMRMLRemoteExecNode* remoteExecNode = vtkMRMLRemoteExecNode::SafeDownCast( mrmlNode );
  if ( remoteExecNode == NULL )
  {
    return 0;
  }
  
  /*
  std::string command = remoteExecNode->PopOutgoingCommand();
  if ( command.size() > 0 )
  {
    if ( ! remoteExecNode->GetScene() )
    {
      return 0;
    }
    
    if ( this->StringMsg.IsNull() )
    {
      this->StringMsg = igtl::StringMessage::New();
      this->StringMsg->SetDeviceName( mrmlNode->GetName() );
    }
    
    this->StringMsg->SetString( command );
    this->StringMsg->Pack();
    
    *size = this->StringMsg->GetPackSize();
    *igtlMsg = (void*)this->StringMsg->GetPackPointer();
    return 1;
  }
  else
  {
    return 0;
  }
  */
  
  return 0;
}

