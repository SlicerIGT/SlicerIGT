
// FiducialRegistrationWizard MRML includes
#include "vtkMRMLFiducialRegistrationWizardNode.h"


// MACROS ---------------------------------------------------------------------

#define DELETE_IF_NOT_NULL(x) \
  if ( x != NULL ) \
    { \
    x->Delete(); \
    x = NULL; \
    }

#define WRITE_STRING_XML(x) \
  if ( this->x != NULL ) \
  { \
    of << indent << " "#x"=\"" << this->x << "\"\n"; \
  }

#define READ_AND_SET_STRING_XML(x) \
    if ( !strcmp( attName, #x ) ) \
      { \
      this->SetAndObserve##x( NULL ); \
      this->Set##x( attValue ); \
      }


// Constructors and Destructors
// ----------------------------------------------------------------------------

vtkMRMLFiducialRegistrationWizardNode* vtkMRMLFiducialRegistrationWizardNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLFiducialRegistrationWizardNode" );
  if( ret )
    {
      return ( vtkMRMLFiducialRegistrationWizardNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialRegistrationWizardNode;
}



vtkMRMLFiducialRegistrationWizardNode
::vtkMRMLFiducialRegistrationWizardNode()
{
  this->HideFromEditorsOn();
  this->SetSaveWithScene( true );
  this->SetSingletonTag( "FiducialRegistrationWizard" );
  // this->SetModifiedSinceRead( true );

  this->ProbeTransformID = "";
  this->FromFiducialListID = "";
  this->ToFiducialListID = "";
  this->ActiveFiducialListID = "";
  this->OutputTransformID = "";
  this->RegistrationMode = "";

  this->Modified();
}



vtkMRMLFiducialRegistrationWizardNode
::~vtkMRMLFiducialRegistrationWizardNode()
{
}



vtkMRMLNode* vtkMRMLFiducialRegistrationWizardNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLFiducialRegistrationWizardNode" );
  if( ret )
    {
      return ( vtkMRMLFiducialRegistrationWizardNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialRegistrationWizardNode;
}



// Scene: Save and load
// ----------------------------------------------------------------------------


void vtkMRMLFiducialRegistrationWizardNode::WriteXML( ostream& of, int nIndent )
{

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << " ProbeTransformID=\"" << this->ProbeTransformID << "\"";
  of << indent << " FromFiducialListID=\"" << this->FromFiducialListID << "\"";
  of << indent << " ToFiducialListID=\"" << this->ToFiducialListID << "\"";
  of << indent << " ActiveFiducialListID=\"" << this->ActiveFiducialListID << "\"";
  of << indent << " OutputTransformID=\"" << this->OutputTransformID << "\"";
  of << indent << " RegistrationMode=\"" << this->RegistrationMode << "\"";

}



void vtkMRMLFiducialRegistrationWizardNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
	if ( ! strcmp( attName, "ProbeTransformID" ) )
    {
      this->ProbeTransformID = std::string( attValue );
    }
	if ( ! strcmp( attName, "FromFiducialListID" ) )
    {
      this->FromFiducialListID = std::string( attValue );
    }
	if ( ! strcmp( attName, "ToFiducialListID" ) )
    {
      this->ToFiducialListID = std::string( attValue );
    }
    if ( ! strcmp( attName, "ActiveFiducialListID" ) )
    {
      this->ActiveFiducialListID = std::string( attValue );
    }
    if ( ! strcmp( attName, "OutputTransformID" ) )
    {
      this->OutputTransformID = std::string( attValue );
    }
    if ( ! strcmp( attName, "RegistrationMode" ) )
    {
      this->RegistrationMode = std::string( attValue );
    }

  }

  this->Modified();
}



// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLFiducialRegistrationWizardNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode );
  vtkMRMLFiducialRegistrationWizardNode *node = ( vtkMRMLFiducialRegistrationWizardNode* ) anode;
  
  // Note: It seems that the WriteXML function copies the node then writes the copied node to file
  // So, anything we want in the MRML file we must copy here (I don't think we need to copy other things)
  this->ProbeTransformID = node->ProbeTransformID;
  this->FromFiducialListID = node->FromFiducialListID;
  this->ToFiducialListID = node->ToFiducialListID;
  this->ActiveFiducialListID = node->ActiveFiducialListID;
  this->OutputTransformID = node->OutputTransformID;
  this->RegistrationMode = node->RegistrationMode;

  this->Modified();
}



void vtkMRMLFiducialRegistrationWizardNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "ProbeTransformID: " << this->ProbeTransformID << "\n";
  os << indent << "FromFiducialListID: " << this->FromFiducialListID << "\n";
  os << indent << "ToFiducialListID: " << this->ToFiducialListID << "\n";
  os << indent << "ActiveFiducialListID: " << this->ActiveFiducialListID << "\n";
  os << indent << "OutputTransformID: " << this->OutputTransformID << "\n";
  os << indent << "RegistrationMode: " << this->RegistrationMode << "\n";
}