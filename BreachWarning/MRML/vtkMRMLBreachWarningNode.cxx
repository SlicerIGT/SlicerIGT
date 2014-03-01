
// BreachWarning MRML includes
#include "vtkMRMLBreachWarningNode.h"


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

vtkMRMLBreachWarningNode* vtkMRMLBreachWarningNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLBreachWarningNode" );
  if( ret )
    {
      return ( vtkMRMLBreachWarningNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLBreachWarningNode;
}



vtkMRMLBreachWarningNode
::vtkMRMLBreachWarningNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  // this->SetModifiedSinceRead( true );

  this->AddNodeReferenceRole( "WatchedModel" );
  this->AddNodeReferenceRole( "ToolTipTransform" );

  this->WatchedModelID = "";
  this->ToolTipTransformID = "";

  this->Modified();

  /*
  // Old sample.
  this->AddNodeReferenceRole( PROBE_TRANSFORM_REFERENCE_ROLE );
  this->AddNodeReferenceRole( FROM_FIDUCIAL_LIST_REFERENCE_ROLE );
  this->AddNodeReferenceRole( TO_FIDUCIAL_LIST_REFERENCE_ROLE );
  this->AddNodeReferenceRole( OUTPUT_TRANSFORM_REFERENCE_ROLE );
  this->RegistrationMode = "Rigid";
  */

}



vtkMRMLBreachWarningNode
::~vtkMRMLBreachWarningNode()
{
}



vtkMRMLNode* vtkMRMLBreachWarningNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLBreachWarningNode" );
  if( ret )
    {
      return ( vtkMRMLBreachWarningNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLBreachWarningNode;
}



// Scene: Save and load
// ----------------------------------------------------------------------------


void vtkMRMLBreachWarningNode
::WriteXML( ostream& of, int nIndent )
{

  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes

  vtkIndent indent(nIndent);
  
  of << indent << " WatchedModelID=\"" << this->WatchedModelID << "\"";
  of << indent << " ToolTipTransformID=\"" << this->ToolTipTransformID << "\"";

  // of << indent << " RegistrationMode=\"" << this->RegistrationMode << "\"";
}



void vtkMRMLBreachWarningNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "WatchedModuleID" ) )
    {
      this->WatchedModelID = std::string( attValue );
    }

    if ( ! strcmp( attName, "ToolTipTransformID" ) )
    {
      this->ToolTipTransformID = std::string( attValue );
    }

    // Old sample.
    if ( ! strcmp( attName, "RegistrationMode" ) )
    {
      this->RegistrationMode = std::string( attValue );
    }

  }

  this->Modified();
}



// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLBreachWarningNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLBreachWarningNode *node = ( vtkMRMLBreachWarningNode* ) anode;
  
  // Note: It seems that the WriteXML function copies the node then writes the copied node to file
  // So, anything we want in the MRML file we must copy here (I don't think we need to copy other things)
  
  this->WatchedModelID = node->WatchedModelID;
  this->ToolTipTransformID = node->ToolTipTransformID;

  // this->RegistrationMode = node->RegistrationMode;
  this->Modified();
}



void vtkMRMLBreachWarningNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes

  os << indent << "WatchedModelID: " << this->WatchedModelID << "\n";
  os << indent << "ToolTipTransformID: " << this->ToolTipTransformID << "\n";

  // os << indent << "RegistrationMode: " << this->RegistrationMode << "\n";
}


void vtkMRMLBreachWarningNode
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



std::string vtkMRMLBreachWarningNode
::GetProbeTransformID()
{
  return this->GetNodeReferenceIDString( PROBE_TRANSFORM_REFERENCE_ROLE );
}



void vtkMRMLBreachWarningNode
::SetWatchedModelID( std::string modelID, int modifyType )
{
  bool modify = false;

  if ( modelID.compare( "" ) == 0 )
  {
    this->RemoveAllNodeReferenceIDs( "WatchedModel" );
  }
  else if ( this->GetWatchedModelID() != modelID )
  {
    if ( modifyType == DefaultModify || modifyType == AlwaysModify )
    {
      modify = true;
    }
    this->SetNodeReferenceID( "WatchedModel", modelID.c_str() );
  }

  if ( modify )
  {
    this->Modified();
  }
}



std::string vtkMRMLBreachWarningNode
::GetWatchedModelID()
{
  return this->GetNodeReferenceIDString( "WatchedModel" );
}



void vtkMRMLBreachWarningNode
::SetToolTipTransformID( std::string newTransformID, int modifyType )
{
  if ( newTransformID.compare( "" ) == 0 )
  {
    this->RemoveAllNodeReferenceIDs( "ToolTipTransform" );
  }
  else if ( this->GetToolTipTransformID() != newTransformID )
  {
    this->SetNodeReferenceID( "ToolTipTransform", newTransformID.c_str() );
  }
  if ( this->GetToolTipTransformID() != newTransformID && modifyType == DefaultModify || modifyType == AlwaysModify )
  {
    this->Modified();
  }
}



std::string vtkMRMLBreachWarningNode
::GetToolTipTransformID()
{
  return this->GetNodeReferenceIDString( "ToolTipTransform" );
}



void vtkMRMLBreachWarningNode
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


std::string vtkMRMLBreachWarningNode
::GetFromFiducialListID()
{
  return this->GetNodeReferenceIDString( FROM_FIDUCIAL_LIST_REFERENCE_ROLE );
}


void vtkMRMLBreachWarningNode
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


std::string vtkMRMLBreachWarningNode
::GetToFiducialListID()
{
  return this->GetNodeReferenceIDString( TO_FIDUCIAL_LIST_REFERENCE_ROLE );
}


void vtkMRMLBreachWarningNode
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


std::string vtkMRMLBreachWarningNode
::GetOutputTransformID()
{
  return this->GetNodeReferenceIDString( OUTPUT_TRANSFORM_REFERENCE_ROLE );
}


void vtkMRMLBreachWarningNode
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


std::string vtkMRMLBreachWarningNode
::GetRegistrationMode()
{
  return this->RegistrationMode;
}


void vtkMRMLBreachWarningNode
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


std::string vtkMRMLBreachWarningNode
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


void vtkMRMLBreachWarningNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  this->Modified(); // This will tell the logic to update
}