
// BreachWarning MRML includes
#include "vtkMRMLBreachWarningNode.h"

// Other MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkIntArray.h>
#include <vtkCommand.h>

// Other includes
#include <sstream>

// Constants
static const char* MODEL_ROLE = "watchedModelNode";
static const char* TOOL_ROLE = "toolTransformNode";
static const char* LINE_TO_CLOSEST_POINT_ROLE = "lineToClosestPointNode";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLBreachWarningNode);

//------------------------------------------------------------------------------
vtkMRMLBreachWarningNode::vtkMRMLBreachWarningNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent );

  this->AddNodeReferenceRole( MODEL_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( TOOL_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( LINE_TO_CLOSEST_POINT_ROLE, NULL, events.GetPointer() );

  this->OriginalColor[0] = 0.5;
  this->OriginalColor[1] = 0.5;
  this->OriginalColor[2] = 0.5;

  this->WarningColor[0] = 1; // Red
  this->WarningColor[1] = 0;
  this->WarningColor[2] = 0;


  this->DisplayWarningColor = true;
  this->PlayWarningSound = false;

  this->ClosestDistanceToModelFromToolTip = 0.0;

  this->ClosestPointOnModel[0] = 0.0;
  this->ClosestPointOnModel[1] = 0.0;
  this->ClosestPointOnModel[2] = 0.0;

}

//------------------------------------------------------------------------------
vtkMRMLBreachWarningNode::~vtkMRMLBreachWarningNode()
{
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);

  of << indent << " warningColor=\"" << this->WarningColor[0] << " " << this->WarningColor[1] << " " << this->WarningColor[2] << "\"";
  of << indent << " originalColor=\"" << this->OriginalColor[0] << " " << this->OriginalColor[1] << " " << this->OriginalColor[2] << "\"";
  of << indent << " displayWarningColor=\"" << ( this->DisplayWarningColor ? "true" : "false" ) << "\"";
  of << indent << " playWarningSound=\"" << ( this->PlayWarningSound ? "true" : "false" ) << "\"";
  of << indent << " closestDistanceToModelFromToolTip=\"" << ClosestDistanceToModelFromToolTip << "\"";
  of << indent << " closestPointOnModel=\"" << this->ClosestPointOnModel[0] << " " << this->ClosestPointOnModel[1] << " " << this->ClosestPointOnModel[2] << "\"";
}

//------------------------------------------------------------------------------
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
    
    if (!strcmp(attName, "warningColor"))
    {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->WarningColor[0] = val;
      ss >> val;
      this->WarningColor[1] = val;
      ss >> val;
      this->WarningColor[2] = val;
    }
    else if (!strcmp(attName, "originalColor"))
    {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->OriginalColor[0] = val;
      ss >> val;
      this->OriginalColor[1] = val;
      ss >> val;
      this->OriginalColor[2] = val;
    }
    else if ( ! strcmp( attName, "displayWarningColor" ) )
    {
      if (!strcmp(attValue,"true"))
      {
        this->DisplayWarningColor = true;
      }
      else
      {
        this->DisplayWarningColor = false;
      }
    }
    else if ( ! strcmp( attName, "playWarningSound" ) )
    {
      if (!strcmp(attValue,"true"))
      {
        this->PlayWarningSound = true;
      }
      else
      {
        this->PlayWarningSound = false;
      }
    }
    else if (!strcmp(attName, "closestDistanceToModelFromToolTip"))
    {
      std::stringstream ss;
      ss << attValue;
      double val=0.0;
      ss >> val;
      this->ClosestDistanceToModelFromToolTip = val;
    }
    else if (!strcmp(attName, "closestPointOnModel"))
    {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->ClosestPointOnModel[0] = val;
      ss >> val;
      this->ClosestPointOnModel[1] = val;
      ss >> val;
      this->ClosestPointOnModel[2] = val;
    }
  }
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  
  vtkMRMLBreachWarningNode *node = ( vtkMRMLBreachWarningNode* ) anode;
  
  for ( int i = 0; i < 3; ++ i )
  {
    this->WarningColor[ i ] = node->WarningColor[ i ];
    this->OriginalColor[ i ] = node->OriginalColor[ i ];
  }

  this->PlayWarningSound = node->PlayWarningSound;  
  this->DisplayWarningColor = node->DisplayWarningColor;

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes

  os << indent << "WatchedModelID: " << (this->GetWatchedModelNode() && this->GetWatchedModelNode()->GetID() ?
   this->GetWatchedModelNode()->GetID() : "(none)" ) << std::endl;
  os << indent << "ToolTipTransformID: " << (this->GetToolTransformNode() && this->GetToolTransformNode()->GetID() ?
   this->GetToolTransformNode()->GetID() : "(none)" ) << std::endl;
  os << indent << "LineToClosestPointID: " << (this->GetLineToClosestPointNode() && this->GetLineToClosestPointNode()->GetID() ?
   this->GetLineToClosestPointNode()->GetID() : "(none)" ) << std::endl;
  os << indent << "DisplayWarningColor: " << this->DisplayWarningColor << std::endl;
  os << indent << "PlayWarningSound: " << this->PlayWarningSound << std::endl;
  os << indent << "WarningColor: " << this->WarningColor[0] << ", " << this->WarningColor[1] << ", " << this->WarningColor[2] << std::endl;
  os << indent << "OriginalColor: " << this->OriginalColor[0] << ", " << this->OriginalColor[1] << ", " << this->OriginalColor[2] << std::endl;
}

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLBreachWarningNode::GetWatchedModelNode()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( MODEL_ROLE ) );
  return modelNode;
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetAndObserveWatchedModelNodeID( const char* modelId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.
  const char* currentNodeId=this->GetNodeReferenceID(MODEL_ROLE);
  if (modelId!=NULL && currentNodeId!=NULL)
  {
    if (strcmp(modelId,currentNodeId)==0)
    {
      // not changed
      return;
    }
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLTransformNode::TransformModifiedEvent );
  this->SetAndObserveNodeReferenceID( MODEL_ROLE, modelId, events.GetPointer() );
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode* vtkMRMLBreachWarningNode::GetLineToClosestPointNode()
{
  vtkMRMLAnnotationRulerNode* lineToClosestPointNode = vtkMRMLAnnotationRulerNode::SafeDownCast( this->GetNodeReference( LINE_TO_CLOSEST_POINT_ROLE ) );
  return lineToClosestPointNode;
}

//------------------------------------------------------------------------------
const char* vtkMRMLBreachWarningNode::GetLineToClosestPointNodeID()
{
  return this->GetNodeReferenceID( LINE_TO_CLOSEST_POINT_ROLE );
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetLineToClosestPointNodeID( const char* nodeId )
{
  const char* currentNodeId=this->GetNodeReferenceID(LINE_TO_CLOSEST_POINT_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL && strcmp(nodeId,currentNodeId)==0)
  {
    // not changed
    return;
  }
  this->SetNodeReferenceID( LINE_TO_CLOSEST_POINT_ROLE, nodeId);
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}


//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLBreachWarningNode::GetToolTransformNode()
{
  vtkMRMLTransformNode* ltNode = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( TOOL_ROLE ) );
  return ltNode;
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetAndObserveToolTransformNodeId( const char* nodeId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.
  const char* currentNodeId=this->GetNodeReferenceID(TOOL_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL)
  {
    if (strcmp(nodeId,currentNodeId)==0)
    {
      // not changed
      return;
    }
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLTransformNode::TransformModifiedEvent );
  this->SetAndObserveNodeReferenceID( TOOL_ROLE, nodeId, events.GetPointer() );
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;

  if (this->GetToolTransformNode() && this->GetToolTransformNode()==caller)
  {
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
  else if (this->GetWatchedModelNode() && this->GetWatchedModelNode()==caller)
  {
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

//------------------------------------------------------------------------------
bool vtkMRMLBreachWarningNode::IsToolTipInsideModel()
{
  return (this->ClosestDistanceToModelFromToolTip<0);
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetDisplayWarningColor(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DisplayWarningColor to " << _arg);
  if (this->DisplayWarningColor != _arg)
  {
    this->DisplayWarningColor = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetPlayWarningSound(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting PlayWarningSound to " << _arg);
  if (this->PlayWarningSound != _arg)
  {
    this->PlayWarningSound = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetWarningColor(double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting WarningColor to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->WarningColor[0] != _arg1)||(this->WarningColor[1] != _arg2)||(this->WarningColor[2] != _arg3))
  {
    this->WarningColor[0] = _arg1;
    this->WarningColor[1] = _arg2;
    this->WarningColor[2] = _arg3;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetWarningColor(double _arg[3])
{
  this->SetWarningColor(_arg[0], _arg[1], _arg[2]);
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetOriginalColor(double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting OriginalColor to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->OriginalColor[0] != _arg1)||(this->OriginalColor[1] != _arg2)||(this->OriginalColor[2] != _arg3))
  {
    this->OriginalColor[0] = _arg1;
    this->OriginalColor[1] = _arg2;
    this->OriginalColor[2] = _arg3;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

//------------------------------------------------------------------------------
void vtkMRMLBreachWarningNode::SetOriginalColor(double _arg[3])
{
  this->SetOriginalColor(_arg[0], _arg[1], _arg[2]);
}
