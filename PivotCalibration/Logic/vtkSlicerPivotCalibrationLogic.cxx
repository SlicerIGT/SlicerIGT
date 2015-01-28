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
  
==============================================================================*/  

// PivotCalibration Logic includes
#include "vtkSlicerPivotCalibrationLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>
#include <cmath>

// VNL includes
#include "vnl/algo/vnl_symmetric_eigensystem.h"
#include "vnl/vnl_vector.h"
#include "vnl/algo/vnl_svd.h"
#include "vnl/algo/vnl_determinant.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPivotCalibrationLogic);

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::vtkSlicerPivotCalibrationLogic()
{
  this->ToolTipToToolMatrix = vtkMatrix4x4::New();
}

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::~vtkSlicerPivotCalibrationLogic()
{
  this->ClearToolToReferenceMatrices();
  this->ToolTipToToolMatrix->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf( os, indent );
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller != NULL)
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
    if ( event = vtkMRMLLinearTransformNode::TransformModifiedEvent && this->RecordingState == true && transformNode->GetID() == this->ObservedTransformID)
    {
#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
      vtkMatrix4x4* matrixCopy = vtkMatrix4x4::New();
      transformNode->GetMatrixTransformToParent(matrixCopy);
#else
      vtkMatrix4x4* matrixToParent = transformNode->GetMatrixTransformToParent();
      vtkMatrix4x4* matrixCopy = vtkMatrix4x4::New();
      matrixCopy->DeepCopy(matrixToParent);
#endif
      
      this->AddToolToReferenceMatrix(matrixCopy);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::InitializeObserver(vtkMRMLNode* node)
{
  if (node != NULL)
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
    
    this->ObservedTransformID = transformNode->GetID();
    
    node->AddObserver(vtkMRMLLinearTransformNode::TransformModifiedEvent, (vtkCommand*) this->GetMRMLNodesCallbackCommand());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::AddToolToReferenceMatrix(vtkMatrix4x4* transformMatrix)
{
  this->ToolToReferenceMatrices.push_back(transformMatrix);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ClearToolToReferenceMatrices()
{
  std::vector<vtkMatrix4x4*>::const_iterator it; 
  std::vector<vtkMatrix4x4*>::const_iterator matricesEnd = this->ToolToReferenceMatrices.end();
  for(it = this->ToolToReferenceMatrices.begin(); it != matricesEnd; it++)
  {
    (*it)->Delete();
  }
  this->ToolToReferenceMatrices.clear();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ComputePivotCalibration()
{
  if (this->ToolToReferenceMatrices.size() == 0)
  {
    return;
  }

  unsigned int rows = 3 * this->ToolToReferenceMatrices.size();
  unsigned int columns = 6;

  vnl_matrix<double> A(rows, columns), minusI(3,3,0), R(3,3);
  vnl_vector<double> b(rows), x(columns), t(3);
  minusI(0, 0) = -1;
  minusI(1, 1) = -1;
  minusI(2, 2) = -1;
        
  std::vector<vtkMatrix4x4*>::const_iterator it;
  std::vector<vtkMatrix4x4*>::const_iterator matricesEnd = this->ToolToReferenceMatrices.end();

  unsigned int currentRow;
  for(currentRow = 0, it = this->ToolToReferenceMatrices.begin(); it != matricesEnd; it++, currentRow += 3)
  {
    for (int i = 0; i < 3; i++)
    {
      t(i) = (*it)->GetElement(i, 3);
    }
    t *= -1;
    b.update(t, currentRow);

    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++ )
      {
        R(i, j) = (*it)->GetElement(i, j);
      }
    }
    A.update(R, currentRow, 0);
    A.update( minusI, currentRow, 3 );    
  }
    
    
  vnl_svd<double> svdA(A);    
  svdA.zero_out_absolute( 1e-1 );    
  x = svdA.solve( b );
    
  //set the RMSE
  this->PivotRMSE = ( A * x - b ).rms();

  //set the transformation
  this->ToolTipToToolMatrix->SetElement( 0, 3, x[ 0 ] );
  this->ToolTipToToolMatrix->SetElement( 1, 3, x[ 1 ] );
  this->ToolTipToToolMatrix->SetElement( 2, 3, x[ 2 ] );

}


//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ComputeSpinCalibration()
{

  if ( this->ToolToReferenceMatrices.size() == 0 )
  {
    return;
  }

  // Find the plane of best fit for the points
  vnl_matrix<double> ToolPoints_Reference( this->ToolToReferenceMatrices.size(), 3, 0.0 );
  vnl_vector<double> meanToolPoint_Reference( 3, 0.0 );

  for ( int i = 0; i < this->ToolToReferenceMatrices.size(); i++ )
  {
    ToolPoints_Reference.put( i, 0, this->ToolToReferenceMatrices.at( i )->GetElement( 0, 3 ) );
    ToolPoints_Reference.put( i, 1, this->ToolToReferenceMatrices.at( i )->GetElement( 1, 3 ) );
    ToolPoints_Reference.put( i, 2, this->ToolToReferenceMatrices.at( i )->GetElement( 2, 3 ) );

    meanToolPoint_Reference.put( 0, meanToolPoint_Reference.get( 0 ) + this->ToolToReferenceMatrices.at( i )->GetElement( 0, 3 ) );
    meanToolPoint_Reference.put( 1, meanToolPoint_Reference.get( 1 ) + this->ToolToReferenceMatrices.at( i )->GetElement( 1, 3 ) );
    meanToolPoint_Reference.put( 2, meanToolPoint_Reference.get( 2 ) + this->ToolToReferenceMatrices.at( i )->GetElement( 2, 3 ) );
  }

  meanToolPoint_Reference.put( 0, meanToolPoint_Reference.get( 0 ) / this->ToolToReferenceMatrices.size() );
  meanToolPoint_Reference.put( 1, meanToolPoint_Reference.get( 1 ) / this->ToolToReferenceMatrices.size() );
  meanToolPoint_Reference.put( 2, meanToolPoint_Reference.get( 2 ) / this->ToolToReferenceMatrices.size() );

  vnl_matrix<double> ToolCovariance_Reference = ToolPoints_Reference.transpose() * ToolPoints_Reference / this->ToolToReferenceMatrices.size() - outer_product( meanToolPoint_Reference, meanToolPoint_Reference );
  
  vnl_matrix<double> ToolCovarianceEigenvectors_Reference( 3, 3, 0.0 );
  vnl_vector<double> ToolCovarianceEigenvalues_Reference( 3, 0.0 );
  vnl_symmetric_eigensystem_compute( ToolCovariance_Reference, ToolCovarianceEigenvectors_Reference, ToolCovarianceEigenvalues_Reference );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )

  vnl_vector<double> basisVector1_Projection = ToolCovarianceEigenvectors_Reference.get_column( 1 );
  vnl_vector<double> basisVector2_Projection = ToolCovarianceEigenvectors_Reference.get_column( 2 );

  // Project positions onto plane
  vnl_matrix<double> ToolCircleMatrix_Projection( ToolPoints_Reference.rows(), 3, 0.0 );
  vnl_vector<double> ToolCircleVector_Projection( ToolPoints_Reference.rows(), 0.0 );

  for ( int i = 0; i < ToolPoints_Reference.rows(); i++ )
  {
    vnl_vector<double> currentToolPoint_Reference = ToolPoints_Reference.get_row( i ) - meanToolPoint_Reference;
    vnl_vector<double> currentToolTip_Projection( 2, 0.0 );
    currentToolTip_Projection.put( 0, dot_product( currentToolPoint_Reference, basisVector1_Projection ) );
    currentToolTip_Projection.put( 1, dot_product( currentToolPoint_Reference, basisVector2_Projection ) );

    ToolCircleMatrix_Projection.put( i, 0, 2 * currentToolTip_Projection.get( 0 ) );
    ToolCircleMatrix_Projection.put( i, 1, 2 * currentToolTip_Projection.get( 1 ) );
    ToolCircleMatrix_Projection.put( i, 2, 1 );

    ToolCircleVector_Projection.put( i, currentToolTip_Projection.get( 0 ) * currentToolTip_Projection.get( 0 ) + currentToolTip_Projection.get( 1 ) * currentToolTip_Projection.get( 1 ) );
  }

  // Solve the system
  vnl_svd<double> ToolCircleSolver_Reference( ToolCircleMatrix_Projection );
  vnl_vector<double> ToolRotationCentre_Projection = ToolCircleSolver_Reference.solve( ToolCircleVector_Projection );

  // This will compute the rms of the square roots of the residual vector
  // This is a better error metrics because the residuals are the sums/differences of squared quantities
  this->SpinRMSE = sqrt( ( ToolCircleMatrix_Projection * ToolRotationCentre_Projection - ToolCircleVector_Projection ).one_norm() / ToolCircleVector_Projection.size() );

  // Unproject the centre
  vnl_vector<double> ToolRotationCentre_Reference = ToolRotationCentre_Projection.get( 0 ) * basisVector1_Projection + ToolRotationCentre_Projection.get( 1 ) * basisVector2_Projection + meanToolPoint_Reference;
  
  // Put into the ToolTip coordinate system
  double arrayToolRotationCentre_Reference[ 4 ] = { ToolRotationCentre_Reference[ 0 ], ToolRotationCentre_Reference[ 1 ], ToolRotationCentre_Reference[ 2 ], 1 };
  double arrayToolRotationCentre_ToolTip[ 4 ] = { 0, 0, 0, 1 };

  vtkSmartPointer< vtkMatrix4x4 > ReferenceToToolTipTransform = vtkSmartPointer< vtkMatrix4x4 >::New();
  vtkSmartPointer< vtkMatrix4x4 > ReferenceToToolTransform = vtkSmartPointer< vtkMatrix4x4 >::New();
  vtkSmartPointer< vtkMatrix4x4 > ToolToToolTipTransform = vtkSmartPointer< vtkMatrix4x4 >::New();
  ToolToToolTipTransform->DeepCopy( this->ToolTipToToolMatrix );
  ToolToToolTipTransform->Invert();

  for ( int i = 0; i < this->ToolToReferenceMatrices.size(); i++ )
  {
    ReferenceToToolTransform->DeepCopy( this->ToolToReferenceMatrices.at( i ) );
    ReferenceToToolTransform->Invert();
    vtkMatrix4x4::Multiply4x4( ToolToToolTipTransform, ReferenceToToolTransform, ReferenceToToolTipTransform );
    
    double currentArrayToolRotationCentre_ToolTip[ 4 ] = { 0, 0, 0, 1 };
    ReferenceToToolTipTransform->MultiplyPoint( arrayToolRotationCentre_Reference, currentArrayToolRotationCentre_ToolTip );

    arrayToolRotationCentre_ToolTip[ 0 ] += currentArrayToolRotationCentre_ToolTip[ 0 ];
    arrayToolRotationCentre_ToolTip[ 1 ] += currentArrayToolRotationCentre_ToolTip[ 1 ];
    arrayToolRotationCentre_ToolTip[ 2 ] += currentArrayToolRotationCentre_ToolTip[ 2 ];
    arrayToolRotationCentre_ToolTip[ 3 ] += currentArrayToolRotationCentre_ToolTip[ 3 ];
  }

  arrayToolRotationCentre_ToolTip[ 0 ] /= this->ToolToReferenceMatrices.size();
  arrayToolRotationCentre_ToolTip[ 1 ] /= this->ToolToReferenceMatrices.size();
  arrayToolRotationCentre_ToolTip[ 2 ] /= this->ToolToReferenceMatrices.size();
  arrayToolRotationCentre_ToolTip[ 3 ] /= this->ToolToReferenceMatrices.size();

  // This is the shaft point in the ToolTip coordinate frame
  vnl_vector<double> ToolRotationCentre_ToolTip( 3, 0.0 );
  ToolRotationCentre_ToolTip.put( 0, arrayToolRotationCentre_ToolTip[ 0 ] );
  ToolRotationCentre_ToolTip.put( 1, arrayToolRotationCentre_ToolTip[ 1 ] );
  ToolRotationCentre_ToolTip.put( 2, arrayToolRotationCentre_ToolTip[ 2 ] );

  vnl_vector<double> yPoint_ToolTip( 3, 0.0 );
  yPoint_ToolTip.put( 1, 1 ); // Put the y part in
  yPoint_ToolTip = yPoint_ToolTip - dot_product( yPoint_ToolTip, ToolRotationCentre_ToolTip ) * ToolRotationCentre_ToolTip;
  yPoint_ToolTip.normalize();

  // Register X,Y,O points in the two coordinate frames (only spherical registration - since pure rotation)
  vnl_matrix<double> ToolTipPoints( 3, 3, 0.0 );
  vnl_matrix<double> XShaftPoints( 3, 3, 0.0 );

  ToolTipPoints.put( 0, 0, ToolRotationCentre_ToolTip.get( 0 ) );
  ToolTipPoints.put( 0, 1, ToolRotationCentre_ToolTip.get( 1 ) );
  ToolTipPoints.put( 0, 2, ToolRotationCentre_ToolTip.get( 2 ) );
  ToolTipPoints.put( 1, 0, yPoint_ToolTip.get( 0 ) );
  ToolTipPoints.put( 1, 1, yPoint_ToolTip.get( 1 ) );
  ToolTipPoints.put( 1, 2, yPoint_ToolTip.get( 2 ) );
  ToolTipPoints.put( 2, 0, 0 );
  ToolTipPoints.put( 2, 1, 0 );
  ToolTipPoints.put( 2, 2, 0 );

  XShaftPoints.put( 0, 0, 1 );
  XShaftPoints.put( 0, 1, 0 );
  XShaftPoints.put( 0, 2, 0 );
  XShaftPoints.put( 1, 0, 0 );
  XShaftPoints.put( 1, 1, 1 );
  XShaftPoints.put( 1, 2, 0 );
  XShaftPoints.put( 2, 0, 0 );
  XShaftPoints.put( 2, 1, 0 );
  XShaftPoints.put( 2, 2, 0 );
  
  vnl_svd<double> XShaftToToolTipRegistrator( XShaftPoints.transpose() * ToolTipPoints );
  vnl_matrix<double> V = XShaftToToolTipRegistrator.V();
  vnl_matrix<double> U = XShaftToToolTipRegistrator.U();
  vnl_matrix<double> Rotation = V * U.transpose();

  // Make sure the determinant is positve (i.e. +1)
  double determinant = vnl_determinant( Rotation );
  if ( determinant < 0 )
  {
    // Switch the sign of the third column of V if the determinant is not +1
    // This is the recommended approach from Huang et al. 1987
    V.put( 0, 2, -V.get( 0, 2 ) );
    V.put( 1, 2, -V.get( 1, 2 ) );
    V.put( 2, 2, -V.get( 2, 2 ) );
    Rotation = V * U.transpose();
  }

  
  this->ToolTipToToolMatrix->SetElement( 0, 0, Rotation[ 0 ][ 0 ] );
  this->ToolTipToToolMatrix->SetElement( 0, 1, Rotation[ 0 ][ 1 ] );
  this->ToolTipToToolMatrix->SetElement( 0, 2, Rotation[ 0 ][ 2 ] );
  this->ToolTipToToolMatrix->SetElement( 1, 0, Rotation[ 1 ][ 0 ] );
  this->ToolTipToToolMatrix->SetElement( 1, 1, Rotation[ 1 ][ 1 ] );
  this->ToolTipToToolMatrix->SetElement( 1, 2, Rotation[ 1 ][ 2 ] );
  this->ToolTipToToolMatrix->SetElement( 2, 0, Rotation[ 2 ][ 0 ] );
  this->ToolTipToToolMatrix->SetElement( 2, 1, Rotation[ 2 ][ 1 ] );
  this->ToolTipToToolMatrix->SetElement( 2, 2, Rotation[ 2 ][ 2 ] );
}


//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SnapRotationRightAngle()
{
  // The idea is to find the axis-angle representation
  // Then find the closest coordinate axis to the rotation
  // Then round the angle to the nearest multiple of 90 degrees
  vtkSmartPointer< vtkTransform > XShaftToToolTipTransform = vtkSmartPointer< vtkTransform >::New();
  XShaftToToolTipTransform->SetMatrix( this->ToolTipToToolMatrix );

  // Get the axis-angl representation
  double XShaftToToolTipOrientationWXYZ[ 4 ] = { 0.0, 0.0, 0.0, 0.0 };
  XShaftToToolTipTransform->GetOrientationWXYZ( XShaftToToolTipOrientationWXYZ );

  // Calculate the right-angle axis vector
  vnl_vector<double> XShaftToToolTipAxisVector( 3, 0.0 );
  XShaftToToolTipAxisVector.put( 0, XShaftToToolTipOrientationWXYZ[ 1 ] );
  XShaftToToolTipAxisVector.put( 1, XShaftToToolTipOrientationWXYZ[ 2 ] );
  XShaftToToolTipAxisVector.put( 2, XShaftToToolTipOrientationWXYZ[ 3 ] );
  int maxAbsAxisVectorLocation = element_product( XShaftToToolTipAxisVector, XShaftToToolTipAxisVector ).arg_max();

  vnl_vector<double> XShaftToToolTipSnapAxisVector( 3, 0.0 );
  XShaftToToolTipSnapAxisVector.put( maxAbsAxisVectorLocation, XShaftToToolTipAxisVector.get( maxAbsAxisVectorLocation ) );
  XShaftToToolTipSnapAxisVector.normalize();

  double XShaftToToolTipSnapAxisArray[ 3 ] = { 0.0, 0.0, 0.0 };
  XShaftToToolTipSnapAxisArray[ 0 ] = XShaftToToolTipSnapAxisVector.get( 0 );
  XShaftToToolTipSnapAxisArray[ 1 ] = XShaftToToolTipSnapAxisVector.get( 1 );
  XShaftToToolTipSnapAxisArray[ 2 ] = XShaftToToolTipSnapAxisVector.get( 2 );

  // Calculate the 90 degree rotation
  double XShaftToToolTipSnapAngle = 90 * floor( XShaftToToolTipOrientationWXYZ[ 0 ] / 90 + 0.5 ); // Trick for raounding without round function

  // Put it back into a matrix
  vtkSmartPointer< vtkTransform > XShaftToToolTipSnapTransform = vtkSmartPointer< vtkTransform >::New();
  XShaftToToolTipSnapTransform->RotateWXYZ( XShaftToToolTipSnapAngle, XShaftToToolTipSnapAxisArray );
  vtkSmartPointer< vtkMatrix4x4 > XShaftToToolTipSnapMatrix = XShaftToToolTipSnapTransform->GetMatrix();

  this->ToolTipToToolMatrix->SetElement( 0, 0, XShaftToToolTipSnapMatrix->GetElement( 0, 0 ) );
  this->ToolTipToToolMatrix->SetElement( 0, 1, XShaftToToolTipSnapMatrix->GetElement( 0, 1 ) );
  this->ToolTipToToolMatrix->SetElement( 0, 2, XShaftToToolTipSnapMatrix->GetElement( 0, 2 ) );
  this->ToolTipToToolMatrix->SetElement( 1, 0, XShaftToToolTipSnapMatrix->GetElement( 1, 0 ) );
  this->ToolTipToToolMatrix->SetElement( 1, 1, XShaftToToolTipSnapMatrix->GetElement( 1, 1 ) );
  this->ToolTipToToolMatrix->SetElement( 1, 2, XShaftToToolTipSnapMatrix->GetElement( 1, 2 ) );
  this->ToolTipToToolMatrix->SetElement( 2, 0, XShaftToToolTipSnapMatrix->GetElement( 2, 0 ) );
  this->ToolTipToToolMatrix->SetElement( 2, 1, XShaftToToolTipSnapMatrix->GetElement( 2, 1 ) );
  this->ToolTipToToolMatrix->SetElement( 2, 2, XShaftToToolTipSnapMatrix->GetElement( 2, 2 ) );

}


//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetRecordingState(bool state)
{
  this->RecordingState = state;
}


bool vtkSlicerPivotCalibrationLogic::GetRecordingState()
{
  return this->RecordingState;
}


void vtkSlicerPivotCalibrationLogic
::GetToolTipToToolTranslation( vtkMatrix4x4* translationMatrix )
{
  translationMatrix->Identity();

  translationMatrix->SetElement( 0, 3, this->ToolTipToToolMatrix->GetElement( 0, 3 ) );
  translationMatrix->SetElement( 1, 3, this->ToolTipToToolMatrix->GetElement( 1, 3 ) );
  translationMatrix->SetElement( 2, 3, this->ToolTipToToolMatrix->GetElement( 2, 3 ) );
}

void vtkSlicerPivotCalibrationLogic
::GetToolTipToToolRotation( vtkMatrix4x4* rotationMatrix )
{
  rotationMatrix->Identity();

  rotationMatrix->SetElement( 0, 0, this->ToolTipToToolMatrix->GetElement( 0, 0 ) );
  rotationMatrix->SetElement( 0, 1, this->ToolTipToToolMatrix->GetElement( 0, 1 ) );
  rotationMatrix->SetElement( 0, 2, this->ToolTipToToolMatrix->GetElement( 0, 2 ) );
  rotationMatrix->SetElement( 1, 0, this->ToolTipToToolMatrix->GetElement( 1, 0 ) );
  rotationMatrix->SetElement( 1, 1, this->ToolTipToToolMatrix->GetElement( 1, 1 ) );
  rotationMatrix->SetElement( 1, 2, this->ToolTipToToolMatrix->GetElement( 1, 2 ) );
  rotationMatrix->SetElement( 2, 0, this->ToolTipToToolMatrix->GetElement( 2, 0 ) );
  rotationMatrix->SetElement( 2, 1, this->ToolTipToToolMatrix->GetElement( 2, 1 ) );
  rotationMatrix->SetElement( 2, 2, this->ToolTipToToolMatrix->GetElement( 2, 2 ) );
}

void vtkSlicerPivotCalibrationLogic
::GetToolTipToToolMatrix( vtkMatrix4x4* matrix )
{
  matrix->DeepCopy( this->ToolTipToToolMatrix );
}


double vtkSlicerPivotCalibrationLogic
::GetPivotRMSE()
{
  return this->PivotRMSE;
}


double vtkSlicerPivotCalibrationLogic
::GetSpinRMSE()
{
  return this->SpinRMSE;
}


/*
//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}
//*/

