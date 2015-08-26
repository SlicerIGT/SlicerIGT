
// BreachWarning MRML includes
#include "vtkMRMLBreachWarningNode.h"

// Other MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkIntArray.h>
#include <vtkCommand.h>
#include <vtkCollection.h>

// Other includes
#include <sstream>

// Constants
static const char* MODEL_ROLE = "watchedModelNode";
static const char* TOOL_ROLE = "toolTransformNode";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLBreachWarningNode);

vtkMRMLBreachWarningNode
::vtkMRMLBreachWarningNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent );

  this->AddNodeReferenceRole( MODEL_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( TOOL_ROLE, NULL, events.GetPointer() );  
    
  // TrajectoryModelNode
	//vtkCollection* trajectoryModelNodeCollection = this->GetScene()->GetNodesByName("TrajectoryModel");
	//vtkMRMLModelNode* trajectoryModelNode = vtkMRMLModelNode::SafeDownCast( trajectoryModelNodeCollection->GetItemAsObject( 0 ) );	
	//if (trajectoryModelNode != NULL)
	//{
	//	this->TrajectoryModelNode = trajectoryModelNode;
	//}
 // else
 // {
 //   
 // }
	//trajectoryModelNodeCollection->Delete();
  
 // // CrossHairFiducialNode  
	//vtkCollection* crossHairFiducialNodeCollection = this->GetScene()->GetNodesByName("CrossHairFiducial");
	//vtkMRMLMarkupsFiducialNode* crossHairFiducial = vtkMRMLMarkupsFiducialNode::SafeDownCast( crossHairFiducialNodeCollection->GetItemAsObject( 0 ) );	
	//if (crossHairFiducial != NULL)
	//{
	//	this->CrossHairFiducialNode = crossHairFiducial;
	//}
 // else
 // {
 //   
 // }
	//crossHairFiducialNodeCollection->Delete();

 // // TipFiducialNode
 // vtkCollection* tipFiducialNodeCollection = this->GetScene()->GetNodesByName("TipFiducial");
	//vtkMRMLMarkupsFiducialNode* tipFiducial = vtkMRMLMarkupsFiducialNode::SafeDownCast( tipFiducialNodeCollection->GetItemAsObject( 0 ) );	
	//if (tipFiducial != NULL)
	//{
	//	this->TipFiducialNode = tipFiducial;
	//}
 // else
 // {
 //   
 // }
	//tipFiducialNodeCollection->Delete();

  // Colors
  this->OriginalColor[0] = 0.5;
  this->OriginalColor[1] = 0.5;
  this->OriginalColor[2] = 0.5;

  this->WarningColor[0] = 1; // Red
  this->WarningColor[1] = 0;
  this->WarningColor[2] = 0;

  this->TrajectoryColor[0] = 0; // Green
  this->TrajectoryColor[1] = 1;
  this->TrajectoryColor[2] = 0;

  this->DistanceColor[0] = 1; // White
  this->DistanceColor[1] = 1;
  this->DistanceColor[2] = 1;

  this->CrossHairColor[0] = 0; // Blue
  this->CrossHairColor[1] = 0;
  this->CrossHairColor[2] = 1;

  // Enabled
  this->DisplayWarningColor = true;
  this->PlayWarningSound = false;
  this->DisplayTrajectory = true;
  this->DisplayCrossHair = true;
  this->DisplayDistance = true;

  this->ClosestDistanceToModelFromToolTip = 0.0;
}

vtkMRMLBreachWarningNode
::~vtkMRMLBreachWarningNode()
{
}

void
vtkMRMLBreachWarningNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);

  of << indent << " warningColor=\"" << this->WarningColor[0] << " " << this->WarningColor[1] << " " << this->WarningColor[2] << "\"";
  of << indent << " originalColor=\"" << this->OriginalColor[0] << " " << this->OriginalColor[1] << " " << this->OriginalColor[2] << "\"";
  of << indent << " trajectoryColor=\"" << this->TrajectoryColor[0] << " " << this->TrajectoryColor[1] << " " << this->TrajectoryColor[2] << "\"";
  of << indent << " distanceColor=\"" << this->DistanceColor[0] << " " << this->DistanceColor[1] << " " << this->DistanceColor[2] << "\"";
  of << indent << " crossHairColor=\"" << this->CrossHairColor[0] << " " << this->CrossHairColor[1] << " " << this->CrossHairColor[2] << "\"";
  of << indent << " displayWarningColor=\"" << ( this->DisplayWarningColor ? "true" : "false" ) << "\"";
  of << indent << " playWarningSound=\"" << ( this->PlayWarningSound ? "true" : "false" ) << "\"";
  of << indent << " displayTrajectory=\"" << ( this->DisplayTrajectory ? "true" : "false" ) << "\"";
  of << indent << " displayCrossHair=\"" << ( this->DisplayCrossHair ? "true" : "false" ) << "\"";
  of << indent << " displayDistance=\"" << ( this->DisplayDistance ? "true" : "false" ) << "\"";
  of << indent << " closestDistanceToModelFromToolTip=\"" << ClosestDistanceToModelFromToolTip << "\"";
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
    else if (!strcmp(attName, "trajectoryColor"))
    {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->TrajectoryColor[0] = val;
      ss >> val;
      this->TrajectoryColor[1] = val;
      ss >> val;
      this->TrajectoryColor[2] = val;
    }
    else if (!strcmp(attName, "distanceColor"))
    {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->DistanceColor[0] = val;
      ss >> val;
      this->DistanceColor[1] = val;
      ss >> val;
      this->DistanceColor[2] = val;
    }
    else if (!strcmp(attName, "crossHairColor"))
    {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->CrossHairColor[0] = val;
      ss >> val;
      this->CrossHairColor[1] = val;
      ss >> val;
      this->CrossHairColor[2] = val;
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
    else if ( ! strcmp( attName, "displayTrajectory" ) )
    {
      if (!strcmp(attValue,"true"))
      {
        this->DisplayTrajectory = true;
      }
      else
      {
        this->DisplayTrajectory = false;
      }
    }
    else if ( ! strcmp( attName, "displayCrossHair" ) )
    {
      if (!strcmp(attValue,"true"))
      {
        this->DisplayCrossHair = true;
      }
      else
      {
        this->DisplayCrossHair = false;
      }
    }
    else if ( ! strcmp( attName, "displayDistance" ) )
    {
      if (!strcmp(attValue,"true"))
      {
        this->DisplayDistance = true;
      }
      else
      {
        this->DisplayDistance = false;
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
  }
}

void
vtkMRMLBreachWarningNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  
  vtkMRMLBreachWarningNode *node = ( vtkMRMLBreachWarningNode* ) anode;
  
  for ( int i = 0; i < 3; ++ i )
  {
    this->WarningColor[ i ] = node->WarningColor[ i ];
    this->OriginalColor[ i ] = node->OriginalColor[ i ];
    this->TrajectoryColor[ i ] = node->TrajectoryColor[ i ];
    this->CrossHairColor[ i ] = node->CrossHairColor[ i ];
    this->DistanceColor[ i ] = node->DistanceColor[ i ];
  }

  this->PlayWarningSound = node->PlayWarningSound;  
  this->DisplayWarningColor = node->DisplayWarningColor;
  this->DisplayCrossHair = node->DisplayCrossHair;
  this->DisplayDistance = node->DisplayDistance;
  this->DisplayTrajectory = node->DisplayTrajectory;
  
  this->Modified();
}

void
vtkMRMLBreachWarningNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes

  os << indent << "WatchedModelID: " << this->GetWatchedModelNode()->GetID() << std::endl;
  os << indent << "ToolTipTransformID: " << this->GetToolTransformNode()->GetID() << std::endl;
  os << indent << "DisplayWarningColor: " << this->DisplayWarningColor << std::endl;
  os << indent << "DisplayCrossHair: " << this->DisplayCrossHair << std::endl;
  os << indent << "DisplayTrajectory: " << this->DisplayTrajectory << std::endl;
  os << indent << "DisplayDistance: " << this->DisplayDistance << std::endl;
  os << indent << "PlayWarningSound: " << this->PlayWarningSound << std::endl;
  os << indent << "WarningColor: " << this->WarningColor[0] << ", " << this->WarningColor[1] << ", " << this->WarningColor[2] << std::endl;
  os << indent << "OriginalColor: " << this->OriginalColor[0] << ", " << this->OriginalColor[1] << ", " << this->OriginalColor[2] << std::endl;
  os << indent << "TrajectoryColor: " << this->TrajectoryColor[0] << ", " << this->TrajectoryColor[1] << ", " << this->TrajectoryColor[2] << std::endl;
  os << indent << "CrossHairColor: " << this->CrossHairColor[0] << ", " << this->CrossHairColor[1] << ", " << this->CrossHairColor[2] << std::endl;
  os << indent << "DistanceColor: " << this->DistanceColor[0] << ", " << this->DistanceColor[1] << ", " << this->DistanceColor[2] << std::endl;
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
::SetAndObserveWatchedModelNodeID( const char* modelId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the reques, but it adds another observer instead)
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

vtkMRMLTransformNode*
vtkMRMLBreachWarningNode
::GetToolTransformNode()
{
  vtkMRMLTransformNode* ltNode = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( TOOL_ROLE ) );
  return ltNode;
}

void
vtkMRMLBreachWarningNode
::SetAndObserveToolTransformNodeId( const char* nodeId )
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

void
vtkMRMLBreachWarningNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
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

bool 
vtkMRMLBreachWarningNode::IsToolTipInsideModel()
{
  return (this->ClosestDistanceToModelFromToolTip<0);
}

void 
vtkMRMLBreachWarningNode::SetDisplayWarningColor(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DisplayWarningColor to " << _arg);
  if (this->DisplayWarningColor != _arg)
  {
    this->DisplayWarningColor = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetPlayWarningSound(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting PlayWarningSound to " << _arg);
  if (this->PlayWarningSound != _arg)
  {
    this->PlayWarningSound = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetDisplayTrajectory(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DisplayTrajectory to " << _arg);
  if (this->DisplayTrajectory != _arg)
  {
    this->DisplayTrajectory = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetDisplayCrossHair(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DisplayCrossHair to " << _arg);
  if (this->DisplayCrossHair != _arg)
  {
    this->DisplayCrossHair = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetDisplayDistance(bool _arg)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DisplayDistance to " << _arg);
  if (this->DisplayDistance != _arg)
  {
    this->DisplayDistance = _arg;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetWarningColor(double _arg1, double _arg2, double _arg3)
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

void 
vtkMRMLBreachWarningNode::SetWarningColor(double _arg[3])
{
  this->SetWarningColor(_arg[0], _arg[1], _arg[2]);
}

void 
vtkMRMLBreachWarningNode::SetOriginalColor(double _arg1, double _arg2, double _arg3)
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

void 
vtkMRMLBreachWarningNode::SetOriginalColor(double _arg[3])
{
  this->SetOriginalColor(_arg[0], _arg[1], _arg[2]);
}

void 
vtkMRMLBreachWarningNode::SetTrajectoryColor(double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting TrajectoryColor to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->TrajectoryColor[0] != _arg1)||(this->TrajectoryColor[1] != _arg2)||(this->TrajectoryColor[2] != _arg3))
  {
    this->TrajectoryColor[0] = _arg1;
    this->TrajectoryColor[1] = _arg2;
    this->TrajectoryColor[2] = _arg3;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetTrajectoryColor(double _arg[3])
{
  this->SetWarningColor(_arg[0], _arg[1], _arg[2]);
}

void 
vtkMRMLBreachWarningNode::SetDistanceColor(double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting DistanceColor to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->DistanceColor[0] != _arg1)||(this->DistanceColor[1] != _arg2)||(this->DistanceColor[2] != _arg3))
  {
    this->DistanceColor[0] = _arg1;
    this->DistanceColor[1] = _arg2;
    this->DistanceColor[2] = _arg3;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetDistanceColor(double _arg[3])
{
  this->SetWarningColor(_arg[0], _arg[1], _arg[2]);
}

void 
vtkMRMLBreachWarningNode::SetCrossHairColor(double _arg1, double _arg2, double _arg3)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting CrossHairColor to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")");
  if ((this->CrossHairColor[0] != _arg1)||(this->CrossHairColor[1] != _arg2)||(this->CrossHairColor[2] != _arg3))
  {
    this->CrossHairColor[0] = _arg1;
    this->CrossHairColor[1] = _arg2;
    this->CrossHairColor[2] = _arg3;
    this->Modified();
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void 
vtkMRMLBreachWarningNode::SetCrossHairColor(double _arg[3])
{
  this->SetWarningColor(_arg[0], _arg[1], _arg[2]);
}