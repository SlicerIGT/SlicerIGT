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

// TransformProcessor includes
#include "vtkSlicerTransformProcessorLogic.h"
#include "vtkMRMLTransformProcessorNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkNumberToString.h>
#include <vtkTimerLog.h>

//#include <vtkQuaternionInterpolator.h>

// STD includes
#include <cassert>
#include <sstream>

const float EPSILON = 0.00001;

vtkStandardNewMacro( vtkSlicerTransformProcessorLogic );

//-----------------------------------------------------------------------------
vtkSlicerTransformProcessorLogic::vtkSlicerTransformProcessorLogic()
{
}

//-----------------------------------------------------------------------------
vtkSlicerTransformProcessorLogic::~vtkSlicerTransformProcessorLogic()
{
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if ( scene == NULL )
  {
    vtkErrorMacro( "vtkSlicerTransformProcessorLogic::RegisterNodes failed: invalid scene" );
    return;
  }
  
  scene->RegisterNodeClass( vtkSmartPointer<vtkMRMLTransformProcessorNode>::New() );
}

//---------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLScene::NodeAddedEvent );
  events->InsertNextValue( vtkMRMLScene::NodeRemovedEvent );
  this->SetAndObserveMRMLSceneEventsInternal( newScene, events.GetPointer() );
}

//---------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast( node );
  if ( pNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( pNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLTransformProcessorNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( pNode, events.GetPointer() );
    this->UpdateContinuouslyUpdatedNodesList(pNode);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLTransformProcessorNode* pNode = vtkMRMLTransformProcessorNode::SafeDownCast(node);
  if (pNode)
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( pNode );
    this->UpdateContinuouslyUpdatedNodesList(pNode);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::UpdateContinuouslyUpdatedNodesList(vtkMRMLTransformProcessorNode* paramNode)
{
  if (!paramNode)
  {
    return;
  }
  // Find node in the current list
  std::deque< vtkWeakPointer<vtkMRMLTransformProcessorNode> >::iterator continuouslyUpdatedNodesIt;
  for (continuouslyUpdatedNodesIt = this->ContinuouslyUpdatedNodes.begin();
    continuouslyUpdatedNodesIt != this->ContinuouslyUpdatedNodes.end(); ++continuouslyUpdatedNodesIt)
  {
    if (*continuouslyUpdatedNodesIt == paramNode)
    {
      break;
    }
  }
  // Add/remove node to current list
  if (paramNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_STABILIZE && paramNode->GetStabilizationEnabled())
  {
    // node should be in ContinuouslyUpdatedNodes
    if (continuouslyUpdatedNodesIt == this->ContinuouslyUpdatedNodes.end())
    {
      this->ContinuouslyUpdatedNodes.push_back(paramNode);
    }
  }
  else
  {
    // node should not be in ContinuouslyUpdatedNodes
    // node should be in ContinuouslyUpdatedNodes
    if (continuouslyUpdatedNodesIt != this->ContinuouslyUpdatedNodes.end())
    {
      this->ContinuouslyUpdatedNodes.erase(continuouslyUpdatedNodesIt);
    }
  }
  // Remove deleted nodes
  for (continuouslyUpdatedNodesIt = this->ContinuouslyUpdatedNodes.begin();
    continuouslyUpdatedNodesIt != this->ContinuouslyUpdatedNodes.end(); ++continuouslyUpdatedNodesIt)
  {
    if (continuouslyUpdatedNodesIt->GetPointer() == nullptr)
    {
      continuouslyUpdatedNodesIt = this->ContinuouslyUpdatedNodes.erase(continuouslyUpdatedNodesIt);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* vtkNotUsed(callData) )
{
  vtkMRMLTransformProcessorNode* paramNode = vtkMRMLTransformProcessorNode::SafeDownCast( caller );
  if ( paramNode == NULL )
  {
    return;
  }

  // these are the only two events that should be handled
  if ( event == vtkMRMLTransformProcessorNode::InputDataModifiedEvent )
  {
    if ( paramNode->GetUpdateMode() == vtkMRMLTransformProcessorNode::UPDATE_MODE_AUTO )
    {
      this->UpdateOutputTransform( paramNode );
    }
  }
  else if (event == vtkCommand::ModifiedEvent)
  {
    // This is less frequent than vtkMRMLTransformProcessorNode::InputDataModifiedEvent
    // (which is called at every input transform node change)
    this->UpdateContinuouslyUpdatedNodesList(paramNode);
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::UpdateOutputTransform( vtkMRMLTransformProcessorNode* paramNode )
{
  int mode = paramNode->GetProcessingMode();
  if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_QUATERNION_AVERAGE )
  {
    this->QuaternionAverage( paramNode );
  }
  else if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT )
  {
    this->ComputeShaftPivotTransform( paramNode );
  }
  else if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_ROTATION )
  {
    this->ComputeRotation( paramNode );
  }
  else if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_TRANSLATION )
  {
    this->ComputeTranslation( paramNode );
  }
  else if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM )
  {
    this->ComputeFullTransform( paramNode );
  }
  else if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_INVERSE )
  {
    this->ComputeInverseTransform( paramNode );
  }
  else if (mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_STABILIZE)
  {
    this->ComputeStabilizedTransform(paramNode);
  }
}

//-----------------------------------------------------------------------------
// Please note that the implementation in code below is not fully correct.
// It does *seem* to approximate the average transform reasonably well,
// but it is missing a technically correct implementation such as the
// one found in this technical note:
//   F. Landis Markley, Yang Cheng, John Lucas Crassidis, and Yaakov Oshman. 
//   "Averaging Quaternions", Journal of Guidance, Control, and Dynamics, 
//   Vol. 30, No. 4 (2007), pp. 1193-1197. 
//   http://dx.doi.org/10.2514/1.28949
void vtkSlicerTransformProcessorLogic::QuaternionAverage( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  if ( outputNode == NULL )
  {
    return;
  }

  // Average quaternion
  vtkSmartPointer< vtkMatrix4x4 > resultMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  int numberOfInputs = paramNode->GetNumberOfInputCombineTransformNodes();
  // numberOfInputs is greater than 1, as checked by IsTransformProcessingPossible
  vtkSmartPointer< vtkMatrix4x4 > matrix4x4Pointer = vtkSmartPointer< vtkMatrix4x4 >::New();

  float rotationMatrix[ 3 ][ 3 ] = { { 0 } };
  float averageRotationMatrix[ 3 ][ 3 ] = { { 0 } };
  float singleQuaternion[ 4 ] = { 0 };
  float averageQuaternion[ 4 ] = { 0 };

  for ( int i = 0; i < numberOfInputs; i++ )
  {

    paramNode->GetNthInputCombineTransformNode( i )->GetMatrixTransformToParent( matrix4x4Pointer );

    for ( int row = 0; row < 3; row++ )
    {
      for ( int column = 0; column < 3; column++ )
      { 
        rotationMatrix[ row ][ column ] = matrix4x4Pointer->GetElement( row, column );
      }
    }

    vtkMath::Matrix3x3ToQuaternion( rotationMatrix, singleQuaternion );

    averageQuaternion[ 0 ] = averageQuaternion[ 0 ] + singleQuaternion[ 0 ];
    averageQuaternion[ 1 ] = averageQuaternion[ 1 ] + singleQuaternion[ 1 ];
    averageQuaternion[ 2 ] = averageQuaternion[ 2 ] + singleQuaternion[ 2 ];
    averageQuaternion[ 3 ] = averageQuaternion[ 3 ] + singleQuaternion[ 3 ];
  }

  averageQuaternion[ 0 ] = averageQuaternion[ 0 ] / numberOfInputs;
  averageQuaternion[ 1 ] = averageQuaternion[ 1 ] / numberOfInputs;
  averageQuaternion[ 2 ] = averageQuaternion[ 2 ] / numberOfInputs;
  averageQuaternion[ 3 ] = averageQuaternion[ 3 ] / numberOfInputs;

  float magnitude = sqrt(averageQuaternion[ 0 ] * averageQuaternion[ 0 ] + 
                         averageQuaternion[ 1 ] * averageQuaternion[ 1 ] + 
                         averageQuaternion[ 2 ] * averageQuaternion[ 2 ] + 
                         averageQuaternion[ 3 ] * averageQuaternion[ 3 ]);
  averageQuaternion[ 0 ] = averageQuaternion[ 0 ] / magnitude;
  averageQuaternion[ 1 ] = averageQuaternion[ 1 ] / magnitude;
  averageQuaternion[ 2 ] = averageQuaternion[ 2 ] / magnitude;
  averageQuaternion[ 3 ] = averageQuaternion[ 3 ] / magnitude;

  vtkMath::QuaternionToMatrix3x3( averageQuaternion,averageRotationMatrix );
  
  for ( int row = 0; row < 3; row++ )
  {
    for ( int column = 0; column < 3; column++ )
    { 
      resultMatrix->SetElement( row, column, averageRotationMatrix[ row ][ column ] );
    }
  }

  // Average linear elements
  double value = 0;
  for ( int row = 0; row < 4; row++ )
  {
    for ( int i = 0; i < numberOfInputs; i++ )
    {
      paramNode->GetNthInputCombineTransformNode( i )->GetMatrixTransformToParent( matrix4x4Pointer );
      value += matrix4x4Pointer->GetElement( row, 3 );
    }
    value = value / numberOfInputs;
    resultMatrix->SetElement( row, 3 , value );
    value = 0;
  }

  outputNode->SetMatrixTransformToParent( resultMatrix );
}

//-----------------------------------------------------------------------------
// Re-express the Input transform so that the shaft direction and translation from the primary source are 
// preserved, but the other axes resemble the secondary source coordinate system
void vtkSlicerTransformProcessorLogic::ComputeShaftPivotTransform( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  // The output will be the adjustedToInputAnchorTransform transform.
  // We will compute AdjustedToInputAnchorRotation = InputInitialToInputAnchorRotation * AdjustedToInputInitialRotation
  // Translation must be handled separately as:
  // AdjustedToInputAnchorTranslation = InputChangedToInputInitialTranslation

  // first determine rotation components
  vtkMRMLLinearTransformNode* inputChangedNode = paramNode->GetInputChangedTransformNode();
  vtkMRMLLinearTransformNode* inputInitialNode = paramNode->GetInputInitialTransformNode();
  vtkSmartPointer< vtkGeneralTransform > inputChangedToInputInitialTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLTransformNode::GetTransformBetweenNodes( inputChangedNode, inputInitialNode, inputChangedToInputInitialTransform );
  double shaftDirection[ 3 ] = { 0.0, 0.0, -1.0 }; // conventional shaft direction in SlicerIGT
  vtkSmartPointer< vtkTransform > adjustedToInputInitialRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  this->GetRotationSingleAxisWithPivotFromTransform( inputChangedToInputInitialTransform, shaftDirection, adjustedToInputInitialRotationOnlyTransform );

  vtkMRMLLinearTransformNode* inputAnchorNode = paramNode->GetInputAnchorTransformNode();
  vtkSmartPointer< vtkGeneralTransform > inputInitialToInputAnchorTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLTransformNode::GetTransformBetweenNodes( inputInitialNode, inputAnchorNode, inputInitialToInputAnchorTransform );
  vtkSmartPointer< vtkTransform > inputInitialToInputAnchorRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  this->GetRotationAllAxesFromTransform( inputInitialToInputAnchorTransform, inputInitialToInputAnchorRotationOnlyTransform );
  
  // Translation is same as input translation, since they share the same origin
  vtkSmartPointer< vtkGeneralTransform > inputChangedToInputAnchorTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLTransformNode::GetTransformBetweenNodes( inputChangedNode, inputAnchorNode, inputChangedToInputAnchorTransform );
  vtkSmartPointer< vtkTransform > inputChangedToInputAnchorTranslationTransform = vtkSmartPointer< vtkTransform >::New();
  bool copyComponents[ 3 ] = { 1, 1, 1 }; // copy x, y, and z
  this->GetTranslationOnlyFromTransform( inputChangedToInputAnchorTransform, copyComponents, inputChangedToInputAnchorTranslationTransform );

  // put it all together
  vtkSmartPointer< vtkTransform > adjustedToInputAnchorTransform = vtkSmartPointer< vtkTransform >::New();
  adjustedToInputAnchorTransform->PreMultiply();
  adjustedToInputAnchorTransform->Identity();
  adjustedToInputAnchorTransform->Concatenate( inputChangedToInputAnchorTranslationTransform );
  adjustedToInputAnchorTransform->Concatenate( inputInitialToInputAnchorRotationOnlyTransform );
  adjustedToInputAnchorTransform->Concatenate( adjustedToInputInitialRotationOnlyTransform );
  adjustedToInputAnchorTransform->Update();

  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  // the existence of outputNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputNode->SetMatrixTransformToParent( adjustedToInputAnchorTransform->GetMatrix() );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeRotation( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  // first get the parameters from the parameter node
  int rotationMode = paramNode->GetRotationMode();
  int dependentAxesMode = paramNode->GetDependentAxesMode();

  double primaryAxis[ 3 ] = { 0.0, 0.0, 0.0 };
  int primaryAxisLabel = paramNode->GetPrimaryAxisLabel();
  switch ( primaryAxisLabel )
  {
    case vtkMRMLTransformProcessorNode::AXIS_LABEL_X:
      primaryAxis[ 0 ] = 1.0;
      break;
    case vtkMRMLTransformProcessorNode::AXIS_LABEL_Y:
      primaryAxis[ 1 ] = 1.0;
      break;
    case vtkMRMLTransformProcessorNode::AXIS_LABEL_Z:
      primaryAxis[ 2 ] = 1.0;
      break;
    default:
      vtkWarningMacro( "CopyRotation: Unrecognized primary axis " << primaryAxisLabel << ". Returning, no operation performed." );
      return;
  }

  double secondaryAxis[ 3 ] = { 0.0, 0.0, 0.0 };
  int secondaryAxisLabel = paramNode->GetSecondaryAxisLabel();
  switch ( secondaryAxisLabel )
  {
    case vtkMRMLTransformProcessorNode::AXIS_LABEL_X:
      secondaryAxis[ 0 ] = 1.0;
      break;
    case vtkMRMLTransformProcessorNode::AXIS_LABEL_Y:
      secondaryAxis[ 1 ] = 1.0;
      break;
    case vtkMRMLTransformProcessorNode::AXIS_LABEL_Z:
      secondaryAxis[ 2 ] = 1.0;
      break;
    default:
      vtkWarningMacro( "CopyRotation: Unrecognized primary axis " << secondaryAxisLabel << ". Returning, no operation performed." );
      return;
  }

  vtkSmartPointer< vtkGeneralTransform > fromToToGeneralTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLLinearTransformNode* fromTransformNode = paramNode->GetInputFromTransformNode();
  vtkMRMLLinearTransformNode* toTransformNode = paramNode->GetInputToTransformNode();
  vtkMRMLTransformNode::GetTransformBetweenNodes( fromTransformNode, toTransformNode, fromToToGeneralTransform );

  // if there are other modes that need to check and corrrect for duplicate axes, these should be added below:
  if ( paramNode->GetDependentAxesMode() == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS )
  {
    paramNode->CheckAndCorrectForDuplicateAxes();
  }

  // computation
  vtkSmartPointer< vtkTransform > fromToToRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  vtkSlicerTransformProcessorLogic::GetRotationOnlyFromTransform( fromToToGeneralTransform, rotationMode, dependentAxesMode, primaryAxis, secondaryAxis, fromToToRotationOnlyTransform );
  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  // the existence of outputNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputNode->SetMatrixTransformToParent( fromToToRotationOnlyTransform->GetMatrix() );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeTranslation( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  // get parameters from parameter node
  const bool* copyComponents = paramNode->GetCopyTranslationComponents();
  vtkSmartPointer< vtkGeneralTransform > fromToToGeneralTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLLinearTransformNode* fromTransformNode = paramNode->GetInputFromTransformNode();
  vtkMRMLLinearTransformNode* toTransformNode = paramNode->GetInputToTransformNode();
  vtkMRMLTransformNode::GetTransformBetweenNodes( fromTransformNode, toTransformNode, fromToToGeneralTransform );
  vtkSmartPointer< vtkTransform > fromToToTranslationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  this->GetTranslationOnlyFromTransform( fromToToGeneralTransform, copyComponents, fromToToTranslationOnlyTransform );
  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  // the existence of outputNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputNode->SetMatrixTransformToParent( fromToToTranslationOnlyTransform->GetMatrix() );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeFullTransform( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  vtkSmartPointer< vtkGeneralTransform > fromToToGeneralTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLLinearTransformNode* fromTransformNode = paramNode->GetInputFromTransformNode();
  vtkMRMLLinearTransformNode* toTransformNode = paramNode->GetInputToTransformNode();
  vtkMRMLTransformNode::GetTransformBetweenNodes( fromTransformNode, toTransformNode, fromToToGeneralTransform );

  // need to convert the general transform to a matrix. Decompose then concatenate the rotation and translation
  vtkSmartPointer< vtkTransform > fromToToRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  this->GetRotationAllAxesFromTransform( fromToToGeneralTransform, fromToToRotationOnlyTransform );
  bool copyComponents[ 3 ] = { 1, 1, 1 }; // copy x, y, and z
  vtkSmartPointer< vtkTransform > fromToToTranslationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  this->GetTranslationOnlyFromTransform( fromToToGeneralTransform, copyComponents, fromToToTranslationOnlyTransform );
  
  vtkSmartPointer< vtkTransform > fromToToLinearTransformNode = vtkSmartPointer< vtkTransform >::New();
  fromToToLinearTransformNode->PreMultiply();
  fromToToLinearTransformNode->Identity();
  fromToToLinearTransformNode->Concatenate( fromToToTranslationOnlyTransform );
  fromToToLinearTransformNode->Concatenate( fromToToRotationOnlyTransform );
  fromToToLinearTransformNode->Update();

  vtkMRMLLinearTransformNode* outputTransformNode = paramNode->GetOutputTransformNode();
  // the existence of outputTransformNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputTransformNode->SetAndObserveTransformToParent( fromToToLinearTransformNode );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeInverseTransform( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  vtkMRMLLinearTransformNode* forwardTransformNode = paramNode->GetInputForwardTransformNode();
  // node stores the transform _to_ parent. Inverse will be the transform _from_ parent.
  vtkSmartPointer< vtkMatrix4x4 > matrixTransformFromParent = vtkSmartPointer< vtkMatrix4x4 >::New(); 
  forwardTransformNode->GetMatrixTransformFromParent( matrixTransformFromParent );
  vtkMRMLLinearTransformNode* outputTransformNode = paramNode->GetOutputTransformNode();
  // the existence of outputTransformNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputTransformNode->SetMatrixTransformToParent( matrixTransformFromParent );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::GetRotationOnlyFromTransform( vtkGeneralTransform* sourceToTargetTransform, int rotationMode, int dependentAxesMode, const double* primaryAxis, const double* secondaryAxis, vtkTransform* rotationOnlyTransform )
{
  if ( rotationOnlyTransform == NULL )
  {
    vtkErrorMacro( "GetRotationOnlyFromTransform: rotationOnlyTransform is null. Returning, but transform will remain null." );
    return;
  }

  if ( sourceToTargetTransform == NULL )
  {
    vtkErrorMacro( "GetRotationOnlyFromTransform: inputFullTransform is null. Returning, but no operation performed." );
    return;
  }

  switch ( rotationMode )
  {
    case vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_ALL_AXES:
      vtkSlicerTransformProcessorLogic::GetRotationAllAxesFromTransform( sourceToTargetTransform, rotationOnlyTransform );
      break;
    case vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_SINGLE_AXIS:
      vtkSlicerTransformProcessorLogic::GetRotationSingleAxisFromTransform( sourceToTargetTransform, dependentAxesMode, primaryAxis, secondaryAxis, rotationOnlyTransform );
      break;
    default:
      vtkErrorMacro( "GetRotationOnlyFromTransform: rotationMode " << rotationMode << " is unrecognized. Returning, but no operation performed." );
      break;
  }
}

//----------------------------------------------------------------------------
// Get the orientation transform from one transform to the other.
// In other words, return the 3x3 matrix that is used to
// rotate from one basis to another. Translation is not used here.
void vtkSlicerTransformProcessorLogic::GetRotationAllAxesFromTransform ( vtkGeneralTransform* sourceToTargetTransform, vtkTransform* rotationOnlyTransform )
{
  if ( rotationOnlyTransform == NULL )
  {
    vtkGenericWarningMacro( "rotationOnlyTransform is null" );
    return;
  }

  if ( sourceToTargetTransform == NULL )
  {
    vtkGenericWarningMacro( "sourceToTargetTransform is NULL" );
    return;
  }

  // Unfortunately, the vtkGeneralTransform class does not provide an easy way to copy the full transform
  // and leave the translation component at zero. So we just have to do some manual vtk math here to find the axes
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };
  double xAxisVector[ 3 ] = { 1.0, 0.0, 0.0 };
  double xAxisInput[ 3 ];
  sourceToTargetTransform->TransformVectorAtPoint( zeroVector3, xAxisVector, xAxisInput );
  double yAxisVector[ 3 ] = { 0.0, 1.0, 0.0 };
  double yAxisInput[ 3 ];
  sourceToTargetTransform->TransformVectorAtPoint( zeroVector3, yAxisVector, yAxisInput );
  double zAxisVector[ 3 ] = { 0.0, 0.0, 1.0 };
  double zAxisInput[ 3 ];
  sourceToTargetTransform->TransformVectorAtPoint( zeroVector3, zAxisVector, zAxisInput );
  
  // set the matrix accordingly
  vtkSmartPointer< vtkMatrix4x4 > rotationOnlyMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  vtkSlicerTransformProcessorLogic::GetRotationMatrixFromAxes( xAxisInput, yAxisInput, zAxisInput, rotationOnlyMatrix );
  rotationOnlyTransform->SetMatrix( rotationOnlyMatrix );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::GetRotationSingleAxisFromTransform( vtkGeneralTransform* sourceToTargetTransform, int dependentAxesMode, const double* primaryAxis, const double* secondaryAxis, vtkTransform* rotationOnlyTransform )
{
  if ( rotationOnlyTransform == NULL )
  {
    vtkErrorMacro( "GetRotationSingleAxisFromTransform: rotationOnlyTransform is null. Returning, but transform will remain null." );
    return;
  }

  if ( sourceToTargetTransform == NULL )
  {
    vtkErrorMacro( "GetRotationSingleAxisFromTransform: inputFullTransform is null. Returning, but no operation performed." );
    return;
  }

  switch ( dependentAxesMode )
  {
    case vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_PIVOT:
      this->GetRotationSingleAxisWithPivotFromTransform( sourceToTargetTransform, primaryAxis, rotationOnlyTransform );
      break;
    case vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS:
      this->GetRotationSingleAxisWithSecondaryFromTransform( sourceToTargetTransform, primaryAxis, secondaryAxis, rotationOnlyTransform );
      break;
    default:
      vtkErrorMacro( "GetRotationSingleAxisFromTransform: dependentAxesMode " << dependentAxesMode << " is unrecognized. Returning, but no operation performed." );
      break;
  }
}

//----------------------------------------------------------------------------
// Get the orientation transform *such that* the primary axis
// the other axes are described using the smallest pivot rotation
// from the source
void vtkSlicerTransformProcessorLogic::GetRotationSingleAxisWithPivotFromTransform( vtkGeneralTransform* sourceToTargetTransform, const double* primaryAxis, vtkTransform* rotationOnlyTransform )
{
  if ( sourceToTargetTransform == NULL )
  {
    vtkGenericWarningMacro( "sourceToTargetTransform is NULL" );
    return;
  }

  if ( primaryAxis == NULL )
  {
    vtkGenericWarningMacro( "primaryAxis is NULL" );
    return;
  }

  if ( rotationOnlyTransform == NULL )
  {
    vtkGenericWarningMacro( "rotationOnlyTransform is NULL" );
    return;
  }

  // Key point: We REFER to the Target transform, then
  // rotate between it and the Source. We use an axis-angle rotation,
  // but make sure that the rotation axis is perpendicular to primary axis.
  // This eliminates any rotation about the axis itself, so the other
  // two axes are aligned as closely as possible.

  // first determine the rotated primary axis
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };
  double primaryAxisRotated[ 3 ];
  sourceToTargetTransform->TransformVectorAtPoint( zeroVector3, primaryAxis, primaryAxisRotated );

  // compute ROTATION axis and angle between primary axes (source vs target)
  double rotationAxisSourceToTarget[ 3 ];
  // cross product will be perpendicular to the inputs
  vtkMath::Cross( primaryAxis, primaryAxisRotated, rotationAxisSourceToTarget );
  double rotationDegreesSourceToTarget = asin( vtkMath::Norm( rotationAxisSourceToTarget) ) * 180.0 / vtkMath::Pi();
  // the angle could be much higher in magnitude than indicated by the cross product -
  // a dot product should be done on the axes. If negative, then the angle is higher
  // in magnitude than 90 degrees (the max value reportable by asin) and therefore
  // shoud be corrected
  bool rotationMagnitudeGreaterThan90 = ( vtkMath::Dot( primaryAxis, primaryAxisRotated ) < 0.0 );
  if ( rotationMagnitudeGreaterThan90 )
  {
    if ( rotationDegreesSourceToTarget < 0 )
    {
      rotationDegreesSourceToTarget = -180.0 - rotationDegreesSourceToTarget;
    }
    else
    {
      rotationDegreesSourceToTarget = 180.0 - rotationDegreesSourceToTarget;
    }
  }

  vtkMath::Normalize( rotationAxisSourceToTarget );
  if ( vtkMath::Norm( rotationAxisSourceToTarget ) <= EPSILON )
  {
    // if the axis is zero, then there is no rotation and any arbitrary axis is fine.
    rotationAxisSourceToTarget[ 0 ] = 1.0;
    rotationAxisSourceToTarget[ 1 ] = 0.0;
    rotationAxisSourceToTarget[ 2 ] = 0.0;
    rotationDegreesSourceToTarget = 0.0;
  }

  rotationOnlyTransform->Identity();
  rotationOnlyTransform->RotateWXYZ( rotationDegreesSourceToTarget, rotationAxisSourceToTarget );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::GetRotationSingleAxisWithSecondaryFromTransform( vtkGeneralTransform* sourceToTargetTransform, const double* primaryAxis, const double* secondaryAxis, vtkTransform* rotationOnlyTransform )
{
  if ( sourceToTargetTransform == NULL )
  {
    vtkGenericWarningMacro( "sourceToTargetTransform is NULL" );
    return;
  }

  if ( primaryAxis == NULL )
  {
    vtkGenericWarningMacro( "primaryAxis is NULL" );
    return;
  }

  if ( secondaryAxis == NULL )
  {
    vtkGenericWarningMacro( "secondaryAxis is NULL" );
    return;
  }

  if ( rotationOnlyTransform == NULL )
  {
    vtkGenericWarningMacro( "rotationOnlyTransform is NULL" );
    return;
  }

  // Rotate from the sourceToTargetTransform, such that the primary axis 
  // remains the same, but the secondary axis is as close as possible to the target.

  // first determine the rotated axes
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };

  const double* primarySourceAxisInSource = primaryAxis;
  
  double primarySourceAxisInTarget[ 3 ];
  sourceToTargetTransform->TransformVectorAtPoint( zeroVector3, primaryAxis, primarySourceAxisInTarget );

  const double* secondaryTargetAxisInTarget = secondaryAxis;

  // cross product will be perpendicular to the inputs
  double tertiaryResultAxisInTarget[ 3 ];
  vtkMath::Cross( primarySourceAxisInTarget, secondaryTargetAxisInTarget, tertiaryResultAxisInTarget );
  double tertiaryAxisLength = vtkMath::Norm( tertiaryResultAxisInTarget );
  if ( tertiaryAxisLength < EPSILON )
  {
    // In this case, any arbitrary vector will have to do.
    vtkMath::Perpendiculars( primarySourceAxisInTarget, tertiaryResultAxisInTarget, NULL, 0.0 );
  }
  vtkMath::Normalize( tertiaryResultAxisInTarget );

  double secondaryResultAxisInTarget[ 3 ];
  vtkMath::Cross( tertiaryResultAxisInTarget, primarySourceAxisInTarget, secondaryResultAxisInTarget );
  vtkMath::Normalize( secondaryResultAxisInTarget );

  vtkSmartPointer< vtkGeneralTransform > targetToSourceTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  targetToSourceTransform->DeepCopy( sourceToTargetTransform );
  targetToSourceTransform->Inverse();
  double secondaryResultAxisInSource[ 3 ];
  targetToSourceTransform->TransformVectorAtPoint( zeroVector3, secondaryResultAxisInTarget, secondaryResultAxisInSource );
  
  const double* secondarySourceAxisInSource = secondaryAxis;

  // compute the angle and axis
  double rotationAxisTargetToResult[ 3 ];
  vtkMath::Cross( secondarySourceAxisInSource, secondaryResultAxisInSource, rotationAxisTargetToResult );
  double rotationDegreesTargetToResult = asin( vtkMath::Norm( rotationAxisTargetToResult ) ) * 180.0 / vtkMath::Pi();
  vtkMath::Normalize( rotationAxisTargetToResult );
  // If the secondary axes are almost parallel, we have some numerical instability.
  // The rotation should be around the primary axis for sure, the only question
  // is which direction (forward or back). Do a dot product to choose the direction,
  // and copy into rotationAxis. This will correct the numerical instability.
  double dotProductRotationAxis = vtkMath::Dot( rotationAxisTargetToResult, primarySourceAxisInSource );
  if ( dotProductRotationAxis > 0 )
  {
    rotationAxisTargetToResult[ 0 ] = primarySourceAxisInSource[ 0 ];
    rotationAxisTargetToResult[ 1 ] = primarySourceAxisInSource[ 1 ];
    rotationAxisTargetToResult[ 2 ] = primarySourceAxisInSource[ 2 ];
  }
  else
  {
    // opposite direction
    rotationAxisTargetToResult[ 0 ] = - primarySourceAxisInSource[ 0 ];
    rotationAxisTargetToResult[ 1 ] = - primarySourceAxisInSource[ 1 ];
    rotationAxisTargetToResult[ 2 ] = - primarySourceAxisInSource[ 2 ];
  }

  // when the rotation is greater than 90 degrees, the value reported by asin
  // goes down instead of up (ie, 91 becomes 89, 92 becomes 88).
  // To correct this, check the dot product between the axes and correct if negative
  double dotProductSecondAxes = vtkMath::Dot( secondaryResultAxisInSource, secondarySourceAxisInSource );
  bool isRotationDegreesGreaterThan90 = ( dotProductSecondAxes < 0 );
  if ( isRotationDegreesGreaterThan90 )
  {
    rotationDegreesTargetToResult = 180 - rotationDegreesTargetToResult;
  }

  vtkSmartPointer< vtkTransform > sourceToTargetRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  vtkSlicerTransformProcessorLogic::GetRotationAllAxesFromTransform( sourceToTargetTransform, sourceToTargetRotationOnlyTransform );
  
  rotationOnlyTransform->Identity();
  rotationOnlyTransform->PreMultiply();
  rotationOnlyTransform->Concatenate( sourceToTargetRotationOnlyTransform );
  rotationOnlyTransform->RotateWXYZ( rotationDegreesTargetToResult, rotationAxisTargetToResult );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::GetTranslationOnlyFromTransform( vtkGeneralTransform* sourceToTargetTransform, const bool* copyComponents, vtkTransform* translationOnlyTransform )
{
  if ( translationOnlyTransform == NULL )
  {
    vtkGenericWarningMacro( "translationOnlyTransform is NULL" );
    return;
  }

  if ( sourceToTargetTransform == NULL )
  {
    vtkGenericWarningMacro( "sourceToTargetTransform is NULL" );
    return;
  }

  if ( copyComponents == NULL )
  {
    vtkGenericWarningMacro( "copyComponents is NULL" );
    return;
  }

  double sourceToTargetTranslation[ 3 ] = { 0.0, 0.0, 0.0 };
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };
  sourceToTargetTransform->TransformPoint( zeroVector3, sourceToTargetTranslation );
  
  for ( int dimension = 0; dimension < 3; dimension++ )
  {
    if ( copyComponents[ dimension ] == false )
    {
      sourceToTargetTranslation[ dimension ] = 0.0;
    }
  }
  
  // copy to the output
  translationOnlyTransform->Identity();
  translationOnlyTransform->Translate( sourceToTargetTranslation );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::GetRotationMatrixFromAxes( const double* xAxis, const double* yAxis, const double* zAxis, vtkMatrix4x4* rotationMatrix )
{
  if ( xAxis == NULL )
  {
    vtkGenericWarningMacro( "xAxis is NULL" );
    return;
  }
  
  if ( yAxis == NULL )
  {
    vtkGenericWarningMacro( "yAxis is NULL" );
    return;
  }

  if ( zAxis == NULL )
  {
    vtkGenericWarningMacro( "zAxis is NULL" );
    return;
  }

  if ( rotationMatrix == NULL )
  {
    vtkGenericWarningMacro( "rotationMatrix is NULL" );
    return;
  }

  rotationMatrix->Identity();
  rotationMatrix->SetElement( 0, 0, xAxis[ 0 ] );
  rotationMatrix->SetElement( 1, 0, xAxis[ 1 ] );
  rotationMatrix->SetElement( 2, 0, xAxis[ 2 ] );
  rotationMatrix->SetElement( 0, 1, yAxis[ 0 ] );
  rotationMatrix->SetElement( 1, 1, yAxis[ 1 ] );
  rotationMatrix->SetElement( 2, 1, yAxis[ 2 ] );
  rotationMatrix->SetElement( 0, 2, zAxis[ 0 ] );
  rotationMatrix->SetElement( 1, 2, zAxis[ 1 ] );
  rotationMatrix->SetElement( 2, 2, zAxis[ 2 ] );
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformProcessorLogic::IsTransformProcessingPossible( vtkMRMLTransformProcessorNode *node, bool verbose )
{
  if ( node == NULL )
  {
    vtkErrorMacro( "IsTransformProcessingPossible: Input parameter node is null. Returning false." );
    return false;
  }

  // if verbose, output why the conditions fail
  bool result = true; // assume the conditions are met until otherwise shown
  
  // first check for valid mode
  int mode = node->GetProcessingMode();
  if ( mode < 0 || mode >= vtkMRMLTransformProcessorNode::PROCESSING_MODE_LAST )
  {
    // output regardless of verbose, since this is an error that should not happen at all.
    // output the processing mode number, since the "Unknown" string is not going to be useful
    vtkErrorMacro( "IsTransformProcessingPossible: Unrecognized processing mode provided as input: " << mode << ". Returning false." );
    return false;
  }

  // Check for individual modes whether the requisite nodes have been provided
  // Not all modes require non-NULL transform nodes. NULL is treated like RAS
  // If many more modes are added, then this scheme should be re-evaluated
  
  if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_INVERSE )
  {
    if ( node->GetInputForwardTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: No \"Forward\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
  }

  if (mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_STABILIZE)
  {
    if (node->GetInputUnstabilizedTransformNode() == NULL)
    {
      if (verbose)
      {
        vtkWarningMacro("IsTransformProcessingPossible: No input transform node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString(mode));
      }
      result = false;
    }
  }

  if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_QUATERNION_AVERAGE )
  {
    if ( node->GetNumberOfInputCombineTransformNodes() < 1 )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: Not enough source nodes as input for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
  }
  
  // All modes so far need an output transform node
  if ( node->GetOutputTransformNode() == NULL )
  {
    // this is an error for *any* mode, no need to check mode
    if ( verbose )
    {
      vtkWarningMacro( "IsTransformProcessingPossible: No \"Output\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
    }
    result = false;
  }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeStabilizedTransform(vtkMRMLTransformProcessorNode* paramNode)
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = this->IsTransformProcessingPossible(paramNode, verboseWarnings);
  if (conditionsMetForProcessing == false)
  {
    return;
  }

  vtkMRMLLinearTransformNode* inputNode = paramNode->GetInputUnstabilizedTransformNode();
  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  if (inputNode == NULL || outputNode == NULL)
  {
    return;
  }

  // Get timestamps
  double currentTimeSec = vtkTimerLog::GetUniversalTime();
  double lastUpdateTimeSec = 0.0;
  const char* lastUpdateTimeSecStr = outputNode->GetAttribute("TransformProcessor.LastUpdateTimeSec");
  if (lastUpdateTimeSecStr)
  {
    std::istringstream convertedStream(lastUpdateTimeSecStr);
    convertedStream >> lastUpdateTimeSec;
  }
  std::stringstream ss;
  ss.precision(4);
  ss << std::fixed << currentTimeSec;
  outputNode->SetAttribute("TransformProcessor.LastUpdateTimeSec", ss.str().c_str());

  // Get matrices
  vtkNew<vtkMatrix4x4> matrixCurrent;
  inputNode->GetMatrixTransformToParent(matrixCurrent);
  
  if (lastUpdateTimeSec <= 0.0 || !paramNode->GetStabilizationEnabled())
  {
    // No filter enabled or no history of previous values: Output Transform = Input Transform
    outputNode->SetMatrixTransformToParent(matrixCurrent);
  }
  else
  {
    // Compute weights (low-pass filter with w_cutoff frequency)
    const double elapsedTimeSec = currentTimeSec - lastUpdateTimeSec;
    const double cutoff_frequency = paramNode->GetStabilizationCutOffFrequency();
    const double weightPrevious = 1;
    const double weightCurrent = elapsedTimeSec * cutoff_frequency;

    vtkNew<vtkMatrix4x4> matrixPrevious;
    outputNode->GetMatrixTransformToParent(matrixPrevious);

    vtkNew<vtkMatrix4x4> matrixOutput;
    this->GetInterpolatedTransform(matrixPrevious, matrixCurrent, weightPrevious, weightCurrent, matrixOutput);
    outputNode->SetMatrixTransformToParent(matrixOutput);
  }
}

//----------------------------------------------------------------------------
// Spherical linear interpolation between two rotation quaternions.
// t is a value between 0 and 1 that interpolates between from and to (t=0 means the results is the same as "from").
// Precondition: no aliasing problems to worry about ("result" can be "from" or "to" param).
// Parameters: adjustSign - If true, then slerp will operate by adjusting the sign of the slerp to take shortest path. True is recommended, otherwise the interpolation sometimes give unexpected results. 
// References: From Adv Anim and Rendering Tech. Pg 364
void vtkSlicerTransformProcessorLogic::Slerp(double* result, double t, double* from, double* to, bool adjustSign)
{
  const double* p = from; // just an alias to match q

  // calc cosine theta
  double cosom = from[0] * to[0] + from[1] * to[1] + from[2] * to[2] + from[3] * to[3]; // dot( from, to )

  // adjust signs (if necessary)
  double q[4];
  if (adjustSign && (cosom < (double)0.0))
  {
    cosom = -cosom;
    q[0] = -to[0];   // Reverse all signs
    q[1] = -to[1];
    q[2] = -to[2];
    q[3] = -to[3];
  }
  else
  {
    q[0] = to[0];
    q[1] = to[1];
    q[2] = to[2];
    q[3] = to[3];
  }

  // Calculate coefficients
  double sclp, sclq;
  if (((double)1.0 - cosom) > (double)0.0001) // 0.0001 -> some epsillon
  {
    // Standard case (slerp)
    double omega, sinom;
    omega = acos(cosom); // extract theta from dot product's cos theta
    sinom = sin(omega);
    sclp = sin(((double)1.0 - t) * omega) / sinom;
    sclq = sin(t * omega) / sinom;
  }
  else
  {
    // Very close, do linear interp (because it's faster)
    sclp = (double)1.0 - t;
    sclq = t;
  }

  for (int i = 0; i < 4; i++)
  {
    result[i] = sclp * p[i] + sclq * q[i];
  }
}

//----------------------------------------------------------------------------
// Interpolate the matrix for the given timestamp from the two nearest
// transforms in the buffer.
// The rotation is interpolated with SLERP interpolation, and the
// position is interpolated with linear interpolation.
// The flags correspond to the closest element.
void vtkSlicerTransformProcessorLogic::GetInterpolatedTransform(vtkMatrix4x4* itemAmatrix, vtkMatrix4x4* itemBmatrix,
  double itemAweight, double itemBweight,
  vtkMatrix4x4* interpolatedMatrix)
{
  double itemAweightNormalized = itemAweight / (itemAweight + itemBweight);
  double itemBweightNormalized = itemBweight / (itemAweight + itemBweight);

  double matrixA[3][3] = { {0,0,0},{0,0,0},{0,0,0} };
  for (int i = 0; i < 3; i++)
  {
    matrixA[i][0] = itemAmatrix->GetElement(i, 0);
    matrixA[i][1] = itemAmatrix->GetElement(i, 1);
    matrixA[i][2] = itemAmatrix->GetElement(i, 2);
  }

  double matrixB[3][3] = { {0,0,0}, {0,0,0}, {0,0,0} };
  for (int i = 0; i < 3; i++)
  {
    matrixB[i][0] = itemBmatrix->GetElement(i, 0);
    matrixB[i][1] = itemBmatrix->GetElement(i, 1);
    matrixB[i][2] = itemBmatrix->GetElement(i, 2);
  }

  double matrixAquat[4] = { 0,0,0,0 };
  vtkMath::Matrix3x3ToQuaternion(matrixA, matrixAquat);
  double matrixBquat[4] = { 0,0,0,0 };
  vtkMath::Matrix3x3ToQuaternion(matrixB, matrixBquat);
  double interpolatedRotationQuat[4] = { 0,0,0,0 };
  this->Slerp(interpolatedRotationQuat, itemBweightNormalized, matrixAquat, matrixBquat);
  double interpolatedRotation[3][3] = { {0,0,0},{0,0,0},{0,0,0} };
  vtkMath::QuaternionToMatrix3x3(interpolatedRotationQuat, interpolatedRotation);

  for (int i = 0; i < 3; i++)
  {
    interpolatedMatrix->Element[i][0] = interpolatedRotation[i][0];
    interpolatedMatrix->Element[i][1] = interpolatedRotation[i][1];
    interpolatedMatrix->Element[i][2] = interpolatedRotation[i][2];
    interpolatedMatrix->Element[i][3] = itemAmatrix->GetElement(i, 3) * itemAweightNormalized +
      itemBmatrix->GetElement(i, 3) * itemBweightNormalized;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::UpdateAllOutputs()
{
  for (auto paramNode : this->ContinuouslyUpdatedNodes)
  {
    if (paramNode->GetUpdateMode() == vtkMRMLTransformProcessorNode::UPDATE_MODE_AUTO &&
      paramNode->GetProcessingMode() == vtkMRMLTransformProcessorNode::PROCESSING_MODE_STABILIZE && paramNode->GetStabilizationEnabled())
    {
      this->UpdateOutputTransform(paramNode);
    }
  }
}
