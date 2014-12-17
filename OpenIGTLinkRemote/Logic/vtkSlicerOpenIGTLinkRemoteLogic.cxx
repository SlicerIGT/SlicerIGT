#include "vtkMRMLAnnotationTextNode.h"
#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTextNode.h"
#include "vtkSlicerOpenIGTLinkIFLogic.h"
#include "vtkSlicerOpenIGTLinkRemoteLogic.h"
#include <cassert>
#include <sstream>
#include <string>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLUtilities.h>


// Share a single command counter across all possible logic instances.
int vtkSlicerOpenIGTLinkRemoteLogic::CommandCounter = 0;

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
}

//----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkRemoteLogic::~vtkSlicerOpenIGTLinkRemoteLogic()
{
  delete this->Internal;
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
 * @param attributes A string in name1="value1" name2="value2" ... format. It will be placed in the Command XML element.
 * @returns CommandId number that identifies this command and its response. Returns 0 on error.
 */
int vtkSlicerOpenIGTLinkRemoteLogic::ExecuteCommand( const char* connectorNodeId, const char* commandName, const char* attributes )
{
  std::string messageString = std::string("<Command Name=\"") + (commandName?commandName:"") + "\" " + (attributes?attributes:"") + " />";
  return this->SendCommand( messageString, connectorNodeId );
}

/**
 * @returns status of the command reply
 */
vtkSlicerOpenIGTLinkRemoteLogic::REPLY_RESULT vtkSlicerOpenIGTLinkRemoteLogic::GetCommandReply( int commandId, std::string &message, std::string &attributes)
{
  message.clear();
  attributes.clear();

  if ( this->GetMRMLScene() == NULL )
  {
    return REPLY_FAIL;
  }
  
  std::stringstream ss;
  ss << "ACK_" << commandId;
  vtkSmartPointer<vtkCollection> replyNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByName( ss.str().c_str() ));
  
/*

    // If there are some custom attributes, then return all attributes as attibuteName=attributeValue pairs
    // (otherwise just return the Data attribute value as a simple string)
    if (!attributes.empty())
    {
      message = "Data=\""+message+"\" "+attributes;
    }


*/

  int replyCount = replyNodes->GetNumberOfItems();
  if ( replyCount < 1 )
  {
    return REPLY_WAITING;
  }
  
  vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast( replyNodes->GetItemAsObject( 0 ) );
  if ( textNode == NULL )
  {
    vtkErrorMacro( "Could not cast reply node to vtkMRMLAnnotationTextNode!" );
    return REPLY_FAIL;
  }
  
  std::string reply = std::string( textNode->GetText( 0 ).c_str() );
  
  REPLY_RESULT status = REPLY_FAIL;  
  
  vtkSmartPointer<vtkXMLDataElement> replyElement = vtkSmartPointer<vtkXMLDataElement>::Take( vtkXMLUtilities::ReadElementFromString(reply.c_str()) );
  if (replyElement != NULL)
  {
    // Retrieve status from XML string
    if (replyElement->GetAttribute("Status")==NULL)
    {
      vtkErrorMacro("OpenIGTLink command reply: missing Status attribute: "<<reply);
    }
    else
    {
      if (strcmp(replyElement->GetAttribute("Status"),"SUCCESS")==0)
      {
        status = REPLY_SUCCESS;
      }
      else if (strcmp(replyElement->GetAttribute("Status"),"FAIL")==0)
      {
        status = REPLY_FAIL;
      }
      else
      {
        vtkErrorMacro("OpenIGTLink command reply: invalid Status attribute value: "<<replyElement->GetAttribute("Status"));
      }
    }
    // Retrieve Data element from XML string
    if (replyElement->GetAttribute("Message")!=NULL)
    {
      message=replyElement->GetAttribute("Message");
    }
    // Retrieve other attributes from XML string    
    for (int attrIndex=0; attrIndex<replyElement->GetNumberOfAttributes(); attrIndex++)
    {
      if (replyElement->GetAttributeName(attrIndex)==0)
      {
        continue;
      }
      if (strcmp(replyElement->GetAttributeName(attrIndex),"Status")==0
        || strcmp(replyElement->GetAttributeName(attrIndex),"Message")==0)
      {
        // Status and Message attributes are processed separately
        continue;
      }
      if (!attributes.empty())
      {
        attributes+=" ";
      }
      attributes += std::string(replyElement->GetAttributeName(attrIndex)) + "=\""
        + (replyElement->GetAttributeValue(attrIndex)?replyElement->GetAttributeValue(attrIndex):"") +"\"";
    }
  }
  else
  {
    // The reply is not XML
    status = REPLY_FAIL;
    message = reply;
  }

  return status;
}

/**
 * @param commandId int that is used to remove any ACK and CMD text annotation nodes
 * @param connectorNodeId Identifies the IGTL connector node that will send the command message to the server.
 */
void vtkSlicerOpenIGTLinkRemoteLogic::DiscardCommand( int commandId, const char* connectorNodeId )
{
  vtkMRMLIGTLConnectorNode* connectorNode = vtkMRMLIGTLConnectorNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( connectorNodeId ) );
  if ( connectorNode == NULL )
  {
    vtkErrorMacro( "SendCommand could not cast MRML node to IGTLConnectorNode." );
    return;
  }

  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  {
    std::stringstream ss;
    ss << "ACK_" << commandId;
    vtkSmartPointer<vtkCollection> replyNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByName( ss.str().c_str() ));
    for( int i = 0; i < replyNodes->GetNumberOfItems(); ++i )
    {
      vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast( replyNodes->GetItemAsObject(i) );
      if ( textNode == NULL )
      {
        vtkErrorMacro( "Could not cast reply node to vtkMRMLAnnotationTextNode!" );
        continue;
      }
      connectorNode->UnregisterIncomingMRMLNode(textNode);
      this->GetMRMLScene()->RemoveNode(textNode);
    }
  }

  {
    std::stringstream ss;
    ss << "CMD_" << commandId;
    vtkCollection* commandNodes = this->GetMRMLScene()->GetNodesByName( ss.str().c_str() );
    for( int i = 0; i < commandNodes->GetNumberOfItems(); ++i )
    {
      vtkMRMLAnnotationTextNode* textNode = vtkMRMLAnnotationTextNode::SafeDownCast( commandNodes->GetItemAsObject(i) );
      if ( textNode == NULL )
      {
        vtkErrorMacro( "Could not cast reply node to vtkMRMLAnnotationTextNode!" );
        continue;
      }
      connectorNode->UnregisterOutgoingMRMLNode(textNode);
      this->GetMRMLScene()->RemoveNode(textNode);
    }
    commandNodes->Delete();
  }

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
  
  // Create a unique Id for this command message.
  // The logic may only be used from the main thread, so there is no need
  // for making the counter increment thread-safe.
  std::stringstream ss;
  (this->CommandCounter)++;
  ss << "CMD_" << this->CommandCounter;
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
  if (this->GetMRMLScene()==NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLTextNode>::New());
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
