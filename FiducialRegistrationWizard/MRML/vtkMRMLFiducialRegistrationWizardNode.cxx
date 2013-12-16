
// FiducialRegistrationWizard MRML includes
#include "vtkMRMLFiducialRegistrationWizardNode.h"


// Constants ------------------------------------------------------------------
static const char* PROBE_TRANSFORM_REFERENCE_ROLE = "ProbeTransform";
static const char* FROM_FIDUCIAL_LIST_REFERENCE_ROLE = "FromFiducialList";
static const char* TO_FIDUCIAL_LIST_REFERENCE_ROLE = "ToFiducialList";
static const char* OUTPUT_TRANSFORM_REFERENCE_ROLE = "OutputTransform";


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

  this->AddNodeReferenceRole( PROBE_TRANSFORM_REFERENCE_ROLE );
  this->AddNodeReferenceRole( FROM_FIDUCIAL_LIST_REFERENCE_ROLE );
  this->AddNodeReferenceRole( TO_FIDUCIAL_LIST_REFERENCE_ROLE );
  this->AddNodeReferenceRole( OUTPUT_TRANSFORM_REFERENCE_ROLE );
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
  this->SetProbeTransformID( this->GetProbeTransformID(), NeverModify );
  this->SetFromFiducialListID( this->GetFromFiducialListID(), NeverModify );
  this->SetToFiducialListID( this->GetToFiducialListID(), NeverModify );
  this->SetOutputTransformID( this->GetOutputTransformID(), NeverModify );

  this->UpdateNodeReferences();
  this->Modified();
}


// Variable getters and setters -----------------------------------------------------

std::string vtkMRMLFiducialRegistrationWizardNode
::GetProbeTransformID()
{
  return this->GetNodeReferenceIDString( PROBE_TRANSFORM_REFERENCE_ROLE );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetProbeTransformID( std::string newProbeTransformID, int modifyType )
{
  if ( newProbeTransformID.compare( "" ) == 0 )
  {
    this->RemoveAllNodeReferenceIDs( PROBE_TRANSFORM_REFERENCE_ROLE );
  }
  else if ( this->GetProbeTransformID() != newProbeTransformID )
  {
    this->SetNodeReferenceID( PROBE_TRANSFORM_REFERENCE_ROLE, newProbeTransformID.c_str() );
  }
  if ( this->GetProbeTransformID() != newProbeTransformID && modifyType == DefaultModify || modifyType == AlwaysModify )
  {
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetFromFiducialListID()
{
  return this->GetNodeReferenceIDString( FROM_FIDUCIAL_LIST_REFERENCE_ROLE );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetFromFiducialListID( std::string newFromFiducialListID, int modifyType )
{
  if ( this->GetFromFiducialListID() != newFromFiducialListID )
  {
    this->SetAndObserveNodeReferenceID( FROM_FIDUCIAL_LIST_REFERENCE_ROLE, newFromFiducialListID.c_str() );
  }
  if ( this->GetFromFiducialListID() != newFromFiducialListID && modifyType == DefaultModify || modifyType == AlwaysModify )
  {
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetToFiducialListID()
{
  return this->GetNodeReferenceIDString( TO_FIDUCIAL_LIST_REFERENCE_ROLE );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetToFiducialListID( std::string newToFiducialListID, int modifyType )
{
  if ( this->GetToFiducialListID() != newToFiducialListID )
  {
    this->SetAndObserveNodeReferenceID( TO_FIDUCIAL_LIST_REFERENCE_ROLE, newToFiducialListID.c_str() );
  }
  if ( this->GetToFiducialListID() != newToFiducialListID && modifyType == DefaultModify || modifyType == AlwaysModify )
  {
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetOutputTransformID()
{
  return this->GetNodeReferenceIDString( OUTPUT_TRANSFORM_REFERENCE_ROLE );
}


void vtkMRMLFiducialRegistrationWizardNode
::SetOutputTransformID( std::string newOutputTransformID, int modifyType )
{
  if ( newOutputTransformID.compare( "" ) == 0 )
  {
    this->RemoveAllNodeReferenceIDs( OUTPUT_TRANSFORM_REFERENCE_ROLE );
  }
  else if ( this->GetOutputTransformID() != newOutputTransformID )
  {
    this->SetNodeReferenceID( OUTPUT_TRANSFORM_REFERENCE_ROLE, newOutputTransformID.c_str() );
  }
  if ( this->GetOutputTransformID() != newOutputTransformID && modifyType == DefaultModify || modifyType == AlwaysModify )
  {
    this->Modified();
  }
}


std::string vtkMRMLFiducialRegistrationWizardNode
::GetRegistrationMode()
{
  return this->RegistrationMode;
}


void vtkMRMLFiducialRegistrationWizardNode
::SetRegistrationMode( std::string newRegistrationMode, int modifyType )
{
  if ( this->GetRegistrationMode() != newRegistrationMode )
  {
    this->RegistrationMode = newRegistrationMode;
  }
  if ( this->GetRegistrationMode() != newRegistrationMode && modifyType == DefaultModify || modifyType == AlwaysModify ) 
  {
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