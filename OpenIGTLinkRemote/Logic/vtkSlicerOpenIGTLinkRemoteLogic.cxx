#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLIGTLQueryNode.h"
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

//----------------------------------------------------------------------------
vtkMRMLIGTLQueryNode* vtkSlicerOpenIGTLinkRemoteLogic::CreateCommandQueryNode()
{
  if ( this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("vtkSlicerOpenIGTLinkRemoteLogic::CreateCommandQueryNode failed: invalid scene");
    return NULL;
  }
  vtkSmartPointer< vtkMRMLIGTLQueryNode > queryNode = vtkSmartPointer< vtkMRMLIGTLQueryNode >::New();
  queryNode->SetHideFromEditors(true);
  queryNode->SetSaveWithScene(false);
  this->GetMRMLScene()->AddNode(queryNode);

  // Also create the response node. It allows the node to be hidden and excluded from saving with the scene.
  vtkSmartPointer< vtkMRMLTextNode > responseNode = vtkSmartPointer< vtkMRMLTextNode >::New();
  responseNode->SetHideFromEditors(true);
  responseNode->SetSaveWithScene(false);
  this->GetMRMLScene()->AddNode(responseNode);

  queryNode->SetResponseDataNodeID(responseNode->GetID());

  return queryNode;
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::DeleteCommandQueryNode(vtkMRMLIGTLQueryNode* commandQueryNode)
{
  if ( this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("vtkSlicerOpenIGTLinkRemoteLogic::DiscardCommand failed: invalid scene");
    return;
  }

  if (commandQueryNode==NULL)
  {
    vtkErrorMacro("vtkSlicerOpenIGTLinkRemoteLogic::DiscardCommand failed: invalid queryNode");
    return;
  }

  // Delete response node
  vtkMRMLNode* responseNode = commandQueryNode->GetResponseDataNode();
  if (responseNode != NULL)
  {
    this->GetMRMLScene()->RemoveNode(responseNode);
  }

  // Delete query node
  this->GetMRMLScene()->RemoveNode(commandQueryNode);
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkRemoteLogic::SendCommand(vtkMRMLIGTLQueryNode* commandQueryNode, const char* connectorNodeId, const char* commandName, const char* attributes)
{
  std::string messageString = std::string("<Command Name=\"") + (commandName?commandName:"") + "\" " + (attributes?attributes:"") + " />";
  return this->SendCommandXML(commandQueryNode, connectorNodeId, messageString.c_str());
}

//----------------------------------------------------------------------------
bool vtkSlicerOpenIGTLinkRemoteLogic::SendCommandXML(vtkMRMLIGTLQueryNode* commandQueryNode, const char* connectorNodeId, const char* commandXml)
{
  if ( this->GetMRMLScene() == NULL )
  {
    vtkErrorMacro( "MRML Scene is invalid" );
    return false;
  }
  
  vtkMRMLIGTLConnectorNode* connectorNode = vtkMRMLIGTLConnectorNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( connectorNodeId ) );
  if ( connectorNode == NULL )
  {
    vtkErrorMacro( "SendCommand could not cast MRML node to IGTLConnectorNode." );
    return false;
  }
  if ( commandQueryNode == NULL )
  {
    vtkErrorMacro( "SendCommand failed: commandQueryNode is invalid" );
    return false;
  }

  // Create a unique Id for this command message.
  // The logic may only be used from the main thread, so there is no need
  // for making the counter increment thread-safe.
  (this->CommandCounter)++;
  std::stringstream commandDeviceName;
  commandDeviceName << "CMD_" << this->CommandCounter;
  std::stringstream responseDeviceName;
  responseDeviceName << "ACK_" << this->CommandCounter;

  commandQueryNode->SetIGTLName("STRING");
  commandQueryNode->SetIGTLDeviceName( responseDeviceName.str().c_str() );
  commandQueryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  commandQueryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_NOT_DEFINED);
  commandQueryNode->SetAttribute("CommandDeviceName", commandDeviceName.str().c_str());
  commandQueryNode->SetAttribute("CommandString", commandXml);

  // Also update the corresponding response data node ID's name to avoid creation of a new response node
  // (the existing response node will be updated).
  vtkMRMLTextNode* responseDataNode = vtkMRMLTextNode::SafeDownCast(commandQueryNode->GetResponseDataNode());
  if (responseDataNode!=NULL)
  {
    responseDataNode->SetName(responseDeviceName.str().c_str());
    responseDataNode->SetText(NULL);
  }
  
  // Sends the command string and register the query node
  connectorNode->PushQuery(commandQueryNode);
  
  return true;
}

//----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkRemoteLogic::COMMAND_RESULT vtkSlicerOpenIGTLinkRemoteLogic::GetCommandResponse(vtkMRMLIGTLQueryNode* queryNode, std::string &message, std::string &attributes)
{
  message.clear();
  attributes.clear();

  if ( this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("vtkSlicerOpenIGTLinkRemoteLogic::GetCommandResponse failed: invalid scene");
    return COMMAND_FAIL;
  }

  if (queryNode==NULL)
  {
    vtkErrorMacro("vtkSlicerOpenIGTLinkRemoteLogic::GetCommandResponse failed: invalid queryNode");
    return COMMAND_FAIL;
  }

  if (queryNode->GetQueryStatus()==vtkMRMLIGTLQueryNode::STATUS_NOT_DEFINED
    || queryNode->GetQueryStatus()==vtkMRMLIGTLQueryNode::STATUS_ERROR
    || queryNode->GetQueryStatus()==vtkMRMLIGTLQueryNode::STATUS_EXPIRED)
  {
    return COMMAND_FAIL;
  }

  if (queryNode->GetQueryStatus()==vtkMRMLIGTLQueryNode::STATUS_PREPARED
    || queryNode->GetQueryStatus()==vtkMRMLIGTLQueryNode::STATUS_WAITING)
  {
    return COMMAND_WAITING;
  }

  // We've successfully received a command response

  vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast( queryNode->GetResponseDataNode() );
  if ( textNode == NULL )
  {
    vtkErrorMacro( "Could not retrieve response node as vtkMRMLTextNode" );
    return COMMAND_FAIL;
  }  
  std::string response = textNode->GetText() ? textNode->GetText() : "";  
  vtkSmartPointer<vtkXMLDataElement> responseElement;
  if (!response.empty())
  {
    responseElement = vtkSmartPointer<vtkXMLDataElement>::Take( vtkXMLUtilities::ReadElementFromString(response.c_str()) );
  }
  if (responseElement == NULL)
  {
    // The response is not XML
    message = response;
    return COMMAND_FAIL;
  }

  // Retrieve status from XML string
  COMMAND_RESULT status = COMMAND_FAIL;
  if (responseElement->GetAttribute("Status")==NULL)
  {
    vtkErrorMacro("OpenIGTLink command response: missing Status attribute: "<<response);
  }
  else
  {
    if (strcmp(responseElement->GetAttribute("Status"),"SUCCESS")==0)
    {
      status = COMMAND_SUCCESS;
    }
    else if (strcmp(responseElement->GetAttribute("Status"),"FAIL")==0)
    {
      status = COMMAND_FAIL;
    }
    else
    {
      vtkErrorMacro("OpenIGTLink command response: invalid Status attribute value: "<<responseElement->GetAttribute("Status"));
    }
  }
  // Retrieve Data element from XML string
  if (responseElement->GetAttribute("Message")!=NULL)
  {
    message=responseElement->GetAttribute("Message");
  }
  // Retrieve other attributes from XML string    
  for (int attrIndex=0; attrIndex<responseElement->GetNumberOfAttributes(); attrIndex++)
  {
    if (responseElement->GetAttributeName(attrIndex)==0)
    {
      continue;
    }
    if (strcmp(responseElement->GetAttributeName(attrIndex),"Status")==0
      || strcmp(responseElement->GetAttributeName(attrIndex),"Message")==0)
    {
      // Status and Message attributes are processed separately
      continue;
    }
    if (!attributes.empty())
    {
      attributes+=" ";
    }
    attributes += std::string(responseElement->GetAttributeName(attrIndex)) + "=\""
      + (responseElement->GetAttributeValue(attrIndex)?responseElement->GetAttributeValue(attrIndex):"") +"\"";
  }

  return status;
}

//----------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic::CancelCommand(vtkMRMLIGTLQueryNode* commandQueryNode)
{
  if (commandQueryNode==NULL)
  {
    vtkErrorMacro("vtkSlicerOpenIGTLinkRemoteLogic::CancelCommand failed: invalid commandQueryNode");
    return;
  }
  vtkMRMLIGTLConnectorNode* connector = commandQueryNode->GetConnectorNode();
  if (connector)
  {
    connector->CancelQuery(commandQueryNode);
  }
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
