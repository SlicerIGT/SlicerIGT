
#include <vtksys/SystemTools.hxx>

#include "vtkCommand.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLAnnotationTextNode.h"
#include "vtkIGTLToMRMLAnnotationText.h"

vtkStandardNewMacro(vtkIGTLToMRMLAnnotationText);
vtkCxxRevisionMacro(vtkIGTLToMRMLAnnotationText, "$Revision: 1.0 $");



vtkIGTLToMRMLAnnotationText
::vtkIGTLToMRMLAnnotationText()
{
  this->IGTLNames.clear();
}



vtkIGTLToMRMLAnnotationText
::~vtkIGTLToMRMLAnnotationText()
{
}



void vtkIGTLToMRMLAnnotationText::PrintSelf( ostream& os, vtkIndent indent )
{
}



vtkMRMLNode* vtkIGTLToMRMLAnnotationText
::CreateNewNode( vtkMRMLScene* scene, const char* name )
{
  vtkMRMLAnnotationTextNode* annotationTextNode;

  annotationTextNode = vtkMRMLAnnotationTextNode::New();
  annotationTextNode->SetName( name );
  annotationTextNode->SetDescription( "Created by OpenIGTLinkRemote module" );

  scene->AddNode( annotationTextNode );  

  return annotationTextNode;
}



vtkIntArray* vtkIGTLToMRMLAnnotationText
::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  events->InsertNextValue( vtkCommand::ModifiedEvent );

  return events;
}



int vtkIGTLToMRMLAnnotationText
::IGTLToMRML( igtl::MessageBase::Pointer buffer, vtkMRMLNode* node )
{
  return 1;
}



int vtkIGTLToMRMLAnnotationText
::MRMLToIGTL( unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg )
{
  if ( mrmlNode == NULL || event != vtkCommand::ModifiedEvent )
  {
    return 0;
  }
  
  vtkMRMLAnnotationTextNode* annotationTextNode = vtkMRMLAnnotationTextNode::SafeDownCast( mrmlNode );
  if ( annotationTextNode == NULL )
  {
    return 0;
  }
  
  int numberOfTexts = annotationTextNode->GetNumberOfTexts();
  if ( numberOfTexts < 1 )
  {
    vtkWarningMacro( "No text found in annotation text to be sent through OpenIGTLink." );
    return 0;
  }
  
  std::string command = std::string( annotationTextNode->GetText( 0 ).c_str() );
  
  
  if ( annotationTextNode->GetScene() == NULL )
  {
    vtkWarningMacro( "No scene set in annotation text node." );
    return 0;
  }
  
  this->StringMsg = igtl::StringMessage::New();
  this->StringMsg->SetDeviceName( annotationTextNode->GetName() );
  this->StringMsg->SetString( command );
  this->StringMsg->Pack();
  
  *size = this->StringMsg->GetPackSize();
  *igtlMsg = (void*)this->StringMsg->GetPackPointer();
  
  return 1;
}

