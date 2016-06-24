// MarkupsToModel MRML includes
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelNode.h"

// Other MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

// VTK includes
#include <vtkNew.h>

// Other includes
#include <sstream>

vtkMRMLMarkupsToModelNode* vtkMRMLMarkupsToModelNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMarkupsToModelNode" );
  if( ret )
  {
    return ( vtkMRMLMarkupsToModelNode* )ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMarkupsToModelNode;
}

vtkMRMLMarkupsToModelNode::vtkMRMLMarkupsToModelNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLMarkupsNode::PointModifiedEvent );//PointEndInteractionEvent

  this->AddNodeReferenceRole( INPUT_MARKUPS_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( OUTPUT_MODEL_ROLE );

  this->AutoUpdateOutput = true;
  this->CleanMarkups = true;
  this->ConvexHull = true;
  this->ButterflySubdivision = true;
  this->DelaunayAlpha = 0.0;
  this->TubeRadius = 1.0;
  this->TubeResolutionLength = 20;
  this->TubeResolutionAround = 20;
  this->ModelType = 0;
  this->InterpolationType = 0;

  this->KochanekTension = 0;
  this->KochanekBias = 0;
  this->KochanekContinuity = 0;
}

vtkMRMLMarkupsToModelNode::~vtkMRMLMarkupsToModelNode()
{
}

vtkMRMLNode* vtkMRMLMarkupsToModelNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMarkupsToModelNode" );
  if( ret )
    {
      return ( vtkMRMLMarkupsToModelNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMarkupsToModelNode;
}

void vtkMRMLMarkupsToModelNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);

  of << indent << " ModelType =\"" << this->ModelType << "\"";
  of << indent << " AutoUpdateOutput =\"" << this->AutoUpdateOutput << "\"";
  of << indent << " CleanMarkups =\"" << this->CleanMarkups << "\"";
  of << indent << " ConvexHull =\"" << this->ConvexHull << "\"";
  of << indent << " ButterflySubdivision =\"" << this->ButterflySubdivision << "\"";
  of << indent << " DelaunayAlpha =\"" << this->DelaunayAlpha << "\"";
  of << indent << " InterpolationType=\"" << this->InterpolationType << "\"";
  of << indent << " TubeRadius=\"" << this->TubeRadius << "\"";
  of << indent << " TubeResolutionAround=\"" << this->TubeResolutionAround << "\"";
  of << indent << " TubeResolutionLength=\"" << this->TubeResolutionLength << "\"";
  of << indent << " KochanekBias=\"" << this->KochanekBias << "\"";
  of << indent << " KochanekContinuity=\"" << this->KochanekContinuity << "\"";
  of << indent << " KochanekTension=\"" << this->KochanekTension << "\"";

}

void vtkMRMLMarkupsToModelNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    if ( ! strcmp( attName, "ModelType" ) )
    {
      int id = this->GetModelTypeFromString(attValue);
      if (id < 0)
      {
        vtkWarningMacro("Invalid ModelType: " << (attValue ? attValue : "(none)"));
      }
      else
      {
        this->ModelType = id;
      }
      std::stringstream nameString;
      nameString << attValue;
      //int r = 0;
      nameString >> this->ModelType;
    }
    else if ( ! strcmp( attName, "AutoUpdateOutput" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->AutoUpdateOutput;
    }
    else if ( ! strcmp( attName, "CleanMarkups" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->CleanMarkups;
    }
    else if ( ! strcmp( attName, "ConvexHull" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->ConvexHull;
    }
    else if ( ! strcmp( attName, "ButterflySubdivision" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->ButterflySubdivision;
    }
    else if ( ! strcmp( attName, "DelaunayAlpha" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->DelaunayAlpha;
    }
    else if ( ! strcmp( attName, "InterpolationType" ) )
    {
      int id = this->GetInterpolationTypeFromString(attValue);
      if (id < 0)
      {
        vtkWarningMacro("Invalid InterpolationType: " << (attValue ? attValue : "(none)"));
      }
      else
      {
        this->InterpolationType = id;
      }
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->InterpolationType;
    }
    else if ( ! strcmp( attName, "TubeRadius" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->TubeRadius;
    }
    else if ( ! strcmp( attName, "TubeResolutionAround" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->TubeResolutionAround;
    }
    else if ( ! strcmp( attName, "TubeResolutionLength" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->TubeResolutionLength;
    }
    else if ( ! strcmp( attName, "KochanekBias" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->KochanekBias;
    }
    else if ( ! strcmp( attName, "KochanekContinuity" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->KochanekContinuity;
    }
    else if ( ! strcmp( attName, "KochanekTension" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      nameString >> this->KochanekTension;
    }
  }

  this->Modified();
}

void vtkMRMLMarkupsToModelNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLMarkupsToModelNode *node = ( vtkMRMLMarkupsToModelNode* ) anode;
  this->Modified();
}

void vtkMRMLMarkupsToModelNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "ModelID: ";
}

vtkMRMLMarkupsFiducialNode * vtkMRMLMarkupsToModelNode::GetMarkupsNode()
{
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( INPUT_MARKUPS_ROLE ) );
  return markupsNode;
}

vtkMRMLModelNode * vtkMRMLMarkupsToModelNode::GetModelNode()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  return modelNode;
}

void vtkMRMLMarkupsToModelNode::SetAndObserveMarkupsNodeID( const char* markupsId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.
  const char* currentNodeId=this->GetNodeReferenceID(INPUT_MARKUPS_ROLE);
  if (markupsId!=NULL && currentNodeId!=NULL)
  {
    if (strcmp(markupsId,currentNodeId)==0)
    {
      // not changed
      return;
    }
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLMarkupsNode::PointModifiedEvent ); // PointEndInteractionEvent 
  this->SetAndObserveNodeReferenceID( INPUT_MARKUPS_ROLE, markupsId, events.GetPointer() );
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

void vtkMRMLMarkupsToModelNode::SetAndObserveModelNodeID( const char* modelId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.
  const char* currentNodeId = this->GetNodeReferenceID( OUTPUT_MODEL_ROLE );
  if ( modelId != NULL && currentNodeId != NULL )
  {
    if ( strcmp( modelId, currentNodeId ) == 0 )
    {
      // not changed
      return;
    }
  }
  this->SetAndObserveNodeReferenceID( OUTPUT_MODEL_ROLE, modelId );
}

void vtkMRMLMarkupsToModelNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;

  if ( this->GetMarkupsNode() && this->GetMarkupsNode()==caller )
  {
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

std::string vtkMRMLMarkupsToModelNode::GetModelNodeName()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return std::string( this->GetID() ).append( "Model" );
  }
  else
  {
    return std::string( modelNode->GetName() );
  }
}

std::string vtkMRMLMarkupsToModelNode::GetModelDisplayNodeName()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return std::string( this->GetID() ).append( "ModelDisplay" );
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    return std::string( this->GetID() ).append( "ModelDisplay" );
  }

  return std::string( displayNode->GetName() );
}

std::string vtkMRMLMarkupsToModelNode::GetMarkupsDisplayNodeName()
{
  vtkMRMLModelNode* markupsNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( markupsNode == NULL )
  {
    return std::string( this->GetID() ).append( "MarkupsDisplay" );
  }
  
  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    return std::string( this->GetID() ).append( "MarkupsDisplay" );
  }

  return std::string( displayNode->GetName() );
}

void vtkMRMLMarkupsToModelNode::SetOutputOpacity( double outputOpacity )
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }

  displayNode->SetOpacity( outputOpacity );
}

void vtkMRMLMarkupsToModelNode::SetOutputVisibility( bool outputVisibility )
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }

  if ( outputVisibility )
  {
    displayNode->VisibilityOn();
  }
  else
  {
    displayNode->VisibilityOff();
  }
}

void vtkMRMLMarkupsToModelNode::SetOutputIntersectionVisibility( bool outputIntersectionVisibility )
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }

  if ( outputIntersectionVisibility )
  {
    displayNode->SliceIntersectionVisibilityOn();
  }
  else
  {
    displayNode->SliceIntersectionVisibilityOff();
  }
}

void vtkMRMLMarkupsToModelNode::SetOutputColor( double redComponent, double greenComponent, double blueComponent )
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }

  double outputColor[3];
  outputColor[0] = redComponent;
  outputColor[1] = greenComponent;
  outputColor[2] = blueComponent;
  displayNode->SetColor( outputColor );
}

void vtkMRMLMarkupsToModelNode::SetMarkupsTextScale( double scale )
{
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( INPUT_MARKUPS_ROLE ) );
  if ( markupsNode == NULL )
  {
    return;
  }

  vtkMRMLMarkupsDisplayNode * displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveMarkupsDisplayNode();
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  }
  displayNode->SetTextScale( scale );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double vtkMRMLMarkupsToModelNode::GetOutputOpacity()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return 1.0;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }
  return displayNode->GetOpacity();
}

bool vtkMRMLMarkupsToModelNode::GetOutputVisibility()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    return true;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }
  return displayNode->GetVisibility();  
}

bool vtkMRMLMarkupsToModelNode::GetOutputIntersectionVisibility()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL)
  {
    return true;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }
  return displayNode->GetSliceIntersectionVisibility();
}

void vtkMRMLMarkupsToModelNode::GetOutputColor( double outputColor[3] )
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL )
  {
    outputColor[0] = 0;
    outputColor[1] = 0;
    outputColor[2] = 0;
    return;
  }
  
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveModelDisplayNode();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  }
  displayNode->GetColor( outputColor[0], outputColor[1], outputColor[2] );
}

double vtkMRMLMarkupsToModelNode::GetMarkupsTextScale()
{
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( INPUT_MARKUPS_ROLE ) );
  if ( markupsNode == NULL )
  {
    return 1.0;
  }

  vtkMRMLMarkupsDisplayNode * displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  if ( displayNode == NULL )
  {
    this->createAndObserveMarkupsDisplayNode();
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  }
  return displayNode->GetTextScale();
}

//-----------------------------------------------------------------
void vtkMRMLMarkupsToModelNode::createAndObserveModelDisplayNode()
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( OUTPUT_MODEL_ROLE ) );
  if ( modelNode == NULL)
  {
    return;
  }
  
  vtkNew< vtkMRMLModelDisplayNode > displayNode;
  this->GetScene()->AddNode( displayNode.GetPointer() );
  displayNode->SetName( GetModelDisplayNodeName().c_str() );
  displayNode->SetColor( 0.5, 0.5, 0.5 );
  modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
}

//-----------------------------------------------------------------
void vtkMRMLMarkupsToModelNode::createAndObserveMarkupsDisplayNode()
{
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->GetNodeReference( INPUT_MARKUPS_ROLE ) );
  if ( markupsNode == NULL)
  {
    return;
  }

  vtkNew< vtkMRMLMarkupsDisplayNode > displayNode;
  this->GetScene()->AddNode( displayNode.GetPointer() );
  displayNode->SetName( this->GetModelDisplayNodeName().c_str() );
  displayNode->SetColor( 1.0, 0.5, 0.5 );
  markupsNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
}

//-----------------------------------------------------------------
const char* vtkMRMLMarkupsToModelNode::GetModelTypeAsString( int id )
{
  switch ( id )
  {
  case ClosedSurface: return "closedSurface";
  case Curve: return "curve";
  default:
    // invalid id
    return "";
  }
}

const char* vtkMRMLMarkupsToModelNode::GetInterpolationTypeAsString( int id )
{
  switch ( id )
  {
  case Linear: return "linear";
  case CardinalSpline: return "cardinalSpline";
  case HermiteSpline: return "hermiteSpline";
  case KochanekSpline: return "kochanekSpline";
  default:
    // invalid id
    return "";
  }
}

int vtkMRMLMarkupsToModelNode::GetModelTypeFromString( const char* name )
{
  if ( name == NULL )
  {
    // invalid name
    return -1;
  }
  for ( int i = 0; i < ModelType_Last; i++ )
  {
    if ( strcmp( name, GetModelTypeAsString( i ) ) == 0 )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

int vtkMRMLMarkupsToModelNode::GetInterpolationTypeFromString( const char* name )
{
  if ( name == NULL )
  {
    // invalid name
    return -1;
  }
  for ( int i = 0; i < InterpolationType_Last; i++ )
  {
    if ( strcmp( name, GetInterpolationTypeAsString( i ) ) == 0 )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
