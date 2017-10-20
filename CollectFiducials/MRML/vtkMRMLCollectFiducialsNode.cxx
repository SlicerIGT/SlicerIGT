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

// CollectFiducials includes
#include "vtkMRMLCollectFiducialsNode.h"

// slicer includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

// vtk includes
#include <vtkNew.h>
#include <vtkPolyData.h>

// std includes
#include <sstream>

// Constants ------------------------------------------------------------------
static const char* PROBE_TRANSFORM_REFERENCE_ROLE = "ProbeTransformNode";
static const char* OUTPUT_REFERENCE_ROLE = "OutputNode";

vtkMRMLNodeNewMacro( vtkMRMLCollectFiducialsNode );

//------------------------------------------------------------------------------
vtkMRMLCollectFiducialsNode::vtkMRMLCollectFiducialsNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkNew<vtkIntArray> transformListEvents;
  transformListEvents->InsertNextValue( vtkCommand::ModifiedEvent );
  transformListEvents->InsertNextValue( vtkMRMLTransformNode::TransformModifiedEvent );

  this->AddNodeReferenceRole( PROBE_TRANSFORM_REFERENCE_ROLE, NULL, transformListEvents.GetPointer() );
  this->AddNodeReferenceRole( OUTPUT_REFERENCE_ROLE );
  this->LabelBase = "P";
  this->LabelCounter = 0;
  this->MinimumDistanceMm = 10.0;
  this->CollectMode = Manual;
}

//------------------------------------------------------------------------------
vtkMRMLCollectFiducialsNode::~vtkMRMLCollectFiducialsNode()
{
}

// ----------------------------------------------------------------------------
void vtkMRMLCollectFiducialsNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML( of, nIndent ); // This will take care of referenced nodes

  vtkIndent indent( nIndent ); 
  of << indent << " LabelBase=\"" << this->LabelBase << "\"";
  of << indent << " LabelCounter=\"" << this->LabelCounter << "\"";
  of << indent << " MinimumDistanceMm=\"" << this->MinimumDistanceMm << "\"";
  of << indent << " CollectMode=\"" << this->CollectMode << "\"";
}

// ----------------------------------------------------------------------------
void vtkMRMLCollectFiducialsNode::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
  os << indent << " LabelBase=\"" << this->LabelBase << "\"";
  os << indent << " LabelCounter=\"" << this->LabelCounter << "\"";
  os << indent << " MinimumDistanceMm=\"" << this->MinimumDistanceMm << "\"";
  os << indent << " CollectMode=\"" << this->CollectMode << "\"";
}

//------------------------------------------------------------------------------
void vtkMRMLCollectFiducialsNode::ReadXMLAttributes( const char** atts )
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
    else if ( ! strcmp( attName, "LabelCounter" ) )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LabelCounter;
      continue;
    }
    else if ( ! strcmp( attName, "MinimumDistanceMm" ) )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MinimumDistanceMm;
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
void vtkMRMLCollectFiducialsNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  this->Modified();
}

//------------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLCollectFiducialsNode::GetProbeTransformNode()
{
  vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( PROBE_TRANSFORM_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLCollectFiducialsNode::SetAndObserveProbeTransformNodeId( const char* nodeId )
{
  const char* currentNodeId = this->GetNodeReferenceID( PROBE_TRANSFORM_REFERENCE_ROLE );
  if ( nodeId != NULL && currentNodeId != NULL && strcmp( nodeId, currentNodeId ) == 0 )
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( PROBE_TRANSFORM_REFERENCE_ROLE, nodeId );
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLCollectFiducialsNode::GetOutputNode()
{
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast( this->GetNodeReference( OUTPUT_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
int vtkMRMLCollectFiducialsNode::GetNumberOfPointsInOutput()
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
void vtkMRMLCollectFiducialsNode::SetOutputNodeId( const char* nodeId )
{
  const char* currentNodeId = this->GetNodeReferenceID( OUTPUT_REFERENCE_ROLE );
  if ( nodeId != NULL && currentNodeId != NULL && strcmp( nodeId, currentNodeId ) == 0 )
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( OUTPUT_REFERENCE_ROLE, nodeId );
}

//------------------------------------------------------------------------------
void vtkMRMLCollectFiducialsNode::ProcessMRMLEvents( vtkObject *caller, unsigned long vtkNotUsed(event), void* vtkNotUsed(callData) )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) 
  {
    return;
  }

  if ( this->GetProbeTransformNode() && callerNode == this->GetProbeTransformNode() )
  {
    this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
  }
}

//------------------------------------------------------------------------------
int vtkMRMLCollectFiducialsNode::GetCollectModeFromString( const char* name )
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
const char* vtkMRMLCollectFiducialsNode::GetCollectModeAsString( int id )
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
