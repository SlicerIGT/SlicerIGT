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

// TransformFusion includes
#include "vtkSlicerTransformFusionLogic.h"
#include "vtkMRMLTransformFusionNode.h"

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

vtkStandardNewMacro( vtkSlicerTransformFusionLogic );

//-----------------------------------------------------------------------------
vtkSlicerTransformFusionLogic::vtkSlicerTransformFusionLogic()
{
}

//-----------------------------------------------------------------------------
vtkSlicerTransformFusionLogic::~vtkSlicerTransformFusionLogic()
{
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if ( scene == NULL )
  {
    vtkErrorMacro( "vtkSlicerTransformFusionLogic::RegisterNodes failed: invalid scene" );
    return;
  }
  
  scene->RegisterNodeClass( vtkSmartPointer<vtkMRMLTransformFusionNode>::New() );
}

//---------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLTransformFusionNode* pNode = vtkMRMLTransformFusionNode::SafeDownCast( node );
  if ( pNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( pNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLTransformFusionNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( pNode, events.GetPointer() );
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLTransformFusionNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* vtkNotUsed(callData) )
{
  vtkMRMLTransformFusionNode* paramNode = vtkMRMLTransformFusionNode::SafeDownCast( caller );
  if ( paramNode == NULL )
  {
    return;
  }

  if ( event==vtkMRMLTransformFusionNode::InputDataModifiedEvent )
  {
    if ( paramNode->GetUpdateMode() == vtkMRMLTransformFusionNode::UPDATE_MODE_AUTO )
    {
      this->UpdateOutputTransform( paramNode );
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::UpdateOutputTransform( vtkMRMLTransformFusionNode* paramNode )
{
  if ( paramNode->GetFusionMode() == vtkMRMLTransformFusionNode::FUSION_MODE_QUATERNION_AVERAGE )
  {
    this->QuaternionAverage( paramNode );
  }
  else if ( paramNode->GetFusionMode() == vtkMRMLTransformFusionNode::FUSION_MODE_CONSTRAIN_SHAFT_ROTATION )
  {
    this->ConstrainShaftRotation( paramNode );
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
void vtkSlicerTransformFusionLogic::QuaternionAverage( vtkMRMLTransformFusionNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForFusion = IsTransformFusionPossible( paramNode, verboseWarnings );
  if ( conditionsMetForFusion == false )
  {
    return;
  }

  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  if ( outputNode == NULL )
  {
    return;
  }

  // Average quaternion
  vtkSmartPointer< vtkMatrix4x4 > outputMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  int numberOfInputs = paramNode->GetNumberOfInputTransformNodes();
  // numberOfInputs is greater than 1, as checked by IsTransformFusionPossible
  vtkSmartPointer< vtkMatrix4x4 > matrix4x4Pointer = vtkSmartPointer< vtkMatrix4x4 >::New();

  float rotationMatrix[ 3 ][ 3 ] = { { 0 } };
  float averageRotationMatrix[ 3 ][ 3 ] = { { 0 } };
  float singleQuaternion[ 4 ] = { 0 };
  float averageQuaternion[ 4 ] = { 0 };

  for ( int i = 0; i < numberOfInputs; i++ )
  {

    paramNode->GetNthInputTransformNode( i )->GetMatrixTransformToParent( matrix4x4Pointer );

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
      outputMatrix->SetElement( row, column, averageRotationMatrix[ row ][ column ] );
    }
  }

  // Average linear elements
  double value = 0;
  for ( int row = 0; row < 4; row++ )
  {
    for ( int i = 0; i < numberOfInputs; i++ )
    {
      paramNode->GetNthInputTransformNode( i )->GetMatrixTransformToParent( matrix4x4Pointer );
      value += matrix4x4Pointer->GetElement( row, 3 );
    }
    value = value / numberOfInputs;
    outputMatrix->SetElement( row, 3 , value );
    value = 0;
  }

  outputNode->SetMatrixTransformToParent( outputMatrix );
}

//-----------------------------------------------------------------------------
// Re-express the Input transform so that the shaft direction from the Input is 
// preserved, but the other axes resemble the Resting coordinate system
void vtkSlicerTransformFusionLogic::ConstrainShaftRotation( vtkMRMLTransformFusionNode* paramNode )
{
  bool verboseWarnings = true;
  bool conditionsMetForFusion = IsTransformFusionPossible( paramNode, verboseWarnings );
  if ( conditionsMetForFusion == false )
  {
    return;
  }

  // first determine the shaft directions
  double shaftDirectionInInput[ 3 ] = { 0.0, 0.0, -1.0 };
  double shaftDirectionInResting[ 3 ];
  TransformVectorInputToResting( paramNode, shaftDirectionInInput, shaftDirectionInResting );

  // Key point: We REFER to the Resting transform, then
  // rotate between it and the Input. We use an axis-angle rotation,
  // but make sure that the rotation axis is perpendicular to the shaft.
  // This eliminates any rotation about the shaft itself, so the other
  // two axes are aligned as closely as possible.

  // compute axis and angle between shaft directions (input vs resting)
  double rotationAxisInInputToResting[ 3 ];
  // cross product will be perpendicular to the inputs
  vtkMath::Cross( shaftDirectionInInput, shaftDirectionInResting, rotationAxisInInputToResting );
  double rotationDegreesInputToResting = asin( vtkMath::Norm( rotationAxisInInputToResting) ) * 180.0 / vtkMath::Pi();
  vtkMath::Normalize( rotationAxisInInputToResting );
  float epsilon = 0.00001;
  if ( vtkMath::Norm( rotationAxisInInputToResting ) <= epsilon )
  {
    // if the axis is zero, then there is no rotation and any arbitrary axis is fine.
    rotationAxisInInputToResting[ 0 ] = 1.0;
    rotationAxisInInputToResting[ 1 ] = 0.0;
    rotationAxisInInputToResting[ 2 ] = 0.0;
    rotationDegreesInputToResting = 0.0;
  }

  // The output will be the AdjustedToReference transform.
  // We will compute AdjustedToReferenceRotation = RestingToReferenceRotation * AdjustedToRestingRotation
  // Translation must be handled separately because constrained orientation will change position values in the transform chain

  // First we will take care of the rotation components.
  vtkSmartPointer< vtkTransform > adjustedToRestingRotationTransform = vtkSmartPointer< vtkTransform >::New();
  adjustedToRestingRotationTransform->Identity();
  adjustedToRestingRotationTransform->RotateWXYZ( rotationDegreesInputToResting, rotationAxisInInputToResting );

  vtkSmartPointer< vtkTransform > restingToReferenceRotationTransform = vtkSmartPointer< vtkTransform >::New();
  GetRestingToReferenceRotationTransform( paramNode, restingToReferenceRotationTransform );
  
  // Translation is same as input translation, since they share the same origin
  double adjustedToReferenceTranslation[ 3 ];
  GetInputToReferenceTranslation( paramNode, adjustedToReferenceTranslation );

  // put it all together
  vtkSmartPointer< vtkTransform > adjustedToReference = vtkSmartPointer< vtkTransform >::New();
  adjustedToReference->PreMultiply();
  adjustedToReference->Identity();
  adjustedToReference->Translate( adjustedToReferenceTranslation );
  adjustedToReference->Concatenate( restingToReferenceRotationTransform ); // part of rotation
  adjustedToReference->Concatenate( adjustedToRestingRotationTransform ); // part of rotation
  adjustedToReference->Update();

  vtkMRMLLinearTransformNode* outputNode = paramNode->GetOutputTransformNode();
  // the existence of outputNode is already checked in IsTransformFusionPossible, no error check necessary
  outputNode->SetMatrixTransformToParent( adjustedToReference->GetMatrix() );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::TransformVectorInputToResting( vtkMRMLTransformFusionNode* paramNode, const double input[3], double resting[3] )
{
  vtkSmartPointer< vtkGeneralTransform > inputToRestingTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  paramNode->GetSingleInputTransformNode()->GetTransformToNode( paramNode->GetRestingTransformNode(), inputToRestingTransform );
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };
  inputToRestingTransform->TransformVectorAtPoint( zeroVector3, input, resting );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::GetRestingToReferenceRotationTransform( vtkMRMLTransformFusionNode* paramNode, vtkTransform* restingToReferenceRotationOnlyTransform )
{
  if ( restingToReferenceRotationOnlyTransform == NULL )
  {
    vtkErrorMacro( "GetRestingToReferenceRotationTransform: restingToReferenceRotationOnlyTransform is null. Returning, but transform will remain null." );
    return;
  }

  vtkSmartPointer< vtkGeneralTransform > restingToReferenceTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  paramNode->GetRestingTransformNode()->GetTransformToNode( paramNode->GetReferenceTransformNode(), restingToReferenceTransform );

  // Unfortunately, the vtkGeneralTransform class does not provide an easy way to copy the RestingToReference
  // and leave the translation component at zero. So we just have to do some manual vtk math here to find the axes
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };
  double xAxisResting[ 3 ] = { 1.0, 0.0, 0.0 };
  double xAxisReference[ 3 ];
  restingToReferenceTransform->TransformVectorAtPoint( zeroVector3, xAxisResting, xAxisReference );
  double yAxisResting[ 3 ] = { 0.0, 1.0, 0.0 };
  double yAxisReference[ 3 ];
  restingToReferenceTransform->TransformVectorAtPoint( zeroVector3, yAxisResting, yAxisReference );
  double zAxisResting[ 3 ] = { 0.0, 0.0, 1.0 };
  double zAxisReference[ 3 ];
  restingToReferenceTransform->TransformVectorAtPoint( zeroVector3, zAxisResting, zAxisReference );
  
  // set the matrix accordingly
  vtkSmartPointer< vtkMatrix4x4 > restingToReferenceRotationOnlyMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  restingToReferenceRotationOnlyMatrix->Identity();
  restingToReferenceRotationOnlyMatrix->SetElement( 0, 0, xAxisReference[ 0 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 1, 0, xAxisReference[ 1 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 2, 0, xAxisReference[ 2 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 0, 1, yAxisReference[ 0 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 1, 1, yAxisReference[ 1 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 2, 1, yAxisReference[ 2 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 0, 2, zAxisReference[ 0 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 1, 2, zAxisReference[ 1 ] );
  restingToReferenceRotationOnlyMatrix->SetElement( 2, 2, zAxisReference[ 2 ] );
  restingToReferenceRotationOnlyTransform->SetMatrix( restingToReferenceRotationOnlyMatrix );
}

//----------------------------------------------------------------------------
void vtkSlicerTransformFusionLogic::GetInputToReferenceTranslation( vtkMRMLTransformFusionNode* paramNode, double translation[3] )
{
  vtkSmartPointer< vtkGeneralTransform > inputToReferenceTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  paramNode->GetSingleInputTransformNode()->GetTransformToNode( paramNode->GetReferenceTransformNode(), inputToReferenceTransform );
  double zeroVector3[ 3 ] = { 0.0, 0.0, 0.0 };
  inputToReferenceTransform->TransformPoint( zeroVector3, translation );
  
}

//----------------------------------------------------------------------------
bool vtkSlicerTransformFusionLogic::IsTransformFusionPossible( vtkMRMLTransformFusionNode *node, bool verbose )
{
  // if verbose, output why the conditions fail
  bool result = true; // assume the conditions are met until otherwise shown
  if ( node->GetFusionMode() == vtkMRMLTransformFusionNode::FUSION_MODE_QUATERNION_AVERAGE )
  {
    if ( node->GetNumberOfInputTransformNodes() < 2 )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformFusionPossible: Not enough input transforms as input for fusion mode " << vtkMRMLTransformFusionNode::GetFusionModeAsString( node->GetFusionMode() ) );
      }
      result = false;
    }
    if ( node->GetOutputTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformFusionPossible: No output transform provided for fusion mode " << vtkMRMLTransformFusionNode::GetFusionModeAsString( node->GetFusionMode() ) );
      }
      result = false;
    }
    return result;
  }
  else if ( node->GetFusionMode() == vtkMRMLTransformFusionNode::FUSION_MODE_CONSTRAIN_SHAFT_ROTATION )
  {
    if ( node->GetSingleInputTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformFusionPossible: No input transform provided for fusion mode " << vtkMRMLTransformFusionNode::GetFusionModeAsString( node->GetFusionMode() ) );
      }
      result = false;
    }
    if ( node->GetRestingTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformFusionPossible: No resting transform provided for fusion mode " << vtkMRMLTransformFusionNode::GetFusionModeAsString( node->GetFusionMode() ) );
      }
      result = false;
    }
    if ( node->GetReferenceTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformFusionPossible: No reference transform provided for fusion mode " << vtkMRMLTransformFusionNode::GetFusionModeAsString( node->GetFusionMode() ) );
      }
      result = false;
    }
    if ( node->GetOutputTransformNode() == NULL )
    {
      if ( verbose )
      {
        vtkWarningMacro( "IsTransformFusionPossible: No output transform provided for fusion mode " << vtkMRMLTransformFusionNode::GetFusionModeAsString( node->GetFusionMode() ) );
      }
      result = false;
    }
    return result;
  }
  else
  {
    // output regardless of verbose, since this is an error that should not happen at all.
    // output the fusion mode number, since the "Unknown" string is not going to be useful
    vtkErrorMacro( "IsTransformFusionPossible: Unrecognized fusion mode provided as input: " << node->GetFusionMode() << ". Returning false." );
    return false;
  }
}
