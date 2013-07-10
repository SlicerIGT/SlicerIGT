#include "vtkIGTLToMRMLAnnotationText.h"
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkSlicerOpenIGTLinkIFLogic.h"
#include "vtkSlicerOpenIGTLinkRemoteLogic.h"
#include <cassert>
#include <sstream>
#include <string>
#include <vtkNew.h>

//----------------------------------------------------------------------------

class vtkSlicerOpenIGTLinkRemoteLogic::vtkInternal
{
public:
  vtkInternal();

  vtkSlicerOpenIGTLinkIFLogic* IFLogic;
};

vtkSlicerOpenIGTLinkRemoteLogic::vtkInternal::vtkInternal()
: IFLogic(NULL)
{
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerOpenIGTLinkRemoteLogic);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkRemoteLogic::vtkSlicerOpenIGTLinkRemoteLogic()
{
  this->Internal = new vtkInternal;
  this->CommandConverter = vtkIGTLToMRMLAnnotationText::New();
  this->CommandCounter = 0;
}

//----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkRemoteLogic::~vtkSlicerOpenIGTLinkRemoteLogic()
{
  delete this->Internal;
  
  if ( this->CommandConverter != NULL )
  {
    this->CommandConverter->Delete();
    this->CommandConverter = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::SetIFLogic( vtkSlicerOpenIGTLinkIFLogic* ifLogic )
{
  this->Internal->IFLogic = ifLogic;
}

/**
 * @param connectorNodeId Identifies the IGTL connector node that will send this command message.
 * @param commandName Will translate to Name parameter in the Command element (root) of the command message XML text.
 * @param parameters A string in name1="value1" name2="value2" ... format. It will be placed in the Command XML element.
 * @returns CommandId number that identifies this command and its response. Returns 0 on error.
 */
int vtkSlicerOpenIGTLinkRemoteLogic::ExecuteCommand( const char* connectorNodeId, std::string commandName, std::string parameters )
{
  std::string messageString = "<Command Name=\"" + commandName + "\" " + parameters + " />";
  return this->SendCommand( messageString, connectorNodeId );
}

/**
 * @returns status of the command reply
 */
vtkSlicerOpenIGTLinkRemoteLogic::REPLY_RESULT vtkSlicerOpenIGTLinkRemoteLogic::GetCommandReply( int commandId, std::string &message )
{
  if ( this->GetMRMLScene() == NULL )
  {
    return REPLY_FAIL;
  }
  
  std::stringstream ss;
  ss << "RC" << commandId << "Reply";
  vtkCollection* replyNodes = this->GetMRMLScene()->GetNodesByName( ss.str().c_str() );
  
  int replyCount = replyNodes->GetNumberOfItems();
  if ( replyCount < 1 )
  {
    replyNodes->Delete();
    return REPLY_WAITING;
  }
  
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast( replyNodes->GetItemAsObject( 0 ) );
  if ( textNode == NULL )
  {
    replyNodes->Delete();
    vtkErrorMacro( "Could not cast reply node to vtkMRMLAnnotationTextNode!" );
    return REPLY_FAIL;
  }
  
  std::string reply = std::string( textNode->GetText( 0 ).c_str() );
  REPLY_RESULT status = reply.find("SUCCESS::") != std::string::npos ? REPLY_SUCCESS : REPLY_FAIL;
  message = reply.substr(reply.find("::")+2);
  
  replyNodes->Delete();
  return status;
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::DiscardCommand( int commandId )
{
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  std::stringstream ss;
  ss << "RC" << commandId << "Reply";
  vtkCollection* replyNodes = this->GetMRMLScene()->GetNodesByName( ss.str().c_str() );
  for( int i = 0; i < replyNodes->GetNumberOfItems(); ++i )
  {
    vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast( replyNodes->GetItemAsObject(i) );
    if ( textNode == NULL )
    {
      vtkErrorMacro( "Could not cast reply node to vtkMRMLAnnotationTextNode!" );
      continue;
    }
    this->GetMRMLScene()->RemoveNode(textNode);
  }
  replyNodes->Delete();

  ss.clear();
  ss << "RC" << commandId;
  vtkCollection* commandNodes = this->GetMRMLScene()->GetNodesByName( ss.str().c_str() );
  for( int i = 0; i < replyNodes->GetNumberOfItems(); ++i )
  {
    vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast( replyNodes->GetItemAsObject(i) );
    if ( textNode == NULL )
    {
      vtkErrorMacro( "Could not cast reply node to vtkMRMLAnnotationTextNode!" );
      continue;
    }
    this->GetMRMLScene()->RemoveNode(textNode);
  }
  commandNodes->Delete();

  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
}

/**
 * @param strCommand string that will be sent in the command IGTL message.
 * @param connectorNodeId Identifies the IGTL connector node that will send the command message to the server.
 * @returns CommandId that can be used later to get the response for the created. Returns 0 on error.
 */
int vtkSlicerOpenIGTLinkRemoteLogic::SendCommand( std::string strCommand, const char* connectorNodeId )
{
  if ( this->GetMRMLScene() == NULL )
  {
    vtkErrorMacro( "MRML Scene is invalid" );
    return 0;
  }
  
  vtkMRMLIGTLConnectorNode* connectorNode = vtkMRMLIGTLConnectorNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( connectorNodeId ) );
  if ( connectorNode == NULL )
  {
    vtkErrorMacro( "SendCommand could not cast MRML node to IGTLConnectorNode." );
    return 0;
  }
  
  // Create annotation text node for this command.
  
  vtkSmartPointer< vtkMRMLAnnotationTextNode > newNode = vtkSmartPointer< vtkMRMLAnnotationTextNode >::New();
  newNode->Initialize( this->GetMRMLScene() );
  newNode->SetText( 0, strCommand.c_str(), 1, 0 );
  
  // Giving unique name to this new text node.
  std::stringstream ss;
  this->CommandCounter ++; // Create a unique Id for this command message.
  ss << "RC" << this->CommandCounter;
  newNode->SetName( ss.str().c_str() );
  
  connectorNode->RegisterOutgoingMRMLNode( newNode );
  newNode->Modified();
  
  return this->CommandCounter;
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  
  if (this->Internal->IFLogic)
    // Register IGTL message converter.
    this->Internal->IFLogic->RegisterMessageConverter( this->CommandConverter );
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}
