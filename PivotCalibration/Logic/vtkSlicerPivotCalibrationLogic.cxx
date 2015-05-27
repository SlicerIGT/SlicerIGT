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
  this->ObservedTransformNode = NULL;
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
    if ( event = vtkMRMLLinearTransformNode::TransformModifiedEvent && this->RecordingState == true && strcmp( transformNode->GetID(), this->ObservedTransformNode->GetID() ) == 0 )
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
void vtkSlicerPivotCalibrationLogic::SetAndObserveTransformNode( vtkMRMLLinearTransformNode* transformNode )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLLinearTransformNode::TransformModifiedEvent );
  vtkSetAndObserveMRMLNodeEventsMacro( this->ObservedTransformNode, transformNode, events.GetPointer() );
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

}


//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ComputeSpinCalibration( bool snapRotation )
{

  if ( this->ToolToReferenceMatrices.size() == 0 )
  {
    return;
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

  // Find the eigenvector associated with the smallest eigenvalue
  // This is the best axis of rotation over all instantaneous rotations
  vnl_matrix<double> eigenvectors( columns, columns, 0 );
  vnl_vector<double> eigenvalues( columns, 0 );
  vnl_symmetric_eigensystem_compute( A, eigenvectors, eigenvalues );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )
  vnl_vector<double> x( columns, 0 );
  x( 0 ) = eigenvectors( 0, 0 );
  x( 1 ) = eigenvectors( 1, 0 );
  x( 2 ) = eigenvectors( 2, 0 );
  x.normalize();

  // Snap the direction vector to be exactly aligned with one of the coordinate axes
  // This is if the sensor is known to be parallel to one of the axis, just not which one
  if ( snapRotation )
  {
    int axis = element_product( x, x ).arg_max();
    x.fill( 0 );
    x.put( axis, 1 ); // Doesn't matter the direction, will be sorted out in the next step
  }

  // Make sure it is in the correct direction (opposite the StylusTipToStylus translation)
  vnl_vector<double> toolTipToToolTranslation( 3 );
  toolTipToToolTranslation( 0 ) = this->ToolTipToToolMatrix->GetElement( 0, 3 );
  toolTipToToolTranslation( 1 ) = this->ToolTipToToolMatrix->GetElement( 1, 3 );
  toolTipToToolTranslation( 2 ) = this->ToolTipToToolMatrix->GetElement( 2, 3 );
  if ( dot_product( x, toolTipToToolTranslation ) > 0 )
  {
    x = x * ( -1 );
  }

  //set the RMSE
  this->SpinRMSE = ( A * x ).rms();


  // Do the registration find the appropriate rotation
  vnl_vector<double> y( 3, 0.0 );
  y.put( 1, 1 ); // Put the y part in
  y = y - dot_product( y, x ) * x;
  y.normalize();

  // Register X,Y,O points in the two coordinate frames (only spherical registration - since pure rotation)
  vnl_matrix<double> ToolTipPoints( 3, 3, 0.0 );
  vnl_matrix<double> XShaftPoints( 3, 3, 0.0 );

  ToolTipPoints.put( 0, 0, x.get( 0 ) );
  ToolTipPoints.put( 0, 1, x.get( 1 ) );
  ToolTipPoints.put( 0, 2, x.get( 2 ) );
  ToolTipPoints.put( 1, 0, y.get( 0 ) );
  ToolTipPoints.put( 1, 1, y.get( 1 ) );
  ToolTipPoints.put( 1, 2, y.get( 2 ) );
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

  // Set the elements of the output matrix
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++ )
    {
      this->ToolTipToToolMatrix->SetElement( i, j, Rotation[ i ][ j ] );
    }
  }

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

