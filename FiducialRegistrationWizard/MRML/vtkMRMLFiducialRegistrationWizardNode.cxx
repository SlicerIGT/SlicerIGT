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

// FiducialRegistrationWizard MRML includes
#include "vtkMRMLFiducialRegistrationWizardNode.h"

#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkNew.h"

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
  fiducialListEvents->InsertNextValue( vtkCommand::ModifiedEvent );
  fiducialListEvents->InsertNextValue( vtkMRMLMarkupsNode::PointModifiedEvent ); //PointEndInteractionEvent

  this->AddNodeReferenceRole( PROBE_TRANSFORM_FROM_REFERENCE_ROLE );
  this->AddNodeReferenceRole( PROBE_TRANSFORM_TO_REFERENCE_ROLE );
  this->AddNodeReferenceRole( FROM_FIDUCIAL_LIST_REFERENCE_ROLE, NULL, fiducialListEvents.GetPointer() );
  this->AddNodeReferenceRole( TO_FIDUCIAL_LIST_REFERENCE_ROLE, NULL, fiducialListEvents.GetPointer() );
  this->AddNodeReferenceRole( OUTPUT_TRANSFORM_REFERENCE_ROLE );
  this->RegistrationMode = "Rigid";
  this->UpdateMode = "Automatic";
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
  of << indent << " RegistrationMode=\"" << this->RegistrationMode << "\"";
  of << indent << " UpdateMode=\"" << this->UpdateMode << "\"";
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
    
    if ( ! strcmp( attName, "RegistrationMode" ) )
    {
      this->RegistrationMode = std::string( attValue );
    }
    if ( ! strcmp( attName, "UpdateMode" ) )
    {
      this->UpdateMode = std::string( attValue );
    }
  }

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLFiducialRegistrationWizardNode *node = ( vtkMRMLFiducialRegistrationWizardNode* ) anode;
  
  // Note: It seems that the WriteXML function copies the node then writes the copied node to file
  // So, anything we want in the MRML file we must copy here (I don't think we need to copy other things)
  this->RegistrationMode = node->RegistrationMode;
  this->UpdateMode = node->UpdateMode;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "RegistrationMode: " << this->RegistrationMode << "\n";
  os << indent << "UpdateMode: " << this->UpdateMode << "\n";
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
std::string vtkMRMLFiducialRegistrationWizardNode::GetRegistrationMode()
{
  return this->RegistrationMode;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetRegistrationMode( std::string newRegistrationMode)
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
std::string vtkMRMLFiducialRegistrationWizardNode::GetUpdateMode()
{
  return this->UpdateMode;
}

//------------------------------------------------------------------------------
void vtkMRMLFiducialRegistrationWizardNode::SetUpdateMode( std::string newUpdateMode)
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
void vtkMRMLFiducialRegistrationWizardNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) 
  {
    return;
  }

  if (this->GetFromFiducialListNode()==callerNode)
  {
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
  else if (this->GetToFiducialListNode()==callerNode)
  {
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}
