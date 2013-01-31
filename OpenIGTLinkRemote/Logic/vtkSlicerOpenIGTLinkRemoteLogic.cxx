
// OpenIGTLinkRemote Logic includes
#include "vtkSlicerOpenIGTLinkRemoteLogic.h"

#include "vtkSlicerOpenIGTLinkIFLogic.h"

// MRML includes
#include "vtkMRMLRemoteExecNode.h"
#include "vtkIGTLToMRMLRemoteExec.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>



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
  this->CommandConverter = vtkIGTLToMRMLRemoteExec::New();
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
  
  vtkMRMLRemoteExecNode* node = vtkMRMLRemoteExecNode::New();
  this->GetMRMLScene()->RegisterNodeClass( node );
  node->Delete();
  
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

