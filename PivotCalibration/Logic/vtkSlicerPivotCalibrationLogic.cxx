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


static const double PARALLEL_ANGLE_THRESHOLD_DEGREES = 20.0;
// Note: If the needle orientation protocol changes, only the definitions of shaftAxis and secondaryAxes need to be changed
// Define the shaft axis and the secondary shaft axis
// Current needle orientation protocol dictates: shaft axis -z, orthogonal axis +x
// If StylusX is parallel to ShaftAxis then: shaft axis -z, orthogonal axis +y
static const double SHAFT_AXIS[ 3 ] = { 0, 0, -1 };
static const double ORTHOGONAL_AXIS[ 3 ] = { 1, 0, 0 };
static const double BACKUP_AXIS[ 3 ] = { 0, 1, 0 };

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPivotCalibrationLogic);

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::vtkSlicerPivotCalibrationLogic()
{
  this->ToolTipToToolMatrix = vtkMatrix4x4::New();
  this->ObservedTransformNode = NULL;
  this->MinimumOrientationDifferenceDeg = 15.0;
}

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::~vtkSlicerPivotCalibrationLogic()
{
  this->ClearToolToReferenceMatrices();
  this->ToolTipToToolMatrix->Delete();
  this->SetAndObserveTransformNode( NULL ); // Remove the observer
}

//----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf( os, indent );
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  if (caller != NULL)
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
    if ( event == vtkMRMLLinearTransformNode::TransformModifiedEvent && this->RecordingState == true && strcmp( transformNode->GetID(), this->ObservedTransformNode->GetID() ) == 0 )
    {
      vtkMatrix4x4* matrixCopy = vtkMatrix4x4::New();
      transformNode->GetMatrixTransformToParent(matrixCopy);      
      this->AddToolToReferenceMatrix(matrixCopy);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetAndObserveTransformNode( vtkMRMLLinearTransformNode* transformNode )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLLinearTransformNode::TransformModifiedEvent );
  vtkSetAndObserveMRMLNodeEventsMacro( this->ObservedTransformNode, transformNode, events.GetPointer() );
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::AddToolToReferenceMatrix(vtkMatrix4x4* transformMatrix)
{
  if (!transformMatrix)
  {
    vtkErrorMacro("vtkSlicerPivotCalibrationLogic::AddToolToReferenceMatrix failed: invalid transformMatrix");
    return;
  }
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

//----------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetOrientationDifferenceDeg(vtkMatrix4x4* aMatrix, vtkMatrix4x4* bMatrix)
{
  vtkSmartPointer<vtkMatrix4x4> diffMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); 
  vtkSmartPointer<vtkMatrix4x4> invBmatrix = vtkSmartPointer<vtkMatrix4x4>::New(); 

  vtkMatrix4x4::Invert(bMatrix, invBmatrix);  

  vtkMatrix4x4::Multiply4x4(aMatrix, invBmatrix, diffMatrix); 

  vtkSmartPointer<vtkTransform> diffTransform = vtkSmartPointer<vtkTransform>::New(); 
  diffTransform->SetMatrix(diffMatrix); 

  double angleDiff_rad= vtkMath::RadiansFromDegrees(diffTransform->GetOrientationWXYZ()[0]);

  double normalizedAngleDiff_rad = atan2( sin(angleDiff_rad), cos(angleDiff_rad) ); // normalize angle to domain -pi, pi 

  return vtkMath::DegreesFromRadians(normalizedAngleDiff_rad);
}

//---------------------------------------------------------------------------
double vtkSlicerPivotCalibrationLogic::GetMaximumToolOrientationDifferenceDeg()
{
  // this will store the maximum difference in orientation between the first transform and all the other transforms
  double maximumOrientationDifferenceDeg = 0;
  
    std::vector<vtkMatrix4x4*>::const_iterator it;
  std::vector<vtkMatrix4x4*>::const_iterator matricesEnd = this->ToolToReferenceMatrices.end();
  unsigned int currentRow;
  vtkMatrix4x4* referenceOrientationMatrix = this->ToolToReferenceMatrices.front();
  for(currentRow = 0, it = this->ToolToReferenceMatrices.begin(); it != matricesEnd; it++, currentRow += 3)
  {
    double orientationDifferenceDeg = GetOrientationDifferenceDeg(referenceOrientationMatrix, (*it));
    if (maximumOrientationDifferenceDeg < orientationDifferenceDeg)
    {
      maximumOrientationDifferenceDeg = orientationDifferenceDeg;    
    }
  }

  return maximumOrientationDifferenceDeg;
}

//---------------------------------------------------------------------------
bool vtkSlicerPivotCalibrationLogic::ComputePivotCalibration( bool autoOrient /*=true*/)
{
  if (this->ToolToReferenceMatrices.size() < 10)
  {
    this->ErrorText = "Not enough input transforms are available";
    return false;
  }

  if (this->GetMaximumToolOrientationDifferenceDeg() < this->MinimumOrientationDifferenceDeg)
  {
    this->ErrorText = "Not enough variation in the input transforms";
    return false;
  }
  
  unsigned int rows = 3 * this->ToolToReferenceMatrices.size();
  unsigned int columns = 6;

  vnl_matrix<double> A(rows, columns);

  vnl_matrix<double> minusI(3,3,0);
  minusI(0, 0) = -1;
  minusI(1, 1) = -1;
  minusI(2, 2) = -1;

  vnl_matrix<double> R(3,3);
  vnl_vector<double> b(rows);
  vnl_vector<double> x(columns);
  vnl_vector<double> t(3);

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
  if (autoOrient)
  {
    this->UpdateShaftDirection(); // Flip it if necessary
  }

  this->ErrorText.empty();
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerPivotCalibrationLogic::ComputeSpinCalibration( bool snapRotation /*=false*/, bool autoOrient /*=true*/)
{
  if ( this->ToolToReferenceMatrices.size() < 10 )
  {
    this->ErrorText = "Not enough input transforms are available";
    return false;
  }

  if (this->GetMaximumToolOrientationDifferenceDeg() < this->MinimumOrientationDifferenceDeg)
  {
    this->ErrorText = "Not enough variation in the input transforms";
    return false;
  }
  
  // Setup our system to find the axis of rotation
  unsigned int rows = 3, columns = 3;

  vnl_matrix<double> A( rows, columns, 0);

  vnl_matrix<double> I( 3, 3, 0 );
  I.set_identity();

  vnl_matrix<double> RI( rows, columns );

  std::vector< vtkMatrix4x4* >::const_iterator previt = this->ToolToReferenceMatrices.end();
  for(std::vector< vtkMatrix4x4* >::const_iterator it = this->ToolToReferenceMatrices.begin(); it != this->ToolToReferenceMatrices.end(); it++ )
  {
    if ( previt == this->ToolToReferenceMatrices.end() )
    {
      previt = it;
      continue; // No comparison to make for the first matrix
    }
    
    vtkSmartPointer< vtkMatrix4x4 > itinverse = vtkSmartPointer< vtkMatrix4x4 >::New();
    vtkMatrix4x4::Invert( (*it), itinverse );

    vtkSmartPointer< vtkMatrix4x4 > instRotation = vtkSmartPointer< vtkMatrix4x4 >::New();
    vtkMatrix4x4::Multiply4x4( itinverse, (*previt), instRotation );

    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++ )
      {
        RI(i, j) = instRotation->GetElement(i, j);
      }
    }

    RI = RI - I;
    A = A + RI.transpose() * RI;

    previt = it;
  }

  // Setup the axes
  vnl_vector<double> shaftAxis_Shaft( columns, columns, SHAFT_AXIS );
  vnl_vector<double> orthogonalAxis_Shaft( columns, columns, ORTHOGONAL_AXIS );
  vnl_vector<double> backupAxis_Shaft( columns, columns, BACKUP_AXIS );

  // Find the eigenvector associated with the smallest eigenvalue
  // This is the best axis of rotation over all instantaneous rotations
  vnl_matrix<double> eigenvectors( columns, columns, 0 );
  vnl_vector<double> eigenvalues( columns, 0 );
  vnl_symmetric_eigensystem_compute( A, eigenvectors, eigenvalues );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )
  vnl_vector<double> shaftAxis_ToolTip( columns, 0 );
  shaftAxis_ToolTip( 0 ) = eigenvectors( 0, 0 );
  shaftAxis_ToolTip( 1 ) = eigenvectors( 1, 0 );
  shaftAxis_ToolTip( 2 ) = eigenvectors( 2, 0 );
  shaftAxis_ToolTip.normalize();

  // Snap the direction vector to be exactly aligned with one of the coordinate axes
  // This is if the sensor is known to be parallel to one of the axis, just not which one
  if ( snapRotation )
  {
    int closestCoordinateAxis = element_product( shaftAxis_ToolTip, shaftAxis_ToolTip ).arg_max();
    shaftAxis_ToolTip.fill( 0 );
    shaftAxis_ToolTip.put( closestCoordinateAxis, 1 ); // Doesn't matter the direction, will be sorted out later
  }

  //set the RMSE
  this->SpinRMSE = sqrt( eigenvalues( 0 ) / this->ToolToReferenceMatrices.size() );
  // Note: This error is the RMS distance from the ideal axis of rotation to the axis of rotation for each instantaneous rotation
  // This RMS distance can be computed to an angle in the following way: angle = arccos( 1 - SpinRMSE^2 / 2 )
  // Here we elect to return the RMS distance because this is the quantity that was actually minimized in the calculation


  // If the secondary axis 1 is parallel to the shaft axis in the tooltip frame, then use secondary axis 2
  vnl_vector<double> orthogonalAxis_ToolTip = this->ComputeSecondaryAxis( shaftAxis_ToolTip );
  // Do the registration find the appropriate rotation
  orthogonalAxis_ToolTip = orthogonalAxis_ToolTip - dot_product( orthogonalAxis_ToolTip, shaftAxis_ToolTip ) * shaftAxis_ToolTip;
  orthogonalAxis_ToolTip.normalize();

  // Register X,Y,O points in the two coordinate frames (only spherical registration - since pure rotation)
  vnl_matrix<double> ToolTipPoints( 3, 3, 0.0 );
  vnl_matrix<double> ShaftPoints( 3, 3, 0.0 );

  ToolTipPoints.put( 0, 0, shaftAxis_ToolTip( 0 ) );
  ToolTipPoints.put( 0, 1, shaftAxis_ToolTip( 1 ) );
  ToolTipPoints.put( 0, 2, shaftAxis_ToolTip( 2 ) );
  ToolTipPoints.put( 1, 0, orthogonalAxis_ToolTip( 0 ) );
  ToolTipPoints.put( 1, 1, orthogonalAxis_ToolTip( 1 ) );
  ToolTipPoints.put( 1, 2, orthogonalAxis_ToolTip( 2 ) );
  ToolTipPoints.put( 2, 0, 0 );
  ToolTipPoints.put( 2, 1, 0 );
  ToolTipPoints.put( 2, 2, 0 );

  ShaftPoints.put( 0, 0, shaftAxis_Shaft( 0 ) );
  ShaftPoints.put( 0, 1, shaftAxis_Shaft( 1 ) );
  ShaftPoints.put( 0, 2, shaftAxis_Shaft( 2 ) );
  ShaftPoints.put( 1, 0, orthogonalAxis_Shaft( 0 ) );
  ShaftPoints.put( 1, 1, orthogonalAxis_Shaft( 1 ) );
  ShaftPoints.put( 1, 2, orthogonalAxis_Shaft( 2 ) );
  ShaftPoints.put( 2, 0, 0 );
  ShaftPoints.put( 2, 1, 0 );
  ShaftPoints.put( 2, 2, 0 );
  
  vnl_svd<double> ShaftToToolTipRegistrator( ShaftPoints.transpose() * ToolTipPoints );
  vnl_matrix<double> V = ShaftToToolTipRegistrator.V();
  vnl_matrix<double> U = ShaftToToolTipRegistrator.U();
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

  // Set the elements of the output matrix
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++ )
    {
      this->ToolTipToToolMatrix->SetElement( i, j, Rotation[ i ][ j ] );
    }
  }
  if (autoOrient)
  {
    this->UpdateShaftDirection(); // Flip it if necessary
  }
  
  this->ErrorText.empty();
  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::GetToolTipToToolTranslation( vtkMatrix4x4* translationMatrix )
{
  translationMatrix->Identity();

  translationMatrix->SetElement( 0, 3, this->ToolTipToToolMatrix->GetElement( 0, 3 ) );
  translationMatrix->SetElement( 1, 3, this->ToolTipToToolMatrix->GetElement( 1, 3 ) );
  translationMatrix->SetElement( 2, 3, this->ToolTipToToolMatrix->GetElement( 2, 3 ) );
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::GetToolTipToToolRotation( vtkMatrix4x4* rotationMatrix )
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

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::GetToolTipToToolMatrix( vtkMatrix4x4* matrix )
{
  matrix->DeepCopy( this->ToolTipToToolMatrix );
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetToolTipToToolMatrix( vtkMatrix4x4* matrix )
{
  this->ToolTipToToolMatrix->DeepCopy( matrix );
}

//---------------------------------------------------------------------------
vnl_vector< double > vtkSlicerPivotCalibrationLogic::ComputeSecondaryAxis( vnl_vector< double > shaftAxis_ToolTip )
{
  // If the secondary axis 1 is parallel to the shaft axis in the tooltip frame, then use secondary axis 2
  vnl_vector< double > orthogonalAxis_Shaft( 3, 3, ORTHOGONAL_AXIS );
  double angle = acos( dot_product( shaftAxis_ToolTip, orthogonalAxis_Shaft ) );
  // Force angle to be between -pi/2 and +pi/2
  if ( angle > vtkMath::Pi() / 2 )
  {
    angle -= vtkMath::Pi();
  }
  if ( angle < - vtkMath::Pi() / 2 )
  {
    angle += vtkMath::Pi();
  }

  if ( fabs( angle ) < vtkMath::RadiansFromDegrees( PARALLEL_ANGLE_THRESHOLD_DEGREES ) ) // If shaft axis and orthogonal axis are not parallel
  {
    return vnl_vector< double >( 3, 3, BACKUP_AXIS );
  }
  return orthogonalAxis_Shaft;
}


//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::UpdateShaftDirection()
{
  // We need to verify that the ToolTipToTool vector in the Shaft coordinate system is in the opposite direction of the shaft
  vtkSmartPointer< vtkMatrix4x4 > rotationMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  this->GetToolTipToToolRotation( rotationMatrix );
  rotationMatrix->Invert();

  double toolTipToToolTranslation_ToolTip[ 4 ] = { 0, 0, 0, 0 }; // This is a vector, not a point, so the last element is 0
  toolTipToToolTranslation_ToolTip[ 0 ] = this->ToolTipToToolMatrix->GetElement( 0, 3 );
  toolTipToToolTranslation_ToolTip[ 1 ] = this->ToolTipToToolMatrix->GetElement( 1, 3 );
  toolTipToToolTranslation_ToolTip[ 2 ] = this->ToolTipToToolMatrix->GetElement( 2, 3 );

  double toolTipToToolTranslation_Shaft[ 4 ] = { 0, 0, 0, 0 }; // This is a vector, not a point, so the last element is 0
  rotationMatrix->MultiplyPoint( toolTipToToolTranslation_ToolTip, toolTipToToolTranslation_Shaft );
  double toolTipToToolTranslation3_Shaft[ 3 ] = { toolTipToToolTranslation_Shaft[ 0 ], toolTipToToolTranslation_Shaft[ 1 ], toolTipToToolTranslation_Shaft[ 2 ] };
  
  // Check if it is parallel or opposite to shaft direction
  if ( vtkMath::Dot( SHAFT_AXIS, toolTipToToolTranslation3_Shaft ) > 0 )
  {
    this->FlipShaftDirection();
  }

}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::FlipShaftDirection()
{
  // Need to rotate around the orthogonal axis
  vtkSmartPointer< vtkMatrix4x4 > rotationMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  this->GetToolTipToToolRotation( rotationMatrix );

  double shaftAxis_Shaft[ 4 ] = { SHAFT_AXIS[ 0 ], SHAFT_AXIS[ 1 ], SHAFT_AXIS[ 2 ], 0 }; // This is a vector, not a point, so the last element is 0
  double shaftAxis_ToolTip[ 4 ] = { 0, 0, 0, 0 };
  rotationMatrix->MultiplyPoint( shaftAxis_Shaft, shaftAxis_ToolTip );

  vnl_vector< double > orthogonalAxis_Shaft = this->ComputeSecondaryAxis( vnl_vector< double >( 3, 3, shaftAxis_ToolTip ) );

  vtkSmartPointer< vtkTransform > flipTransform = vtkSmartPointer< vtkTransform >::New();
  flipTransform->RotateWXYZ( 180, orthogonalAxis_Shaft.get( 0 ), orthogonalAxis_Shaft.get( 1 ), orthogonalAxis_Shaft.get( 2 ) );
  vtkSmartPointer< vtkTransform > originalTransform = vtkSmartPointer< vtkTransform >::New();
  originalTransform->SetMatrix( this->ToolTipToToolMatrix );
  originalTransform->PreMultiply();
  originalTransform->Concatenate( flipTransform );
  originalTransform->GetMatrix( this->ToolTipToToolMatrix );
}
