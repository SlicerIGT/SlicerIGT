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

static const char* INPUT_MARKUPS_ROLE = "InputMarkups";
static const char* OUTPUT_MODEL_ROLE = "OutputModel";

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
  this->TubeSegmentsBetweenControlPoints = 5;
  this->TubeNumberOfSides = 8;
  this->ModelType = 0;
  this->InterpolationType = 0;
  this->PointParameterType = 0;

  this->KochanekTension = 0;
  this->KochanekBias = 0;
  this->KochanekContinuity = 0;

  this->PolynomialOrder = 3;
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

  of << indent << " ModelType =\"" << this->GetModelTypeAsString(this->ModelType) << "\"";
  of << indent << " AutoUpdateOutput =\"" << this->AutoUpdateOutput << "\"";
  of << indent << " CleanMarkups =\"" << this->CleanMarkups << "\"";
  of << indent << " ConvexHull =\"" << this->ConvexHull << "\"";
  of << indent << " ButterflySubdivision =\"" << this->ButterflySubdivision << "\"";
  of << indent << " DelaunayAlpha =\"" << this->DelaunayAlpha << "\"";
  of << indent << " InterpolationType=\"" << this->GetInterpolationTypeAsString(this->InterpolationType) << "\"";
  of << indent << " PointParameterType=\"" << this->GetPointParameterTypeAsString(this->PointParameterType) << "\"";
  of << indent << " TubeRadius=\"" << this->TubeRadius << "\"";
  of << indent << " TubeNumberOfSides=\"" << this->TubeNumberOfSides << "\"";
  of << indent << " TubeSegmentsBetweenControlPoints=\"" << this->TubeSegmentsBetweenControlPoints << "\"";
  of << indent << " KochanekBias=\"" << this->KochanekBias << "\"";
  of << indent << " KochanekContinuity=\"" << this->KochanekContinuity << "\"";
  of << indent << " KochanekTension=\"" << this->KochanekTension << "\"";
  of << indent << " PolynomialOrder=\"" << this->PolynomialOrder << "\"";
}

void vtkMRMLMarkupsToModelNode::ReadXMLAttributes( const char** atts )
{
  int disabledModify = this->StartModify();

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
      int typeAsInt = GetModelTypeFromString( attValue );
      if ( typeAsInt >= 0 && typeAsInt < ModelType_Last)
      {
        this->ModelType = typeAsInt;
      }
      else
      {
        vtkWarningMacro("Unrecognized model type read from MRML node: " << attValue << ". Setting to ClosedSurface.");
        this->ModelType = this->ClosedSurface;
      }
    }
    else if ( ! strcmp( attName, "AutoUpdateOutput" ) )
    {
      SetAutoUpdateOutput(!strcmp(attValue,"true"));
    }
    else if ( ! strcmp( attName, "CleanMarkups" ) )
    {
      SetCleanMarkups(!strcmp(attValue,"true"));
    }
    else if ( ! strcmp( attName, "ConvexHull" ) )
    {
      SetConvexHull(!strcmp(attValue,"true"));
    }
    else if ( ! strcmp( attName, "ButterflySubdivision" ) )
    {
      SetButterflySubdivision(!strcmp(attValue,"true"));
    }
    else if ( ! strcmp( attName, "DelaunayAlpha" ) )
    {
      double delaunayAlpha = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> delaunayAlpha;
      SetDelaunayAlpha(delaunayAlpha);
    }
    else if ( ! strcmp( attName, "InterpolationType" ) )
    {
      int typeAsInt = GetInterpolationTypeFromString( attValue );
      if ( typeAsInt >= 0 && typeAsInt < InterpolationType_Last)
      {
        this->InterpolationType = typeAsInt;
      }
      else
      {
        vtkWarningMacro("Unrecognized interpolation type read from MRML node: " << attValue << ". Setting to Linear.");
        this->InterpolationType = this->Linear;
      }
    }
    else if ( ! strcmp( attName, "PointParameterType" ) )
    {
      int typeAsInt = GetPointParameterTypeFromString( attValue );
      if ( typeAsInt >= 0 && typeAsInt < PointParameterType_Last)
      {
        this->PointParameterType = typeAsInt;
      }
      else
      {
        vtkWarningMacro("Unrecognized point parameter type read from MRML node: " << attValue << ". Setting to RawIndices.");
        this->PointParameterType = this->RawIndices;
      }
    }
    else if ( ! strcmp( attName, "TubeRadius" ) )
    {
      double tubeRadius = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> tubeRadius;
      SetTubeRadius(tubeRadius);
    }
    else if ( ! strcmp( attName, "TubeNumberOfSides" ) )
    {
      double tubeNumberOfSides = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> tubeNumberOfSides;
      SetTubeNumberOfSides(tubeNumberOfSides);
    }
    else if ( ! strcmp( attName, "TubeSegmentsBetweenControlPoints" ) )
    {
      double tubeSegmentsBetweenControlPoints = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> tubeSegmentsBetweenControlPoints;
      SetTubeSegmentsBetweenControlPoints(tubeSegmentsBetweenControlPoints);
    }
    else if ( ! strcmp( attName, "KochanekBias" ) )
    {
      double kochanekBias = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> kochanekBias;
      if (kochanekBias < -1.0)
      {
        vtkWarningMacro("Kochanek Bias " << kochanekBias << " is too small. Setting to -1.0.)");
        kochanekBias = -1.0;
      }
      else if (kochanekBias > 1.0)
      {
        vtkWarningMacro("Kochanek Bias " << kochanekBias << " is too large. Setting to 1.0.)");
        kochanekBias = 1.0;
      }
      SetKochanekBias(kochanekBias);
    }
    else if ( ! strcmp( attName, "KochanekContinuity" ) )
    {
      double kochanekContinuity = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> kochanekContinuity;
      if (kochanekContinuity < -1.0)
      {
        vtkWarningMacro("Kochanek Continuity " << kochanekContinuity << " is too small. Setting to -1.0.)");
        kochanekContinuity = -1.0;
      }
      else if (kochanekContinuity > 1.0)
      {
        vtkWarningMacro("Kochanek Continuity " << kochanekContinuity << " is too large. Setting to 1.0.)");
        kochanekContinuity = 1.0;
      }
      SetKochanekContinuity(kochanekContinuity);
    }
    else if ( ! strcmp( attName, "KochanekTension" ) )
    {
      double kochanekTension = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> kochanekTension;
      if (kochanekTension < -1.0)
      {
        vtkWarningMacro("Kochanek Tension " << kochanekTension << " is too small. Setting to -1.0.)");
        kochanekTension = -1.0;
      }
      else if (kochanekTension > 1.0)
      {
        vtkWarningMacro("Kochanek Tension " << kochanekTension << " is too large. Setting to 1.0.)");
        kochanekTension = 1.0;
      }
      SetKochanekTension(kochanekTension);
    }
    else if ( ! strcmp( attName, "PolynomialOrder" ) )
    {
      double polynomialOrder = 0.0;
      std::stringstream nameString;
      nameString << attValue;
      nameString >> polynomialOrder;
      if (polynomialOrder < 1.0)
      {
        vtkWarningMacro("Polynomial Order " << polynomialOrder << " is too small. Setting to 1.)");
        polynomialOrder = 1.0;
      }
      SetPolynomialOrder(polynomialOrder);
    }
  }

  this->EndModify(disabledModify);
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
  case KochanekSpline: return "kochanekSpline";
  case Polynomial: return "polynomial";
  default:
    // invalid id
    return "";
  }
}

const char* vtkMRMLMarkupsToModelNode::GetPointParameterTypeAsString( int id )
{
  switch ( id )
  {
  case RawIndices: return "rawIndices";
  case MinimumSpanningTree: return "minimumSpanningTree";
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

int vtkMRMLMarkupsToModelNode::GetPointParameterTypeFromString( const char* name )
{
  if ( name == NULL )
  {
    // invalid name
    return -1;
  }
  for ( int i = 0; i < PointParameterType_Last; i++ )
  {
    if ( strcmp( name, GetPointParameterTypeAsString( i ) ) == 0 )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

