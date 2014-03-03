
// BreachWarning MRML includes
#include "vtkMRMLBreachWarningNode.h"

// Other MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmartPointer.h>


// Constants ------------------------------------------------------------------
static const char* MODEL_ROLE = "WatchedModel";
static const char* TOOL_ROLE = "ToolToRasTransform";


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
  
  this->AddNodeReferenceRole( MODEL_ROLE );
  this->AddNodeReferenceRole( TOOL_ROLE );

  this->Modified();
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


void
vtkMRMLBreachWarningNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes

  vtkIndent indent(nIndent);
  
  of << indent << " WatchedModelID=\"" << this->GetNodeReferenceIDString( MODEL_ROLE ) << "\"\n";
  of << indent << " ToolTipTransformID=\"" << this->GetNodeReferenceIDString( TOOL_ROLE) << "\"\n";
}



void
vtkMRMLBreachWarningNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  // This does nothing now.
  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    /*
    if ( ! strcmp( attName, "ToolTipTransformID" ) )
    {
      this->ToolTipTransformID = std::string( attValue );
    }
    */

  }

  this->Modified();
}



void
vtkMRMLBreachWarningNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  
  vtkMRMLBreachWarningNode *node = ( vtkMRMLBreachWarningNode* ) anode;
  
  this->ToolInsideModel = node->ToolInsideModel;
  
  this->Modified();
}



void
vtkMRMLBreachWarningNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes

  os << indent << "WatchedModelID: " << this->GetNodeReferenceIDString( MODEL_ROLE ) << std::endl;
  os << indent << "ToolTipTransformID: " << this->GetNodeReferenceIDString( TOOL_ROLE ) << std::endl;
  os << indent << "ToolInsideModel: " << this->ToolInsideModel << std::endl;
}



void
vtkMRMLBreachWarningNode
::SetWatchedModelID( std::string modelID, int modifyType )
{
  bool modify = false;

  if ( modelID.compare( "" ) == 0 )
  {
    this->RemoveAllNodeReferenceIDs( MODEL_ROLE );
    return;
  }

  if ( this->GetWatchedModelID() != modelID )
  {
    if ( modifyType == DefaultModify || modifyType == AlwaysModify )
    {
      modify = true;
    }
    this->SetAndObserveNodeReferenceID( MODEL_ROLE, modelID.c_str() );
  }

  if ( modify )
  {
    this->Modified();
  }
}



std::string
vtkMRMLBreachWarningNode
::GetWatchedModelID()
{
  return this->GetNodeReferenceIDString( MODEL_ROLE );
}



void
vtkMRMLBreachWarningNode
::SetToolTipTransformID( std::string newTransformID, int modifyType )
{
  if ( newTransformID.compare( "" ) == 0 )
  {
    this->RemoveAllNodeReferenceIDs( TOOL_ROLE );
    return;
  }
  
  if ( this->GetToolTipTransformID() != newTransformID )
  {
    this->SetNodeReferenceID( TOOL_ROLE, newTransformID.c_str() );
  }
  if ( this->GetToolTipTransformID() != newTransformID && modifyType == DefaultModify || modifyType == AlwaysModify )
  {
    this->Modified();
  }
}



std::string
vtkMRMLBreachWarningNode
::GetToolTipTransformID()
{
  return this->GetNodeReferenceIDString( TOOL_ROLE );
}



std::string
vtkMRMLBreachWarningNode
::GetNodeReferenceIDString( std::string referenceRole )
{
  const char* refID = this->GetNodeReferenceID( referenceRole.c_str() );
  std::string refIDString;

  if ( refID == NULL )
  {
    refIDString = std::string( "" );
  }
  else
  {
    refIDString = std::string( refID );
  }

  return refIDString;
}



void
vtkMRMLBreachWarningNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;

  if ( this->GetNodeReferenceIDString( TOOL_ROLE ).compare( callerNode->GetID() ) == 0
    && event == vtkMRMLTransformNode::TransformModifiedEvent )
  {
    this->UpdateCalculation();
  }

  this->Modified(); // This will tell the logic to update
}



void
vtkMRMLBreachWarningNode
::UpdateCalculation()
{
  vtkMRMLNode* nodeForModel = this->GetNodeReference( MODEL_ROLE );
  vtkMRMLNode* nodeForTool = this->GetNodeReference( TOOL_ROLE );
  if ( nodeForModel == NULL || nodeForTool == NULL )
  {
    this->ToolInsideModel = false;
    return;
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( nodeForModel );
  if ( modelNode == NULL )
  {
    vtkWarningMacro( "Node type does not match role: surface model" );
    return;
  }

  vtkPolyData* body = modelNode->GetPolyData();
  if ( body == NULL )
  {
    vtkWarningMacro( "No surface model in node" );
    this->ToolInsideModel = false;
    return;
  }

  vtkMRMLLinearTransformNode* toolTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( nodeForTool );
  if ( toolTransformNode == NULL )
  {
    vtkWarningMacro( "Node type does not match role" );
    return;
  }

  vtkSmartPointer< vtkSelectEnclosedPoints > EnclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  EnclosedFilter->Initialize( body );
  
  vtkSmartPointer< vtkMatrix4x4 > ToolToRASMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  toolTransformNode->GetMatrixTransformToWorld( ToolToRASMatrix );
 
  double Origin[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };

  ToolToRASMatrix->MultiplyPoint( Origin, P0 );
  
  int inside = EnclosedFilter->IsInsideSurface( P0[ 0 ], P0[ 1 ], P0[ 2 ] );

  if ( inside )
  {
    this->ToolInsideModel = true;
  }
  else
  {
    this->ToolInsideModel = false;
  }
}
