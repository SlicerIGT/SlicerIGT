
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

  if ( strcmp( this->GetToolTransformNode()->GetID(), callerNode->GetID() ) == 0 
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
