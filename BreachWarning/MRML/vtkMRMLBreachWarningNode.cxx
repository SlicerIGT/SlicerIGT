
// BreachWarning MRML includes
#include "vtkMRMLBreachWarningNode.h"

// Other MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

// Other includes
#include <sstream>

// Constants
static const char* MODEL_ROLE = "WatchedModelNode";
static const char* TOOL_ROLE = "ToolTransformNode";



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
  
  for ( int i = 0; i < 4; ++ i )
  {
    this->WarningColor[ i ] = 0.0;
    this->OriginalColor[ i ] = 0.0;
  }
}



vtkMRMLBreachWarningNode
::~vtkMRMLBreachWarningNode()
{
}



vtkMRMLNode*
vtkMRMLBreachWarningNode
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



void
vtkMRMLBreachWarningNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);

  of << indent << " WarningColorR=\"" << this->WarningColor[ 0 ] << "\"";
  of << indent << " WarningColorG=\"" << this->WarningColor[ 1 ] << "\"";
  of << indent << " WarningColorB=\"" << this->WarningColor[ 2 ] << "\"";
  of << indent << " WarningColorA=\"" << this->WarningColor[ 3 ] << "\"";
  of << indent << " OriginalColorR=\"" << this->OriginalColor[ 0 ] << "\"";
  of << indent << " OriginalColorG=\"" << this->OriginalColor[ 1 ] << "\"";
  of << indent << " OriginalColorB=\"" << this->OriginalColor[ 2 ] << "\"";
  of << indent << " OriginalColorA=\"" << this->OriginalColor[ 3 ] << "\"";
  of << indent << " ToolInsideModel=\"" << ( this->ToolInsideModel ? "true" : "false" ) << "\"";
}



void
vtkMRMLBreachWarningNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "WarningColorR" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->WarningColor[ 0 ] = r;
    }
    else if ( ! strcmp( attName, "WarningColorG" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double g = 0.0;
      ss >> g;
      this->WarningColor[ 1 ] = g;
    }
    else if ( ! strcmp( attName, "WarningColorB" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->WarningColor[ 2 ] = r;
    }
    else if ( ! strcmp( attName, "WarningColorA" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->WarningColor[ 3 ] = r;
    }
    else if ( ! strcmp( attName, "OriginalColorR" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->OriginalColor[ 0 ] = r;
    }
    else if ( ! strcmp( attName, "OriginalColorG" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->OriginalColor[ 1 ] = r;
    }
    else if ( ! strcmp( attName, "OriginalColorB" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->OriginalColor[ 2 ] = r;
    }
    else if ( ! strcmp( attName, "OriginalColorA" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
      this->OriginalColor[ 3 ] = r;
    }
    else if ( ! strcmp( attName, "ToolInsideModel" ) )
    {
      if (!strcmp(attValue,"true"))
      {
        this->ToolInsideModel = true;
      }
      else
      {
        this->ToolInsideModel = false;
      }
    }
  }
}



void
vtkMRMLBreachWarningNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  
  vtkMRMLBreachWarningNode *node = ( vtkMRMLBreachWarningNode* ) anode;
  
  for ( int i = 0; i < 4; ++ i )
  {
    this->WarningColor[ i ] = node->WarningColor[ i ];
  }

  this->ToolInsideModel = node->ToolInsideModel;
  
  this->Modified();
}



void
vtkMRMLBreachWarningNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes

  os << indent << "WatchedModelID: " << this->GetWatchedModelNode()->GetID() << std::endl;
  os << indent << "ToolTipTransformID: " << this->GetToolTransformNode()->GetID() << std::endl;
  os << indent << "ToolInsideModel: " << this->ToolInsideModel << std::endl;
}



void
vtkMRMLBreachWarningNode
::SetWarningColor( double r, double g, double b, double a )
{
  this->WarningColor[ 0 ] = r;
  this->WarningColor[ 1 ] = g;
  this->WarningColor[ 2 ] = b;
  this->WarningColor[ 3 ] = a;
}



double
vtkMRMLBreachWarningNode
::GetWarningColorComponent( int c )
{
  if ( c >= 0 && c < 4 )
  {
    return this->WarningColor[ c ];
  }
  else
  {
    vtkErrorMacro( "GetWarningColorComponent: Index out of range" );
    return 0.0;
  }
}



void
vtkMRMLBreachWarningNode
::SetOriginalColor( double r, double g, double b, double a )
{
  this->OriginalColor[ 0 ] = r;
  this->OriginalColor[ 1 ] = g;
  this->OriginalColor[ 2 ] = b;
  this->OriginalColor[ 3 ] = a;
}



double
vtkMRMLBreachWarningNode
::GetOriginalColorComponent( int c )
{
  if ( c >= 0 && c < 4 )
  {
    return this->OriginalColor[ c ];
  }
  else
  {
    vtkErrorMacro( "GetOriginalColorComponent: Index out of range" );
    return 0.0;
  }
}



vtkMRMLModelNode*
vtkMRMLBreachWarningNode
::GetWatchedModelNode()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( MODEL_ROLE ) );
  return modelNode;
}



void
vtkMRMLBreachWarningNode
::SetWatchedModelNodeID( const char* modelId )
{
  vtkMRMLNode* node = this->GetScene()->GetNodeByID( modelId );
  if ( node == NULL )
  {
    this->SetNodeReferenceID( "", modelId );
    return;
  }
  
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( node );
  if ( modelNode == NULL )
  {
    this->SetNodeReferenceID( "", modelId );
    return;
  }
  
  this->SetNodeReferenceID( MODEL_ROLE, modelId );
}
  


vtkMRMLLinearTransformNode*
vtkMRMLBreachWarningNode
::GetToolTransformNode()
{
  vtkMRMLLinearTransformNode* ltNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( TOOL_ROLE ) );
  return ltNode;
}



void
vtkMRMLBreachWarningNode
::SetAndObserveToolTransformNodeId( const char* nodeId )
{
  this->SetAndObserveNodeReferenceID( TOOL_ROLE, nodeId );
}



void
vtkMRMLBreachWarningNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;

  const char* ObservedTransformNodeId = this->GetToolTransformNode()->GetID();
  if ( strcmp( ObservedTransformNodeId, callerNode->GetID() ) == 0 )
  {
    this->Modified(); // This will tell the logic to update
  }
}
