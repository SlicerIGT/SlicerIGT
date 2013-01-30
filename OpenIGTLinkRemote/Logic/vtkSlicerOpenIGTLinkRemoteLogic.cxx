
// OpenIGTLinkRemote Logic includes
#include "vtkSlicerOpenIGTLinkRemoteLogic.h"

#include "vtkSlicerOpenIGTLinkIFLogic.h"

// MRML includes
#include "vtkMRMLRemoteExecNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerOpenIGTLinkRemoteLogic);



vtkSlicerOpenIGTLinkRemoteLogic
::vtkSlicerOpenIGTLinkRemoteLogic()
{
}



vtkSlicerOpenIGTLinkRemoteLogic
::~vtkSlicerOpenIGTLinkRemoteLogic()
{
}



void vtkSlicerOpenIGTLinkRemoteLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
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
  
  // Register message converters.
  // qSlicerCoreApplication::
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

