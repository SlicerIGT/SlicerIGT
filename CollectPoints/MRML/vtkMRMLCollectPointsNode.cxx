/*==============================================================================

  Copyright (c) Thomas Vaughan
  Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// CollectPoints includes
#include "vtkMRMLCollectPointsNode.h"

// slicer includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

// vtk includes
#include <vtkNew.h>
#include <vtkPolyData.h>

// std includes
#include <sstream>

// Constants ------------------------------------------------------------------
static const char* SAMPLING_TRANSFORM_REFERENCE_ROLE = "ProbeTransformNode";
static const char* ANCHOR_TRANSFORM_REFERENCE_ROLE = "AnchorTransformNode";
static const char* OUTPUT_REFERENCE_ROLE = "OutputNode";

vtkMRMLNodeNewMacro( vtkMRMLCollectPointsNode );

//------------------------------------------------------------------------------
vtkMRMLCollectPointsNode::vtkMRMLCollectPointsNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkNew<vtkIntArray> transformListEvents;
  transformListEvents->InsertNextValue( vtkMRMLTransformNode::TransformModifiedEvent );

  this->AddNodeReferenceRole( SAMPLING_TRANSFORM_REFERENCE_ROLE, NULL, transformListEvents.GetPointer() );
  this->AddNodeReferenceRole( ANCHOR_TRANSFORM_REFERENCE_ROLE, NULL, transformListEvents.GetPointer() );
  this->AddNodeReferenceRole( OUTPUT_REFERENCE_ROLE );
  this->LabelBase = "P";
  this->NextLabelNumber = 0;
  this->MinimumDistance = 10.0;
  this->CollectMode = Manual;
}

//------------------------------------------------------------------------------
vtkMRMLCollectPointsNode::~vtkMRMLCollectPointsNode()
{
}

// ----------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML( of, nIndent ); // This will take care of referenced nodes

  vtkIndent indent( nIndent ); 
  of << indent << " LabelBase=\"" << this->LabelBase << "\"";
  of << indent << " NextLabelNumber=\"" << this->NextLabelNumber << "\"";
  of << indent << " MinimumDistance=\"" << this->MinimumDistance << "\"";
  of << indent << " CollectMode=\"" << this->GetCollectModeAsString( this->CollectMode ) << "\"";
}

// ----------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
  os << indent << " LabelBase=\"" << this->LabelBase << "\"";
  os << indent << " NextLabelNumber=\"" << this->NextLabelNumber << "\"";
  os << indent << " MinimumDistance=\"" << this->MinimumDistance << "\"";
  os << indent << " CollectMode=\"" << this->GetCollectModeAsString(  this->CollectMode ) << "\"";
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "LabelBase" ) )
    {
      this->LabelBase = std::string(attValue);
      continue;
    }
    else if ( ! strcmp( attName, "NextLabelNumber" ) )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->NextLabelNumber;
      continue;
    }
    else if ( ! strcmp( attName, "MinimumDistance" ) )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MinimumDistance;
      continue;
    }
    else if ( ! strcmp( attName, "CollectMode" ) )
    {
      int modeAsInt = GetCollectModeFromString( attValue );
      if (modeAsInt >= 0 && modeAsInt < CollectMode_Last)
      {
        this->CollectMode = modeAsInt;
      }
      else
      {
        vtkWarningMacro("Unrecognized collect mode read from MRML node: " << attValue << ". Setting to manual.")
        this->CollectMode = Manual;
      }
    }
  }

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  this->Modified();
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLCollectPointsNode::GetSamplingTransformNode()
{
  vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( SAMPLING_TRANSFORM_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::SetAndObserveSamplingTransformNodeID( const char* nodeID )
{
  this->SetAndObserveNodeReferenceID( SAMPLING_TRANSFORM_REFERENCE_ROLE, nodeID );
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLCollectPointsNode::GetAnchorTransformNode()
{
  vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( ANCHOR_TRANSFORM_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::SetAndObserveAnchorTransformNodeID( const char* nodeID )
{
  this->SetAndObserveNodeReferenceID( ANCHOR_TRANSFORM_REFERENCE_ROLE, nodeID );
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::CreateDefaultDisplayNodesForOutputNode()
{
  vtkMRMLNode* outputNode = this->GetOutputNode();
  if ( outputNode == NULL )
  {
    vtkErrorMacro( "Output node is null. Cannot create display nodes for null node." );
    return;
  }

  // handle markups outputs  
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
  if ( outputMarkupsNode != NULL && outputMarkupsNode->GetDisplayNode() == NULL )
  {
    outputMarkupsNode->CreateDefaultDisplayNodes();
  }

  // handle model outputs
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
  if ( outputModelNode != NULL && outputModelNode->GetModelDisplayNode() == NULL )
  {
    // display node should be set to show points only
    outputModelNode->CreateDefaultDisplayNodes();
    vtkMRMLModelDisplayNode* outputModelDisplayNode = outputModelNode->GetModelDisplayNode();
    outputModelDisplayNode->SetRepresentation( vtkMRMLDisplayNode::PointsRepresentation );
    const int DEFAULT_POINT_SIZE = 5; // This could be made an advanced display setting in the GUI
    outputModelDisplayNode->SetPointSize( DEFAULT_POINT_SIZE );
  }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLCollectPointsNode::GetOutputNode()
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast( this->GetNodeReference( OUTPUT_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
int vtkMRMLCollectPointsNode::GetNumberOfPointsInOutput()
{
  vtkMRMLNode* outputNode = this->GetOutputNode();
  if ( outputNode == NULL )
  {
    return 0;
  }

  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
  if ( outputMarkupsNode != NULL )
  {
    return outputMarkupsNode->GetNumberOfFiducials();
  }
  else if ( outputModelNode != NULL )
  {
    vtkPolyData* outputPolyData = outputModelNode->GetPolyData();
    if ( outputPolyData == NULL )
    {
      return 0;
    }
    return outputPolyData->GetNumberOfPoints();
  }
  else
  {
    vtkErrorMacro( "Unsupported node type in output. Returning 0." );
    return 0;
  }
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::SetOutputNodeID( const char* nodeID )
{
  this->SetAndObserveNodeReferenceID( OUTPUT_REFERENCE_ROLE, nodeID );
}

//------------------------------------------------------------------------------
void vtkMRMLCollectPointsNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void* callData )
{
  Superclass::ProcessMRMLEvents( caller, event, callData );

  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
  {
    return;
  }
  else if ( callerNode == this->GetSamplingTransformNode() ||
            callerNode == this->GetAnchorTransformNode() )
  {
    if ( event == vtkMRMLTransformNode::TransformModifiedEvent )
    {
      this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
    }
  }
}

//------------------------------------------------------------------------------
int vtkMRMLCollectPointsNode::GetCollectModeFromString( const char* name )
{
  if ( name == NULL )
  {
    // invalid name
    return -1;
  }
  for ( int i = 0; i < CollectMode_Last; i++ )
  {
    if ( strcmp( name, GetCollectModeAsString( i ) ) == 0 )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//------------------------------------------------------------------------------
const char* vtkMRMLCollectPointsNode::GetCollectModeAsString( int id )
{
  switch ( id )
  {
  case Manual: return "manual";
  case Automatic: return "automatic";
  default:
    // invalid id
    return "";
  }
}

//------------------------------------------------------------------------------
// DEPRECATED March 8 2018
vtkMRMLTransformNode* vtkMRMLCollectPointsNode::GetProbeTransformNode()
{
  vtkWarningMacro( "GetProbeTransformNode is deprecated. Use GetSamplingTransformNode instead.");
  return this->GetSamplingTransformNode();
}

//------------------------------------------------------------------------------
// DEPRECATED March 8 2018
void vtkMRMLCollectPointsNode::SetAndObserveProbeTransformNodeID( const char* nodeID )
{
  vtkWarningMacro( "SetAndObserveProbeTransformNodeID is deprecated. Use SetAndObserveSamplingTransformNodeID instead.");
  this->SetAndObserveSamplingTransformNodeID( nodeID );
}
