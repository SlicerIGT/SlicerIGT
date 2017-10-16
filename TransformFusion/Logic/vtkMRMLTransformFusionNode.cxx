/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  
  This file was originally developed by Franklin King, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkMRMLTransformFusionNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLinearTransformNode.h>

#include <sstream>

//----------------------------------------------------------------------------
// constant strings for MRML reference roles
// these should not be used outside of this class
const char* ROLE_INPUT_TRANSFORM     = "Input Transform";
const char* ROLE_RESTING_TRANSFORM   = "Resting Transform";
const char* ROLE_REFERENCE_TRANSFORM = "Reference Transform";
const char* ROLE_OUTPUT_TRANSFORM    = "Output Transform";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro( vtkMRMLTransformFusionNode );

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::vtkMRMLTransformFusionNode()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent );
  this->AddNodeReferenceRole( ROLE_INPUT_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_RESTING_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_REFERENCE_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_OUTPUT_TRANSFORM );

  //Parameters
  this->UpdatesPerSecond = 60;
  this->FusionMode = FUSION_MODE_QUATERNION_AVERAGE;
  this->UpdateMode = UPDATE_MODE_MANUAL;
}

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::~vtkMRMLTransformFusionNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::ReadXMLAttributes( const char** atts )
{
  std::cerr << "Reading TransformFusion parameter node" << std::endl;
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while ( *atts != NULL )
  {
    attName = *( atts++ );
    attValue = *( atts++ );
    
    if ( strcmp( attName, "UpdatesPerSecond" ) == 0 )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UpdatesPerSecond;
      continue;
    }
    else if ( strcmp( attName, "UpdateMode" ) == 0 )
    {
      int modeAsInt = GetUpdateModeFromString( attValue );
      if ( modeAsInt >= 0 && modeAsInt < UPDATE_MODE_LAST)
      {
        this->UpdateMode = modeAsInt;
      }
      else
      {
        vtkWarningMacro("Unrecognized update mode read from MRML node: " << attValue << ". Setting to manual update.")
        this->UpdateMode = UPDATE_MODE_MANUAL;
      }
    }
    else if ( strcmp( attName, "FusionMode" ) == 0 )
    {
      int modeAsInt = GetFusionModeFromString( attValue );
      if ( modeAsInt >= 0 && modeAsInt < FUSION_MODE_LAST)
      {
        this->FusionMode = modeAsInt;
      }
      else
      {
        vtkWarningMacro("Unrecognized fusion mode read from MRML node: " << attValue << ". Setting to quaternion average.")
        this->FusionMode = FUSION_MODE_QUATERNION_AVERAGE;
      }
    }
  }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML( of, nIndent );
  vtkIndent indent( nIndent );
  of << indent << " UpdatesPerSecond=\"" << this->UpdatesPerSecond << "\"";
  of << indent << " UpdateMode=\"" << GetUpdateModeAsString( this->UpdateMode ) << "\"";
  of << indent << " FusionMode=\"" << GetFusionModeAsString( this->FusionMode ) << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
  os << indent << " UpdatesPerSecond = " << this->UpdatesPerSecond << "\n";
  os << indent << " UpdateMode = " << GetUpdateModeAsString( this->UpdateMode ) << "\n";
  os << indent << " FusionMode = " << GetFusionModeAsString( this->FusionMode ) << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLTransformFusionNode *node = vtkMRMLTransformFusionNode::SafeDownCast( anode );
  int wasModifying = node->StartModify();
  
  this->UpdatesPerSecond = node->UpdatesPerSecond;
  this->UpdateMode = node->UpdateMode;
  this->FusionMode = node->FusionMode;

  node->EndModify( wasModifying );
}

//------------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::ProcessMRMLEvents( vtkObject* caller, unsigned long vtkNotUsed(event), void* vtkNotUsed(callData) )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
  {
    return;
  }
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetFusionMode( int newFusionMode )
{
  bool validMode = ( newFusionMode >= 0 && newFusionMode < FUSION_MODE_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new fusion mode " << newFusionMode << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->FusionMode == newFusionMode )
  {
    // no change
    return;
  }
  this->FusionMode = newFusionMode;
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetUpdateMode( int newUpdateMode )
{
  bool validMode = ( newUpdateMode >= 0 && newUpdateMode < UPDATE_MODE_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new update mode " << newUpdateMode << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->UpdateMode == newUpdateMode )
  {
    // no change
    return;
  }
  this->UpdateMode = newUpdateMode;
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetRestingTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( ROLE_RESTING_TRANSFORM ) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveRestingTransformNode(vtkMRMLLinearTransformNode* node)
{
  if ( node == GetRestingTransformNode() )
  {
    // if the node is the same, then no need to do anything
    return;
  }

  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetAndObserveNodeReferenceID( ROLE_RESTING_TRANSFORM, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetReferenceTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( ROLE_REFERENCE_TRANSFORM ) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveReferenceTransformNode( vtkMRMLLinearTransformNode* node )
{
  if ( node == GetReferenceTransformNode() )
  {
    // if the node is the same, then no need to do anything
    return;
  }

  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetAndObserveNodeReferenceID( ROLE_REFERENCE_TRANSFORM, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetOutputTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference(ROLE_OUTPUT_TRANSFORM ) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveOutputTransformNode( vtkMRMLLinearTransformNode* node )
{
  if ( node == GetOutputTransformNode() )
  {
    // if the node is the same, then no need to do anything
    return;
  }

  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetNodeReferenceID( ROLE_OUTPUT_TRANSFORM, nodeID );
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetSingleInputTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNthNodeReference( ROLE_INPUT_TRANSFORM, 0) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveSingleInputTransformNode( vtkMRMLLinearTransformNode* node )
{
  if ( node == GetSingleInputTransformNode() )
  {
    // if the node is the same, then no need to do anything
    return;
  }

  // We want only one transform as input when this function is called.
  // Remove all existing input transforms before setting
  this->RemoveNodeReferenceIDs( ROLE_INPUT_TRANSFORM );
  
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetAndObserveNthNodeReferenceID( ROLE_INPUT_TRANSFORM, 0, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetNthInputTransformNode( int n )
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNthNodeReference( ROLE_INPUT_TRANSFORM, n));
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::AddAndObserveInputTransformNode( vtkMRMLLinearTransformNode* node )
{
  // adding null does nothing, so just return in this case
  if ( node == NULL )
  {
    return;
  }

  // need to iterate over existing inputs, make sure we are not adding a duplicate
  for ( int n = 0; n < GetNumberOfInputTransformNodes(); n++ )
  {
    if ( node == GetNthInputTransformNode( n ) )
    {
      return;
    }
  }

  const char* nodeID = node->GetID();
  this->AddAndObserveNodeReferenceID( ROLE_INPUT_TRANSFORM, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::RemoveInputTransformNode( int n )
{
  this->RemoveNthNodeReferenceID( ROLE_INPUT_TRANSFORM, n );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
int vtkMRMLTransformFusionNode::GetNumberOfInputTransformNodes()
{
  return this->GetNumberOfNodeReferences( ROLE_INPUT_TRANSFORM );
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformFusionNode::GetFusionModeAsString( int mode )
{
  switch ( mode )
  {
  case FUSION_MODE_QUATERNION_AVERAGE:
    return "Quaternion Average";
  case FUSION_MODE_CONSTRAIN_SHAFT_ROTATION:
    return "Constrain Shaft Rotation";
  default:
    vtkGenericWarningMacro("Unknown fusion mode provided as input to GetFusionModeAsString: " << mode << ". Returning \"Unknown Fusion Mode\"");
    return "Unknown Fusion Mode";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformFusionNode::GetFusionModeFromString( std::string name )
{
  for ( int i = 0; i < FUSION_MODE_LAST; i++ )
  {
    if ( name == GetFusionModeAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformFusionNode::GetUpdateModeAsString( int mode )
{
  switch ( mode )
  {
  case UPDATE_MODE_MANUAL:
    return "Manual Update";
  case UPDATE_MODE_AUTO:
    return "Auto-Update";
  case UPDATE_MODE_TIMED:
    return "Timed Update";
  default:
    vtkGenericWarningMacro("Unknown update mode provided as input to GetUpdateModeAsString: " << mode << ". Returning \"Unknown Update Mode\"");
    return "Unknown Update Mode";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformFusionNode::GetUpdateModeFromString( std::string name )
{
  for ( int i = 0; i < UPDATE_MODE_LAST; i++ )
  {
    if ( name == GetUpdateModeAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

