
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

  this->AddNodeReferenceRole( "ProbeTransform" );
  this->AddNodeReferenceRole( "FromFiducialList" );
  this->AddNodeReferenceRole( "ToFiducialList" );
  this->AddNodeReferenceRole( "ActiveFiducialList" );
  this->AddNodeReferenceRole( "OutputTransform" );
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


void vtkMRMLFiducialRegistrationWizardNode
::WriteXML( ostream& of, int nIndent )
{

  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes

  vtkIndent indent(nIndent);
  
  of << indent << " RegistrationMode=\"" << this->RegistrationMode << "\"";
}



void vtkMRMLFiducialRegistrationWizardNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "RegistrationMode" ) )
    {
      this->RegistrationMode = std::string( attValue );
    }

  }

  this->Modified();
}



// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLFiducialRegistrationWizardNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLFiducialRegistrationWizardNode *node = ( vtkMRMLFiducialRegistrationWizardNode* ) anode;
  
  // Note: It seems that the WriteXML function copies the node then writes the copied node to file
  // So, anything we want in the MRML file we must copy here (I don't think we need to copy other things)
  this->RegistrationMode = node->RegistrationMode;
  this->Modified();
}



void vtkMRMLFiducialRegistrationWizardNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "RegistrationMode: " << this->RegistrationMode << "\n";
}


void vtkMRMLFiducialRegistrationWizardNode
::ObserveAllReferenceNodes()
{
  this->SetProbeTransformID( this->GetProbeTransformID() );
  this->SetFromFiducialListID( this->GetFromFiducialListID() );
  this->SetToFiducialListID( this->GetToFiducialListID() );
  this->SetActiveFiducialListID( this->GetActiveFiducialListID() );
  this->SetOutputTransformID( this->GetOutputTransformID() );

  this->UpdateScene( this->GetScene() );
  this->Modified();
}


// Variable getters and setters -----------------------------------------------------

std::string vtkMRMLFiducialRegistrationWizardNode
::GetProbeTransformID()
{
  return this->GetNodeReferenceIDString( "ProbeTransform" );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetProbeTransformID( std::string newProbeTransformID )
{
  if( this->GetProbeTransformID() != newProbeTransformID )
  {
    this->SetAndObserveNodeReferenceID( "ProbeTransform", newProbeTransformID.c_str() );
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetFromFiducialListID()
{
  return this->GetNodeReferenceIDString( "FromFiducialList" );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetFromFiducialListID( std::string newFromFiducialListID )
{
  if ( this->GetFromFiducialListID() != newFromFiducialListID )
  {
    this->SetAndObserveNodeReferenceID( "FromFiducialList", newFromFiducialListID.c_str() );
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetToFiducialListID()
{
  return this->GetNodeReferenceIDString( "ToFiducialList" );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetToFiducialListID( std::string newToFiducialListID )
{
  if ( this->GetToFiducialListID() != newToFiducialListID )
  {
    this->SetAndObserveNodeReferenceID( "ToFiducialList", newToFiducialListID.c_str() );
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetActiveFiducialListID()
{
  return this->GetNodeReferenceIDString( "ActiveFiducialList" );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetActiveFiducialListID( std::string newActiveFiducialListID )
{
  if( this->GetActiveFiducialListID() != newActiveFiducialListID )
  {
    this->SetAndObserveNodeReferenceID( "ActiveFiducialList", newActiveFiducialListID.c_str() );
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetOutputTransformID()
{
  return this->GetNodeReferenceIDString( "OutputTransform" );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetOutputTransformID( std::string newOutputTransformID )
{
  if ( this->GetOutputTransformID() != newOutputTransformID )
  {
    this->SetAndObserveNodeReferenceID( "OutputTransform", newOutputTransformID.c_str() );
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetRegistrationMode()
{
  return this->RegistrationMode;
}


void vtkMRMLFiducialRegistrationWizardNode
::SetRegistrationMode( std::string newRegistrationMode )
{
  if ( this->RegistrationMode != newRegistrationMode )
  {
    this->RegistrationMode = newRegistrationMode;
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetNodeReferenceIDString( std::string referenceRole )
{
  const char* refID = this->GetNodeReferenceID( referenceRole.c_str() );
  std::string refIDString;

  if ( refID == NULL )
  {
    refIDString = "";
  }
  else
  {
    refIDString = refID;
  }

  return refIDString;
}


void vtkMRMLFiducialRegistrationWizardNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  this->Modified(); // This will tell the logic to update
}