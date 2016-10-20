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
std::string ROLE_INPUT_TRANSFORM     = std::string( "Input Transform" );
std::string ROLE_RESTING_TRANSFORM   = std::string( "Resting Transform" );
std::string ROLE_REFERENCE_TRANSFORM = std::string( "Reference Transform" );
std::string ROLE_OUTPUT_TRANSFORM    = std::string( "Output Transform" );

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro( vtkMRMLTransformFusionNode );

//----------------------------------------------------------------------------
vtkMRMLTransformFusionNode::vtkMRMLTransformFusionNode()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLTransformableNode::TransformModifiedEvent );
  this->AddNodeReferenceRole( ROLE_INPUT_TRANSFORM.c_str(), NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_RESTING_TRANSFORM.c_str(), NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_REFERENCE_TRANSFORM.c_str(), NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_OUTPUT_TRANSFORM.c_str() );

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
      int modeAsInt = GetUpdateModeFromString( attName );
      if ( modeAsInt >= 0 && modeAsInt < UPDATE_MODE_LAST)
      {
        this->UpdateMode = modeAsInt;
      }
      else
      {
        // default
        this->UpdateMode = UPDATE_MODE_MANUAL;
      }
    }
    else if ( strcmp( attName, "FusionMode" ) == 0 )
    {
      int modeAsInt = GetFusionModeFromString( attName );
      if ( modeAsInt >= 0 && modeAsInt < FUSION_MODE_LAST)
      {
        this->FusionMode = modeAsInt;
      }
      else
      {
        // default
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
  this->DisableModifiedEventOn();
  
  this->UpdatesPerSecond = node->UpdatesPerSecond;
  this->UpdateMode = node->UpdateMode;
  this->FusionMode = node->FusionMode;

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//------------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetFusionMode( int newFusionMode )
{
  bool validMode = ( newFusionMode == FUSION_MODE_QUATERNION_AVERAGE ||
                     newFusionMode == FUSION_MODE_CONSTRAIN_SHAFT_ROTATION );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new fusion mode " << newFusionMode << " is not a valid option. Setting to quaternion average." )
    newFusionMode = FUSION_MODE_QUATERNION_AVERAGE;
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
  bool validMode = ( newUpdateMode == UPDATE_MODE_MANUAL ||
                     newUpdateMode == UPDATE_MODE_AUTO ||
                     newUpdateMode == UPDATE_MODE_TIMED );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new update mode " << newUpdateMode << " is not a valid option. Setting to manual update." )
    newUpdateMode = UPDATE_MODE_MANUAL;
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
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( ROLE_RESTING_TRANSFORM.c_str() ) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveRestingTransformNode(vtkMRMLLinearTransformNode* node)
{
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetAndObserveNodeReferenceID( ROLE_RESTING_TRANSFORM.c_str(), nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetReferenceTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( ROLE_REFERENCE_TRANSFORM.c_str() ) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveReferenceTransformNode( vtkMRMLLinearTransformNode* node )
{
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetAndObserveNodeReferenceID( ROLE_REFERENCE_TRANSFORM.c_str(), nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetOutputTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference(ROLE_OUTPUT_TRANSFORM.c_str() ) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveOutputTransformNode( vtkMRMLLinearTransformNode* node )
{
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetNodeReferenceID( ROLE_OUTPUT_TRANSFORM.c_str(), nodeID );
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetSingleInputTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNthNodeReference( ROLE_INPUT_TRANSFORM.c_str(), 0) );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::SetAndObserveSingleInputTransformNode( vtkMRMLLinearTransformNode* node )
{
  // We want only one transform as input when this function is called.
  // Remove all existing input transforms before setting
  this->RemoveNodeReferenceIDs( ROLE_INPUT_TRANSFORM.c_str() );
  
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->SetAndObserveNthNodeReferenceID( ROLE_INPUT_TRANSFORM.c_str(), 0, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformFusionNode::GetNthInputTransformNode( int n )
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNthNodeReference( ROLE_INPUT_TRANSFORM.c_str(), n));
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::AddAndObserveInputTransformNode( vtkMRMLLinearTransformNode* node )
{
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  this->AddAndObserveNodeReferenceID( ROLE_INPUT_TRANSFORM.c_str(), nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformFusionNode::RemoveInputTransformNode( int n )
{
  this->RemoveNthNodeReferenceID( ROLE_INPUT_TRANSFORM.c_str(), n );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
int vtkMRMLTransformFusionNode::GetNumberOfInputTransformNodes()
{
  return this->GetNumberOfNodeReferences( ROLE_INPUT_TRANSFORM.c_str() );
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
    // cannot output warnings in static functions. Just return unknown.
    //vtkWarningMacro("Unknown fusion mode provided as input to GetFusionModeAsString: " << mode << ". Returning \"Unknown Fusion Mode\"");
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
    // cannot output warnings in static functions. Just return unknown.
    //vtkWarningMacro("Unknown update mode provided as input to GetUpdateModeAsString: " << mode << ". Returning \"Unknown Update Mode\"");
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

