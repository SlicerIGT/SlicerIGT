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

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
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

  if ( node->IsA( "vtkMRMLTransformProcessorNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
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
  if ( event != vtkMRMLTransformProcessorNode::InputDataModifiedEvent &&
       event != vtkCommand::ModifiedEvent )
  {
    return;
  }

  if ( paramNode->GetUpdateMode() == vtkMRMLTransformProcessorNode::UPDATE_MODE_AUTO )
  {
    this->UpdateOutputTransform( paramNode );
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
  bool conditionsMetForProcessing = IsTransformProcessingPossible( paramNode, verboseWarnings );
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
  bool conditionsMetForProcessing = IsTransformProcessingPossible( paramNode, verboseWarnings );
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
  inputChangedNode->GetTransformToNode( inputInitialNode, inputChangedToInputInitialTransform );
  double shaftDirection[ 3 ] = { 0.0, 0.0, -1.0 }; // conventional shaft direction in SlicerIGT
  vtkSmartPointer< vtkTransform > adjustedToInputInitialRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  GetRotationSingleAxisWithPivotFromTransform( inputChangedToInputInitialTransform, shaftDirection, adjustedToInputInitialRotationOnlyTransform );

  vtkMRMLLinearTransformNode* inputAnchorNode = paramNode->GetInputAnchorTransformNode();
  vtkSmartPointer< vtkGeneralTransform > inputInitialToInputAnchorTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  inputInitialNode->GetTransformToNode( inputAnchorNode, inputInitialToInputAnchorTransform );
  vtkSmartPointer< vtkTransform > inputInitialToInputAnchorRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  GetRotationAllAxesFromTransform( inputInitialToInputAnchorTransform, inputInitialToInputAnchorRotationOnlyTransform );
  
  // Translation is same as input translation, since they share the same origin
  vtkSmartPointer< vtkGeneralTransform > inputChangedToInputAnchorTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  inputChangedNode->GetTransformToNode( inputAnchorNode, inputChangedToInputAnchorTransform );
  vtkSmartPointer< vtkTransform > inputChangedToInputAnchorTranslationTransform = vtkSmartPointer< vtkTransform >::New();
  bool copyComponents[ 3 ] = { 1, 1, 1 }; // copy x, y, and z
  GetTranslationOnlyFromTransform( inputChangedToInputAnchorTransform, copyComponents, inputChangedToInputAnchorTranslationTransform );

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
  bool conditionsMetForProcessing = IsTransformProcessingPossible( paramNode, verboseWarnings );
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

  vtkSmartPointer< vtkGeneralTransform > fromToToTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLLinearTransformNode* fromTransformNode = paramNode->GetInputFromTransformNode();
  vtkMRMLLinearTransformNode* toTransformNode = paramNode->GetInputToTransformNode();
  toTransformNode->GetTransformFromNode( fromTransformNode, fromToToTransform );

  // if there are other modes that need to check and corrrect for duplicate axes, these should be added below:
  if ( paramNode->GetDependentAxesMode() == vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS )
  {
    paramNode->CheckAndCorrectForDuplicateAxes();
  }

  // computation
  vtkSmartPointer< vtkTransform > fromToToRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  GetRotationOnlyFromTransform( fromToToTransform, rotationMode, dependentAxesMode, primaryAxis, secondaryAxis, fromToToRotationOnlyTransform );
  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  // the existence of outputNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputNode->SetMatrixTransformToParent( fromToToRotationOnlyTransform->GetMatrix() );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeTranslation( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  // get parameters from parameter node
  const bool* copyComponents = paramNode->GetCopyTranslationComponents();
  vtkSmartPointer< vtkGeneralTransform > fromToToGeneralTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLLinearTransformNode* fromTransformNode = paramNode->GetInputFromTransformNode();
  vtkMRMLLinearTransformNode* toTransformNode = paramNode->GetInputToTransformNode();
  toTransformNode->GetTransformFromNode( fromTransformNode, fromToToGeneralTransform );
  vtkSmartPointer< vtkTransform > fromToToTranslationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  GetTranslationOnlyFromTransform( fromToToGeneralTransform, copyComponents, fromToToTranslationOnlyTransform );
  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  // the existence of outputNode is already checked in IsTransformProcessingPossible, no error check necessary
  outputNode->SetMatrixTransformToParent( fromToToTranslationOnlyTransform->GetMatrix() );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformProcessorLogic::ComputeFullTransform( vtkMRMLTransformProcessorNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForProcessing = IsTransformProcessingPossible( paramNode, verboseWarnings );
  if ( conditionsMetForProcessing == false )
  {
    return;
  }

  vtkSmartPointer< vtkGeneralTransform > fromToToGeneralTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLLinearTransformNode* fromTransformNode = paramNode->GetInputFromTransformNode();
  vtkMRMLLinearTransformNode* toTransformNode = paramNode->GetInputToTransformNode();
  toTransformNode->GetTransformFromNode( fromTransformNode, fromToToGeneralTransform );

  // need to convert the general transform to a matrix. Decompose then concatenate the rotation and translation
  vtkSmartPointer< vtkTransform > fromToToRotationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  GetRotationAllAxesFromTransform( fromToToGeneralTransform, fromToToRotationOnlyTransform );
  bool copyComponents[ 3 ] = { 1, 1, 1 }; // copy x, y, and z
  vtkSmartPointer< vtkTransform > fromToToTranslationOnlyTransform = vtkSmartPointer< vtkTransform >::New();
  GetTranslationOnlyFromTransform( fromToToGeneralTransform, copyComponents, fromToToTranslationOnlyTransform );
  
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
      GetRotationAllAxesFromTransform( sourceToTargetTransform, rotationOnlyTransform );
      break;
    case vtkMRMLTransformProcessorNode::ROTATION_MODE_COPY_SINGLE_AXIS:
      GetRotationSingleAxisFromTransform( sourceToTargetTransform, dependentAxesMode, primaryAxis, secondaryAxis, rotationOnlyTransform );
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
    vtkErrorMacro( "GetRotationAllAxesFromTransform: rotationOnlyTransform is null. Returning, but transform will remain null." );
    return;
  }

  if ( sourceToTargetTransform == NULL )
  {
    vtkErrorMacro( "GetRotationAllAxesFromTransform: sourceToTargetTransform is null. Returning, but no operation performed." );
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
  GetRotationMatrixFromAxes( xAxisInput, yAxisInput, zAxisInput, rotationOnlyMatrix );
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
      GetRotationSingleAxisWithPivotFromTransform( sourceToTargetTransform, primaryAxis, rotationOnlyTransform );
      break;
    case vtkMRMLTransformProcessorNode::DEPENDENT_AXES_MODE_FROM_SECONDARY_AXIS:
      GetRotationSingleAxisWithSecondaryFromTransform( sourceToTargetTransform, primaryAxis, secondaryAxis, rotationOnlyTransform );
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
  this->GetRotationAllAxesFromTransform( sourceToTargetTransform, sourceToTargetRotationOnlyTransform );
  
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
    vtkErrorMacro( "GetTranslationOnlyFromTransform: translationOnlyTransform is null. Returning, but transform will remain null." );
    return;
  }

  if ( sourceToTargetTransform == NULL )
  {
    vtkErrorMacro( "GetTranslationOnlyFromTransform: inputFullTransform is null. Returning, but no operation performed." );
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

  // check every possible condition,
  // then check whether the current mode applies
  // If many more modes are added, then this scheme should be re-evaluated
  
  if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_ROTATION ||
       mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_TRANSLATION ||
       mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_FULL_TRANSFORM )
  {
    if ( node->GetInputFromTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: No \"From\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
    if ( node->GetInputToTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: No \"To\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
  }

  if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_COMPUTE_SHAFT_PIVOT )
  {
    if ( node->GetInputInitialTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: No \"Initial\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
    if ( node->GetInputChangedTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: No \"Changed\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
    if ( node->GetInputAnchorTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformProcessingPossible: No \"Anchor\" node provided for processing mode " << vtkMRMLTransformProcessorNode::GetProcessingModeAsString( mode ) );
      }
      result = false;
    }
  }

  if ( mode == vtkMRMLTransformProcessorNode::PROCESSING_MODE_QUATERNION_AVERAGE )
  {
    if ( node->GetNumberOfInputCombineTransformNodes() < 2 )
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
