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

#include "vtkMRMLTransformProcessorNode.h"

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
// (further note: Do *not* put spaces in these names -
// will wreak havok when trying to load from XML)
const char* ROLE_INPUT_COMBINE_TRANSFORM = "InputCombineTransform";
const char* ROLE_INPUT_FROM_TRANSFORM = "InputFromTransform";
const char* ROLE_INPUT_TO_TRANSFORM = "InputToTransform";
const char* ROLE_INPUT_INITIAL_TRANSFORM = "InputInitialTransform";
const char* ROLE_INPUT_CHANGED_TRANSFORM = "InputChangedTransform";
const char* ROLE_INPUT_ANCHOR_TRANSFORM = "InputAnchorTransform";
const char* ROLE_INPUT_FORWARD_TRANSFORM = "InputForwardTransform";
const char* ROLE_INPUT_UNSTABILIZED_TRANSFORM = "InputUnstabilizedTransform";
const char* ROLE_OUTPUT_TRANSFORM = "OutputTransform";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro( vtkMRMLTransformProcessorNode );

//----------------------------------------------------------------------------
vtkMRMLTransformProcessorNode::vtkMRMLTransformProcessorNode()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLTransformNode::TransformModifiedEvent );

  this->AddNodeReferenceRole( ROLE_INPUT_COMBINE_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_INPUT_FROM_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_INPUT_TO_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_INPUT_INITIAL_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_INPUT_CHANGED_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_INPUT_ANCHOR_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( ROLE_INPUT_FORWARD_TRANSFORM, NULL, events.GetPointer() );
  this->AddNodeReferenceRole(ROLE_INPUT_UNSTABILIZED_TRANSFORM, NULL, events.GetPointer());
  this->AddNodeReferenceRole( ROLE_OUTPUT_TRANSFORM );

  //Parameters
  this->ProcessingMode = PROCESSING_MODE_QUATERNION_AVERAGE;
  this->UpdateMode = UPDATE_MODE_MANUAL;
  this->CopyTranslationComponents[ 0 ] = true;
  this->CopyTranslationComponents[ 1 ] = true;
  this->CopyTranslationComponents[ 2 ] = true;
  this->RotationMode = ROTATION_MODE_COPY_ALL_AXES;
  this->PrimaryAxisLabel = AXIS_LABEL_Z;
  this->DependentAxesMode = DEPENDENT_AXES_MODE_FROM_PIVOT;
  this->SecondaryAxisLabel = AXIS_LABEL_Y;
  this->StabilizationEnabled = true;
  this->StabilizationCutOffFrequency = 7.5;
}

//----------------------------------------------------------------------------
vtkMRMLTransformProcessorNode::~vtkMRMLTransformProcessorNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::ReadXMLAttributes( const char** atts )
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(updateMode, UpdateMode);
  vtkMRMLReadXMLEnumMacro(processingMode, ProcessingMode);
  vtkMRMLReadXMLEnumMacro(rotationMode, RotationMode);
  vtkMRMLReadXMLEnumMacro(primaryAxisLabel, PrimaryAxisLabel);
  vtkMRMLReadXMLEnumMacro(dependentAxesMode, DependentAxesMode);
  vtkMRMLReadXMLEnumMacro(secondaryAxisLabel, SecondaryAxisLabel);
  vtkMRMLReadXMLBooleanMacro(copyTranslationX, CopyTranslationX);
  vtkMRMLReadXMLBooleanMacro(copyTranslationY, CopyTranslationY);
  vtkMRMLReadXMLBooleanMacro(copyTranslationZ, CopyTranslationZ);
  vtkMRMLReadXMLBooleanMacro(stabilizationEnabled, StabilizationEnabled);
  vtkMRMLReadXMLFloatMacro(stabilizationCutOffFrequency, StabilizationCutOffFrequency);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(updateMode, UpdateMode);
  vtkMRMLWriteXMLEnumMacro(processingMode, ProcessingMode);
  vtkMRMLWriteXMLEnumMacro(rotationMode, RotationMode);
  vtkMRMLWriteXMLEnumMacro(primaryAxisLabel, PrimaryAxisLabel);
  vtkMRMLWriteXMLEnumMacro(dependentAxesMode, DependentAxesMode);
  vtkMRMLWriteXMLEnumMacro(secondaryAxisLabel, SecondaryAxisLabel);
  vtkMRMLWriteXMLBooleanMacro(copyTranslationX, CopyTranslationX);
  vtkMRMLWriteXMLBooleanMacro(copyTranslationY, CopyTranslationY);
  vtkMRMLWriteXMLBooleanMacro(copyTranslationZ, CopyTranslationZ);
  vtkMRMLWriteXMLBooleanMacro(stabilizationEnabled, StabilizationEnabled);
  vtkMRMLWriteXMLFloatMacro(stabilizationCutOffFrequency, StabilizationCutOffFrequency);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf(os, indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(UpdateMode);
  vtkMRMLPrintEnumMacro(ProcessingMode);
  vtkMRMLPrintEnumMacro(RotationMode);
  vtkMRMLPrintEnumMacro(PrimaryAxisLabel);
  vtkMRMLPrintEnumMacro(DependentAxesMode);
  vtkMRMLPrintEnumMacro(SecondaryAxisLabel);
  vtkMRMLPrintBooleanMacro(CopyTranslationX);
  vtkMRMLPrintBooleanMacro(CopyTranslationY);
  vtkMRMLPrintBooleanMacro(CopyTranslationZ);
  vtkMRMLPrintBooleanMacro(StabilizationEnabled);
  vtkMRMLPrintFloatMacro(StabilizationCutOffFrequency);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::Copy(vtkMRMLNode *anode)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(UpdateMode);
  vtkMRMLCopyEnumMacro(ProcessingMode);
  vtkMRMLCopyEnumMacro(RotationMode);
  vtkMRMLCopyEnumMacro(PrimaryAxisLabel);
  vtkMRMLCopyEnumMacro(DependentAxesMode);
  vtkMRMLCopyEnumMacro(SecondaryAxisLabel);
  vtkMRMLCopyBooleanMacro(CopyTranslationX);
  vtkMRMLCopyBooleanMacro(CopyTranslationY);
  vtkMRMLCopyBooleanMacro(CopyTranslationZ);
  vtkMRMLCopyBooleanMacro(StabilizationEnabled);
  vtkMRMLCopyFloatMacro(StabilizationCutOffFrequency);
  vtkMRMLCopyEndMacro();
}

//------------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData )
{
  Superclass::ProcessMRMLEvents( caller, event, callData );

  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
  {
    return;
  }

  // Make sure that the calling node is one of the inputs
  bool callerNodeIsAnInputTransform = ( callerNode == this->GetInputAnchorTransformNode() ||
                                        callerNode == this->GetInputChangedTransformNode() ||
                                        callerNode == this->GetInputInitialTransformNode() ||
                                        callerNode == this->GetInputFromTransformNode() ||
                                        callerNode == this->GetInputToTransformNode() ||
                                        callerNode == this->GetInputForwardTransformNode() ||
                                        callerNode == this->GetInputUnstabilizedTransformNode());
  // Also check the "InputCombine" transforms:
  if ( !callerNodeIsAnInputTransform ) // don't need to check if we already know the caller node is an input transform
  {
    int numberOfInputCombineTransformNodes = GetNumberOfInputCombineTransformNodes();
    for ( int inputCombineTransformIndex = 0; inputCombineTransformIndex < numberOfInputCombineTransformNodes; inputCombineTransformIndex++ )
    {
      if ( callerNode == GetNthInputCombineTransformNode( inputCombineTransformIndex ) )
      {
        callerNodeIsAnInputTransform = true;
        break;
      }
    }
  }

  if ( callerNodeIsAnInputTransform == true )
  {
    if ( event == vtkMRMLTransformNode::TransformModifiedEvent )
    {
      this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
    }
  }
}

//------------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetProcessingMode( int newProcessingMode )
{
  bool validMode = ( newProcessingMode >= 0 && newProcessingMode < PROCESSING_MODE_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new processing mode " << newProcessingMode << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->ProcessingMode == newProcessingMode )
  {
    // no change
    return;
  }
  this->ProcessingMode = newProcessingMode;

  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetUpdateMode( int newUpdateMode )
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
const bool* vtkMRMLTransformProcessorNode::GetCopyTranslationComponents()
{
  return ( const bool* )this->CopyTranslationComponents;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformProcessorNode::GetCopyTranslationX()
{
  return this->CopyTranslationComponents[ 0 ];
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetCopyTranslationX( bool enabled )
{
  if ( this->CopyTranslationComponents[ 0 ] == enabled )
  {
    // no change
    return;
  }
  this->CopyTranslationComponents[ 0 ] = enabled;
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformProcessorNode::GetCopyTranslationY()
{
  return this->CopyTranslationComponents[ 1 ];
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetCopyTranslationY( bool enabled )
{
  if ( this->CopyTranslationComponents[ 1 ] == enabled )
  {
    // no change
    return;
  }
  this->CopyTranslationComponents[ 1 ] = enabled;
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformProcessorNode::GetCopyTranslationZ()
{
  return this->CopyTranslationComponents[ 2 ];
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetCopyTranslationZ( bool enabled )
{
  if ( this->CopyTranslationComponents[ 2 ] == enabled )
  {
    // no change
    return;
  }
  this->CopyTranslationComponents[ 2 ] = enabled;
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetRotationMode( int newRotationMode )
{
  bool validMode = ( newRotationMode >= 0 && newRotationMode < ROTATION_MODE_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new rotation mode " << newRotationMode << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->RotationMode == newRotationMode )
  {
    // no change
    return;
  }
  this->RotationMode = newRotationMode;
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetDependentAxesMode( int newDependentAxesMode )
{
  bool validMode = ( newDependentAxesMode >= 0 && newDependentAxesMode < DEPENDENT_AXES_MODE_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input new dependent axes mode " << newDependentAxesMode << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->DependentAxesMode == newDependentAxesMode )
  {
    // no change
    return;
  }
  this->DependentAxesMode = newDependentAxesMode;
  
  // if there are other modes that need to check for duplicate axes, these should be added below:
  if ( this->DependentAxesMode == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS )
  {
    this->CheckAndCorrectForDuplicateAxes();
  }

  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetPrimaryAxisLabel( int newAxisLabel )
{
  bool validMode = ( newAxisLabel >= 0 && newAxisLabel < AXIS_LABEL_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input primary axis " << newAxisLabel << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->PrimaryAxisLabel == newAxisLabel )
  {
    // no change
    return;
  }
  this->PrimaryAxisLabel = newAxisLabel;
  if ( this->DependentAxesMode == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS )
  {
    this->CheckAndCorrectForDuplicateAxes();
  }
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}


//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetSecondaryAxisLabel( int newAxisLabel )
{
  bool validMode = ( newAxisLabel >= 0 && newAxisLabel < AXIS_LABEL_LAST );
  if ( validMode == false )
  {
    vtkWarningMacro( "Input secondary axis " << newAxisLabel << " is not a valid option. No change will be done." )
    return;
  }

  if ( this->SecondaryAxisLabel == newAxisLabel )
  {
    // no change
    return;
  }
  this->SecondaryAxisLabel = newAxisLabel;
  if ( this->DependentAxesMode == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS )
  {
    this->CheckAndCorrectForDuplicateAxes();
  }
  this->Modified();
  this->InvokeCustomModifiedEvent( InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::CheckAndCorrectForDuplicateAxes()
{
  if (this->PrimaryAxisLabel == this->SecondaryAxisLabel )
  {
    if ( this->PrimaryAxisLabel == vtkMRMLTransformProcessorNode::AXIS_LABEL_Z )
    {
      this->SecondaryAxisLabel = vtkMRMLTransformProcessorNode::AXIS_LABEL_Y;
      vtkWarningMacro( "Duplicate axes for primary and secondary axes. Changing secondary axis to y." );
    }
    else if ( this->PrimaryAxisLabel == vtkMRMLTransformProcessorNode::AXIS_LABEL_Y ||
              this->PrimaryAxisLabel == vtkMRMLTransformProcessorNode::AXIS_LABEL_X )
    {
      this->SecondaryAxisLabel = vtkMRMLTransformProcessorNode::AXIS_LABEL_Z;
      vtkWarningMacro( "Duplicate axes for primary and secondary axes. Changing secondary axis to z." );
    }
  }
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetNthTransformNodeInRole( const char* role, int n )
{
  vtkMRMLNode* node = this->GetNthNodeReference( role, n );
  if ( node == NULL )
  {
    // If necessary a verbose flag should be added. In some cases it is normal for the node to be null (for instance, if it hasn't been set yet)
    //vtkWarningMacro( "Failed to find a node in the role " << role << " at index " << n << ". Returning NULL." );
    return NULL;
  }

  vtkMRMLLinearTransformNode* linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  if ( linearTransformNode == NULL )
  {
    vtkWarningMacro( "Failed to downcast vtkMRMLNode to vtkLinearTransformNode in the role " << role << " at index " << n << ". Returning NULL." );
  }

  return linearTransformNode;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveTransformNodeInRole( const char* role, vtkMRMLLinearTransformNode* node )
{
  if ( node == this->GetTransformNodeInRole( role ) )
  {
    // if the node is the same, then no need to do anything
    return;
  }

  // We want only one transform as input when this function is called.
  // Remove all existing input transforms before setting
  this->RemoveNodeReferenceIDs( role );
  
  const char* nodeID = NULL;
  if ( node )
  {
    nodeID = node->GetID();
  }
  int indexOfNodeID = 0; // This function sets it to the first (and theoretically only) slot
  this->SetAndObserveNthNodeReferenceID( role, indexOfNodeID, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformProcessorNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::AddAndObserveTransformNodeInRole( const char* role, vtkMRMLLinearTransformNode* node )
{
  // adding null does nothing, so just return in this case
  if ( node == NULL )
  {
    return;
  }

  // need to iterate over existing inputs, make sure we are not adding a duplicate
  for ( int n = 0; n < this->GetNumberOfTransformNodesInRole( role ); n++ )
  {
    if ( node == this->GetNthTransformNodeInRole( role, n ) )
    {
      return;
    }
  }

  const char* nodeID = node->GetID();
  this->AddAndObserveNodeReferenceID( role, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformProcessorNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::RemoveNthTransformNodeInRole( const char* role, int n )
{
  this->RemoveNthNodeReferenceID( role, n );
  this->InvokeCustomModifiedEvent( vtkMRMLTransformProcessorNode::InputDataModifiedEvent );
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetNumberOfTransformNodesInRole( const char* role )
{
  return this->GetNumberOfNodeReferences( role );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetTransformNodeInRole( const char* role )
{
  return this->GetNthTransformNodeInRole( role, 0 );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetNthInputCombineTransformNode( int n )
{
  return this->GetNthTransformNodeInRole( ROLE_INPUT_COMBINE_TRANSFORM, n );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::AddAndObserveInputCombineTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->AddAndObserveTransformNodeInRole( ROLE_INPUT_COMBINE_TRANSFORM, node );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::RemoveNthInputCombineTransformNode( int n )
{
  this->RemoveNthTransformNodeInRole( ROLE_INPUT_COMBINE_TRANSFORM, n );
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetNumberOfInputCombineTransformNodes()
{
  return this->GetNumberOfTransformNodesInRole( ROLE_INPUT_COMBINE_TRANSFORM );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputFromTransformNode()
{
  return this->GetTransformNodeInRole( ROLE_INPUT_FROM_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputFromTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_INPUT_FROM_TRANSFORM, node );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputToTransformNode()
{
  return GetTransformNodeInRole( ROLE_INPUT_TO_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputToTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_INPUT_TO_TRANSFORM, node );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputInitialTransformNode()
{
  return GetTransformNodeInRole( ROLE_INPUT_INITIAL_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputInitialTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_INPUT_INITIAL_TRANSFORM, node );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputChangedTransformNode()
{
  return GetTransformNodeInRole( ROLE_INPUT_CHANGED_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputChangedTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_INPUT_CHANGED_TRANSFORM, node );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputAnchorTransformNode()
{
  return GetTransformNodeInRole( ROLE_INPUT_ANCHOR_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputAnchorTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_INPUT_ANCHOR_TRANSFORM, node );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputForwardTransformNode()
{
  return GetTransformNodeInRole( ROLE_INPUT_FORWARD_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputForwardTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_INPUT_FORWARD_TRANSFORM, node );
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetInputUnstabilizedTransformNode()
{
  return this->GetTransformNodeInRole(ROLE_INPUT_UNSTABILIZED_TRANSFORM);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveInputUnstabilizedTransformNode(vtkMRMLLinearTransformNode* node)
{
  this->SetAndObserveTransformNodeInRole(ROLE_INPUT_UNSTABILIZED_TRANSFORM, node);
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransformProcessorNode::GetOutputTransformNode()
{
  return GetTransformNodeInRole( ROLE_OUTPUT_TRANSFORM );
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetAndObserveOutputTransformNode( vtkMRMLLinearTransformNode* node )
{
  this->SetAndObserveTransformNodeInRole( ROLE_OUTPUT_TRANSFORM, node );
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetProcessingModeAsString( int mode )
{
  switch ( mode )
  {
  case PROCESSING_MODE_QUATERNION_AVERAGE:
    return "Quaternion Average";
  case PROCESSING_MODE_COMPUTE_SHAFT_PIVOT:
    return "Compute Shaft Pivot";
  case PROCESSING_MODE_COMPUTE_ROTATION:
    return "Compute Rotation Only";
  case PROCESSING_MODE_COMPUTE_TRANSLATION:
    return "Compute Translation Only";
  case PROCESSING_MODE_COMPUTE_FULL_TRANSFORM:
    return "Compute Full Transform";
  case PROCESSING_MODE_COMPUTE_INVERSE:
    return "Compute Inverse";
  case PROCESSING_MODE_STABILIZE:
    return "Stabilize";
  default:
    vtkGenericWarningMacro("Unknown processing mode provided as input to GetProcessingModeAsString: " << mode << ". Returning \"Unknown Processing Mode\"");
    return "Unknown Processing Mode";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetProcessingModeFromString( std::string name )
{
  for ( int i = 0; i < PROCESSING_MODE_LAST; i++ )
  {
    if ( name == GetProcessingModeAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetUpdateModeAsString( int mode )
{
  switch ( mode )
  {
  case UPDATE_MODE_MANUAL:
    return "Manual Update";
  case UPDATE_MODE_AUTO:
    return "Auto-Update";
  default:
    vtkGenericWarningMacro("Unknown update mode provided as input to GetUpdateModeAsString: " << mode << ". Returning \"Unknown Update Mode\"");
    return "Unknown Update Mode";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetUpdateModeFromString( std::string name )
{
  for ( int i = 0; i < UPDATE_MODE_LAST; i++ )
  {
    if ( name == vtkMRMLTransformProcessorNode::GetUpdateModeAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetRotationModeAsString( int mode )
{
  switch ( mode )
  {
  case ROTATION_MODE_COPY_ALL_AXES:
    return "Copy All Axes";
  case ROTATION_MODE_COPY_SINGLE_AXIS:
    return "Copy Single Axis";
  default:
    vtkGenericWarningMacro("Unknown rotation mode provided as input to GetRotationModeAsString: " << mode << ". Returning \"Unknown Rotation Mode\"");
    return "Unknown Rotation Mode";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetRotationModeFromString( std::string name )
{
  for ( int i = 0; i < ROTATION_MODE_LAST; i++ )
  {
    if ( name == vtkMRMLTransformProcessorNode::GetRotationModeAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
  
//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetDependentAxesModeAsString( int mode )
{
  switch ( mode )
  {
  case DEPENDENT_AXES_MODE_FROM_PIVOT:
    return "From Pivot";
  case DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS:
    return "From Secondary Axis";
  default:
    vtkGenericWarningMacro("Unknown dependent axes mode provided as input to GetRotationDependentAxesModeAsString: " << mode << ". Returning \"Unknown Dependent Axes Mode\"");
    return "Unknown Dependent Axes Mode";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetDependentAxesModeFromString( std::string name )
{
  for ( int i = 0; i < DEPENDENT_AXES_MODE_LAST; i++ )
  {
    if ( name == vtkMRMLTransformProcessorNode::GetDependentAxesModeAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetPrimaryAxisLabelAsString(int label)
{
  return vtkMRMLTransformProcessorNode::GetAxisLabelAsString(label);
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetPrimaryAxisLabelFromString(std::string name)
{
  return vtkMRMLTransformProcessorNode::GetAxisLabelFromString(name);
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetSecondaryAxisLabelAsString(int label)
{
  return vtkMRMLTransformProcessorNode::GetAxisLabelAsString(label);
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetSecondaryAxisLabelFromString(std::string name)
{
  return vtkMRMLTransformProcessorNode::GetAxisLabelFromString(name);
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformProcessorNode::GetAxisLabelAsString( int label )
{
  switch ( label )
  {
  case AXIS_LABEL_X:
    return "X Axis";
  case AXIS_LABEL_Y:
    return "Y Axis";
  case AXIS_LABEL_Z:
    return "Z Axis";
  default:
    vtkGenericWarningMacro("Unknown axis provided as input to GetAxisLabelAsString: " << label << ". Returning \"Unknown Axis\"");
    return "Unknown Axis";
  }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformProcessorNode::GetAxisLabelFromString( std::string name )
{
  for ( int i = 0; i < AXIS_LABEL_LAST; i++ )
  {
    if ( name == vtkMRMLTransformProcessorNode::GetAxisLabelAsString( i ) )
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetStabilizationCutOffFrequency(double cutOffFrequency)
{
  if (this->StabilizationCutOffFrequency == cutOffFrequency)
  {
    // no change
    return;
  }
  this->StabilizationCutOffFrequency = cutOffFrequency;
  this->Modified();
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformProcessorNode::SetStabilizationEnabled(bool enabled)
{
  if (this->StabilizationEnabled == enabled)
  {
    // no change
    return;
  }
  this->StabilizationEnabled = enabled;
  this->Modified();
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}
