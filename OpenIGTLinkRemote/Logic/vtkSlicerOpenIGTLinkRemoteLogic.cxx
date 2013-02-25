
// OpenIGTLinkRemote Logic includes
#include "vtkSlicerOpenIGTLinkRemoteLogic.h"

#include "vtkSlicerOpenIGTLinkIFLogic.h"

// MRML includes
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkIGTLToMRMLAnnotationText.h"

#include "vtkMRMLIGTLConnectorNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>
#include <sstream>
#include <string>



class vtkSlicerOpenIGTLinkRemoteLogic::vtkInternal
{
public:
  vtkInternal();
  
  vtkSlicerOpenIGTLinkIFLogic* IFLogic;
};

vtkSlicerOpenIGTLinkRemoteLogic::vtkInternal::vtkInternal()
{
  this->IFLogic = 0;
}



vtkStandardNewMacro(vtkSlicerOpenIGTLinkRemoteLogic);



vtkSlicerOpenIGTLinkRemoteLogic
::vtkSlicerOpenIGTLinkRemoteLogic()
{
  this->Internal = new vtkInternal;
  this->CommandConverter = vtkIGTLToMRMLAnnotationText::New();
  this->CommandCounter = 0;
}



vtkSlicerOpenIGTLinkRemoteLogic
::~vtkSlicerOpenIGTLinkRemoteLogic()
{
  delete this->Internal;
  
  if ( this->CommandConverter != NULL )
  {
    this->CommandConverter->Delete();
    this->CommandConverter = NULL;
  }
}



void vtkSlicerOpenIGTLinkRemoteLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerOpenIGTLinkRemoteLogic
::SetIFLogic( vtkSlicerOpenIGTLinkIFLogic* ifLogic )
{
  this->Internal->IFLogic = ifLogic;
}



void vtkSlicerOpenIGTLinkRemoteLogic
::SendCommand( std::string strCommand, const char* connectorNodeId )
{
  if ( this->GetMRMLScene() == NULL )
  {
    vtkErrorMacro( "MRML Scene is invalid" );
    return;
  }
  
  vtkMRMLIGTLConnectorNode* connectorNode = vtkMRMLIGTLConnectorNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( connectorNodeId ) );
  if ( connectorNode == NULL )
  {
    vtkErrorMacro( "SendCommand could not cast MRML node to IGTLConnectorNode." );
    return;
  }
  
  // Create annotation text node for this command.
  
  vtkSmartPointer< vtkMRMLAnnotationTextNode > newNode = vtkSmartPointer< vtkMRMLAnnotationTextNode >::New();
  newNode->SetScene( this->GetMRMLScene() );
  newNode->SetText( 0, strCommand.c_str(), 1, 0 );
  
  // Giving unique name to this new text node.
  std::stringstream ss;
  ss << "RC" << this->CommandCounter;
  this->CommandCounter ++;
  newNode->SetName( ss.str().c_str() );
  
  this->GetMRMLScene()->AddNode( newNode );
  
  connectorNode->RegisterOutgoingMRMLNode( newNode );
  newNode->Modified();
}



void vtkSlicerOpenIGTLinkRemoteLogic
::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerOpenIGTLinkRemoteLogic
::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  
  // Register IGTL message converter.
  this->Internal->IFLogic->RegisterMessageConverter( this->CommandConverter );
}



void vtkSlicerOpenIGTLinkRemoteLogic
::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerOpenIGTLinkRemoteLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

