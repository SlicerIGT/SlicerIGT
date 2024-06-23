/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_MAJOR,Slicer_VERSION_MINOR 

// FiducialRegistrationWizard MRML includes
#include "vtkMRMLFiducialRegistrationWizardNode.h"

// slicer includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLI18N.h"
#include "vtkMRMLTransformNode.h"

// vtk includes
#include <vtkNew.h>

// std includes
#include <sstream>

// Constants ------------------------------------------------------------------
static const char* PROBE_TRANSFORM_FROM_REFERENCE_ROLE = "ProbeTransformFrom";
static const char* PROBE_TRANSFORM_TO_REFERENCE_ROLE = "ProbeTransformTo";
static const char* FROM_FIDUCIAL_LIST_REFERENCE_ROLE = "FromFiducialList";
static const char* TO_FIDUCIAL_LIST_REFERENCE_ROLE = "ToFiducialList";
static const char* OUTPUT_TRANSFORM_REFERENCE_ROLE = "OutputTransform";

vtkMRMLNodeNewMacro(vtkMRMLFiducialRegistrationWizardNode);

//------------------------------------------------------------------------------
vtkMRMLFiducialRegistrationWizardNode::vtkMRMLFiducialRegistrationWizardNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkNew<vtkIntArray> fiducialListEvents;
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
  fiducialListEvents->InsertNextValue( vtkMRMLMarkupsNode::PointAddedEvent );
  fiducialListEvents->InsertNextValue( vtkMRMLMarkupsNode::PointRemovedEvent );
#else
  fiducialListEvents->InsertNextValue( vtkMRMLMarkupsNode::MarkupAddedEvent );
  fiducialListEvents->InsertNextValue( vtkMRMLMarkupsNode::MarkupRemovedEvent );
#endif 
  fiducialListEvents->InsertNextValue( vtkMRMLMarkupsNode::PointModifiedEvent );

  this->AddNodeReferenceRole( PROBE_TRANSFORM_FROM_REFERENCE_ROLE );
  this->AddNodeReferenceRole( PROBE_TRANSFORM_TO_REFERENCE_ROLE );
  this->AddNodeReferenceRole( FROM_FIDUCIAL_LIST_REFERENCE_ROLE, NULL, fiducialListEvents.GetPointer() );
  this->AddNodeReferenceRole( TO_FIDUCIAL_LIST_REFERENCE_ROLE, NULL, fiducialListEvents.GetPointer() );
  this->AddNodeReferenceRole( OUTPUT_TRANSFORM_REFERENCE_ROLE );
  this->RegistrationMode = REGISTRATION_MODE_RIGID;
  this->UpdateMode = UPDATE_MODE_AUTOMATIC;
  this->PointMatching = POINT_MATCHING_MANUAL;
  this->WarpingTransformFromParent = true;
  this->CalibrationError = VTK_DOUBLE_MAX;
}

//------------------------------------------------------------------------------
vtkMRMLFiducialRegistrationWizardNode::~vtkMRMLFiducialRegistrationWizardNode()
{
}

// ----------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes

  vtkIndent indent(nIndent); 
  of << indent << " PointMatching=\"" << PointMatchingAsString( this->PointMatching ) << "\"";
  of << indent << " RegistrationMode=\"" << RegistrationModeAsString( this->RegistrationMode ) << "\"";
  of << indent << " UpdateMode=\"" << UpdateModeAsString( this->UpdateMode ) << "\"";
  of << indent << " WarpingTransformFromParent=\"" << (this->WarpingTransformFromParent ? "true" : "false") << "\"";
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "PointMatching" ) )
    {
      this->PointMatching = PointMatchingFromString( std::string( attValue ) );
    }
    else if ( ! strcmp( attName, "RegistrationMode" ) )
    {
      this->RegistrationMode = RegistrationModeFromString( std::string( attValue ) );
    }
    else if ( ! strcmp( attName, "UpdateMode" ) )
    {
      this->UpdateMode = UpdateModeFromString( std::string( attValue ) );
    }
    else if (!strcmp(attName, "WarpingTransformFromParent"))
    {
      this->WarpingTransformFromParent = (strcmp(attValue,"true") ? false : true);
    }
  }

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLFiducialRegistrationWizardNode *node = ( vtkMRMLFiducialRegistrationWizardNode* ) anode;
  
  this->RegistrationMode = node->RegistrationMode;
  this->UpdateMode = node->UpdateMode;
  this->PointMatching = node->PointMatching;
  this->WarpingTransformFromParent = node->WarpingTransformFromParent;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "PointMatching: " << PointMatchingAsString( this->PointMatching ) << "\n";
  os << indent << "RegistrationMode: " << RegistrationModeAsString( this->RegistrationMode ) << "\n";
  os << indent << "UpdateMode: " << UpdateModeAsString( this->UpdateMode ) << "\n";
  os << indent << "WarpingTransformFromParent: " << (this->WarpingTransformFromParent ? "true" : "false") << "\n";
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetAndObserveFromFiducialListNodeId( const char* nodeId )
{
  const char* currentNodeId=this->GetNodeReferenceID(FROM_FIDUCIAL_LIST_REFERENCE_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL && strcmp(nodeId,currentNodeId)==0)
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( FROM_FIDUCIAL_LIST_REFERENCE_ROLE, nodeId);
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLFiducialRegistrationWizardNode::GetFromFiducialListNode()
{
  vtkMRMLMarkupsFiducialNode* node = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( FROM_FIDUCIAL_LIST_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetAndObserveToFiducialListNodeId( const char* nodeId )
{
  const char* currentNodeId=this->GetNodeReferenceID(TO_FIDUCIAL_LIST_REFERENCE_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL && strcmp(nodeId,currentNodeId)==0)
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( TO_FIDUCIAL_LIST_REFERENCE_ROLE, nodeId);
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkMRMLFiducialRegistrationWizardNode::GetToFiducialListNode()
{
  vtkMRMLMarkupsFiducialNode* node = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( TO_FIDUCIAL_LIST_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetProbeTransformFromNodeId( const char* nodeId )
{
  const char* currentNodeId=this->GetNodeReferenceID(PROBE_TRANSFORM_FROM_REFERENCE_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL && strcmp(nodeId,currentNodeId)==0)
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( PROBE_TRANSFORM_FROM_REFERENCE_ROLE, nodeId);
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLFiducialRegistrationWizardNode::GetProbeTransformFromNode()
{
  vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( PROBE_TRANSFORM_FROM_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetProbeTransformToNodeId( const char* nodeId )
{
  const char* currentNodeId=this->GetNodeReferenceID(PROBE_TRANSFORM_TO_REFERENCE_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL && strcmp(nodeId,currentNodeId)==0)
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( PROBE_TRANSFORM_TO_REFERENCE_ROLE, nodeId);
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLFiducialRegistrationWizardNode::GetProbeTransformToNode()
{
  vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( PROBE_TRANSFORM_TO_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetOutputTransformNodeId( const char* nodeId )
{
  const char* currentNodeId=this->GetNodeReferenceID(OUTPUT_TRANSFORM_REFERENCE_ROLE);
  if (nodeId!=NULL && currentNodeId!=NULL && strcmp(nodeId,currentNodeId)==0)
  {
    // not changed
    return;
  }
  this->SetAndObserveNodeReferenceID( OUTPUT_TRANSFORM_REFERENCE_ROLE, nodeId);
  // if the output node is changed then the calibration should be recomputed and placed into the newly selected output node
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLFiducialRegistrationWizardNode::GetOutputTransformNode()
{
  vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast( this->GetNodeReference( OUTPUT_TRANSFORM_REFERENCE_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetRegistrationMode( int newRegistrationMode )
{
  if ( this->GetRegistrationMode() == newRegistrationMode )
  {
    // no change
    return;
  }
  this->RegistrationMode = newRegistrationMode;
  this->Modified();
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
std::string vtkMRMLFiducialRegistrationWizardNode::RegistrationModeAsString( int registrationMode )
{
  switch ( registrationMode )
  {
    case REGISTRATION_MODE_RIGID:
    {
      return "Rigid";
    }
    case REGISTRATION_MODE_SIMILARITY:
    {
      return "Similarity";
    }
    case REGISTRATION_MODE_WARPING:
    {
      return "Warping";
    }
    default:
    {
      vtkGenericWarningMacro( "Unrecognized value for RegistrationMode " << registrationMode << ". Returning \"Unknown\"" );
      return "Unknown";
    }
  }
}

//------------------------------------------------------------------------------
int vtkMRMLFiducialRegistrationWizardNode::RegistrationModeFromString( std::string registrationModeAsString )
{
  for ( int registrationMode = 0; registrationMode < REGISTRATION_MODE_LAST; registrationMode++ )
  {
    bool matchingText = registrationModeAsString.compare( RegistrationModeAsString( registrationMode ) ) == 0;
    if ( matchingText )
    {
      return registrationMode;
    }
  }
  // if there are no matches, then there is likely an error
  vtkGenericWarningMacro( "Unrecognized string for RegistrationMode " << registrationModeAsString << ". Returning value for \"Rigid\"" );
  return REGISTRATION_MODE_RIGID;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetUpdateMode( int newUpdateMode )
{
  if ( this->GetUpdateMode() == newUpdateMode )
  {
    // no change
    return;
  }
  this->UpdateMode = newUpdateMode;
  this->Modified();
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
std::string vtkMRMLFiducialRegistrationWizardNode::UpdateModeAsString( int updateMode )
{
  switch ( updateMode )
  {
    case UPDATE_MODE_MANUAL:
    {
      return "Manual";
    }
    case UPDATE_MODE_AUTOMATIC:
    {
      return "Automatic";
    }
    default:
    {
      vtkGenericWarningMacro( "Unrecognized value for UpdateMode " << updateMode << ". Returning \"Manual\"" );
      return "Manual";
    }
  }
}

//------------------------------------------------------------------------------
int vtkMRMLFiducialRegistrationWizardNode::UpdateModeFromString( std::string updateModeAsString )
{
  for ( int updateMode = 0; updateMode < UPDATE_MODE_LAST; updateMode++ )
  {
    bool matchingText = updateModeAsString.compare( UpdateModeAsString( updateMode ) ) == 0;
    if ( matchingText )
    {
      return updateMode;
    }
  }
  // if there are no matches, then there is likely an error
  vtkGenericWarningMacro( "Unrecognized string for UpdateMode " << updateModeAsString << ". Returning value for \"Manual\"" );
  return UPDATE_MODE_MANUAL;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetPointMatching( int newMethod )
{
  if ( this->GetPointMatching() == newMethod )
  {
    // no change
    return;
  }
  this->PointMatching = newMethod;
  this->Modified();
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
std::string vtkMRMLFiducialRegistrationWizardNode::PointMatchingAsString( int method )
{
  switch ( method )
  {
    case POINT_MATCHING_MANUAL:
    {
      return "Manual";
    }
    case POINT_MATCHING_AUTOMATIC:
    {
      return "Automatic";
    }
    default:
    {
      vtkGenericWarningMacro( "Unrecognized value for PointMatching " << method << ". Returning \"Unknown\"" );
      return "Unknown";
    }
  }
}

//------------------------------------------------------------------------------
std::string vtkMRMLFiducialRegistrationWizardNode::PointMatchingAsDisplayableString(int method)
{
  switch (method)
  {
  case POINT_MATCHING_MANUAL: return vtkMRMLTr("vtkMRMLFiducialRegistrationWizardNode", "Manual");
  case POINT_MATCHING_AUTOMATIC: return vtkMRMLTr("vtkMRMLFiducialRegistrationWizardNode", "Automatic");
  default:
    break;
  }
  return vtkMRMLTr("vtkMRMLFiducialRegistrationWizardNode", "Unknown");
}

//------------------------------------------------------------------------------
int vtkMRMLFiducialRegistrationWizardNode::PointMatchingFromString( std::string methodAsString )
{
  for ( int method = 0; method < POINT_MATCHING_LAST; method++ )
  {
    bool matchingText = methodAsString.compare( PointMatchingAsString( method ) ) == 0;
    if ( matchingText )
    {
      return method;
    }
  }
  // if there are no matches, then there is likely an error
  vtkGenericWarningMacro( "Unrecognized string for PointMatching " << methodAsString << ". Returning value for " << PointMatchingAsString( POINT_MATCHING_MANUAL ) );
  return POINT_MATCHING_MANUAL;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::AddToCalibrationStatusMessage( std::string text )
{
  std::stringstream stream;
  stream << this->CalibrationStatusMessage << text << std::endl; // add std::endl to end to separate messages
  this->CalibrationStatusMessage.assign( stream.str() );
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::ClearCalibrationStatusMessage()
{
  this->CalibrationStatusMessage.assign( "" );
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void* callData )
{
  Superclass::ProcessMRMLEvents( caller, event, callData );

  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );

  if ( callerNode == NULL )
  {
    return;
  }
  else if ( callerNode == this->GetFromFiducialListNode() || 
            callerNode == this->GetToFiducialListNode() )
  {
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR >= 4 && Slicer_VERSION_MINOR >= 11)
    if ( event == vtkMRMLMarkupsNode::PointModifiedEvent ||
         event == vtkMRMLMarkupsNode::PointAddedEvent ||
         event == vtkMRMLMarkupsNode::PointRemovedEvent )
#else
    if ( event == vtkMRMLMarkupsNode::PointModifiedEvent ||
         event == vtkMRMLMarkupsNode::MarkupAddedEvent ||
         event == vtkMRMLMarkupsNode::MarkupRemovedEvent )
#endif
    {
      this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
    }
  }
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetWarpingTransformFromParent(bool warpingTransformFromParent)
{
  if ( this->GetWarpingTransformFromParent() == warpingTransformFromParent )
  {
    // no change
    return;
  }
  this->WarpingTransformFromParent = warpingTransformFromParent;
  this->Modified();
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}
