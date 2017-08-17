#include "vtkPointDistanceMatrix.h"

#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h> //for vtkStandardNewMacro() macro

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkPointDistanceMatrix );

//------------------------------------------------------------------------------
vtkPointDistanceMatrix::vtkPointDistanceMatrix()
{
  this->PointList1 = vtkSmartPointer< vtkPoints >::New();
  this->PointList2 = vtkSmartPointer< vtkPoints >::New();
  this->DistanceMatrix = vtkSmartPointer< vtkDoubleArray >::New();
}

//------------------------------------------------------------------------------
vtkPointDistanceMatrix::~vtkPointDistanceMatrix()
{
}

//------------------------------------------------------------------------------
int vtkPointDistanceMatrix::GetPointList1Length()
{
  return this->PointList1->GetNumberOfPoints();
}

//------------------------------------------------------------------------------
int vtkPointDistanceMatrix::GetPointList2Length()
{
  return this->PointList2->GetNumberOfPoints();
}

//------------------------------------------------------------------------------
double vtkPointDistanceMatrix::GetDistance( int pointList1Index, int pointList2Index )
{
  if ( UpdateNeeded()  )
  {
    Update();
  }

  int pointList1Length = GetPointList1Length();
  if ( pointList1Index >= pointList1Length )
  {
    vtkWarningMacro( "Point of list 1 is outside the range. Returning 0." )
    return 0;
  }

  int pointList2Length = GetPointList2Length();
  if ( pointList2Index >= pointList2Length )
  {
    vtkWarningMacro( "Point of list 2 is outside the range. Returning 0." )
    return 0;
  }

  return this->DistanceMatrix->GetComponent( pointList1Index, pointList2Index );
}

//------------------------------------------------------------------------------
double vtkPointDistanceMatrix::GetMinimumDistance()
{
  if ( UpdateNeeded()  )
  {
    Update();
  }
  
  if ( this->DistanceMatrix->GetNumberOfTuples() == 0 )
  {
    vtkGenericWarningMacro( "Matrix has no contents. Returning 0." )
    return 0;
  }

  int pointList1Length = GetPointList1Length();
  int pointList2Length = GetPointList2Length();
  double minDistance = GetDistance( 0, 0 );
  for ( int pointList1Index = 0; pointList1Index < pointList1Length; pointList1Index++ )
  {
    for ( int pointList2Index = 0; pointList2Index < pointList2Length; pointList2Index++ )
    {
      double currentDistance = GetDistance( pointList1Index, pointList2Index );
      if ( currentDistance < minDistance )
      {
        minDistance = currentDistance;
      }
    }
  }
  return minDistance;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList1( vtkPoints* points )
{
  if ( points == NULL )
  {
    vtkWarningMacro( "Input point list is null. Aborting set operation." );
    return;
  }

  this->PointList1->DeepCopy( points );
  
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList2( vtkPoints* points )
{
  if ( points == NULL )
  {
    vtkWarningMacro( "Input point list is null. Aborting set operation." );
    return;
  }

  this->PointList2->DeepCopy( points );

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::Update()
{
  int pointList2Length = GetPointList2Length();
  this->DistanceMatrix->SetNumberOfComponents( pointList2Length );
  int pointList1Length = GetPointList1Length();
  this->DistanceMatrix->SetNumberOfTuples( pointList1Length );
  for ( int pointList1Index = 0; pointList1Index < pointList1Length; pointList1Index++ )
  {
    for ( int pointList2Index = 0; pointList2Index < pointList2Length; pointList2Index++ )
    {
      double* pointInList1 = this->PointList1->GetPoint( pointList1Index );
      double* pointInList2 = this->PointList2->GetPoint( pointList2Index );
      double distanceSquared = vtkMath::Distance2BetweenPoints( pointInList1, pointInList2 );
      double distance = sqrt( distanceSquared );
      this->DistanceMatrix->SetComponent( pointList1Index, pointList2Index, distance );
    }
  }

  this->Modified();
  this->MatrixUpdateTime.Modified();
}

//------------------------------------------------------------------------------
bool vtkPointDistanceMatrix::UpdateNeeded()
{
  bool thisModified = this->MatrixUpdateTime < this->GetMTime();
  bool pointList1Modified = this->MatrixUpdateTime < this->PointList1->GetMTime();
  bool pointList2Modified = this->MatrixUpdateTime < this->PointList2->GetMTime();
  return ( thisModified || pointList1Modified || pointList2Modified );
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::ComputePairWiseDifferences( vtkPointDistanceMatrix* matrix1, vtkPointDistanceMatrix* matrix2, vtkDoubleArray* outputArray )
{
  if ( matrix1 == NULL )
  {
    vtkGenericWarningMacro( "Input first matrix is null. Cannot compute pair-wise differences." );
    return;
  }

  if ( matrix2 == NULL )
  {
    vtkGenericWarningMacro( "Input second matrix is null. Cannot compute pair-wise differences." );
    return;
  }

  if ( matrix1->GetPointList1Length() != matrix2->GetPointList1Length() )
  {
    vtkGenericWarningMacro( "Input matrices have different numbers of points for first list. Cannot compute pair-wise differences." );
    return;
  }

  if ( matrix1->GetPointList2Length() != matrix2->GetPointList2Length() )
  {
    vtkGenericWarningMacro( "Input matrices have different numbers of points for second list. Cannot compute pair-wise differences." );
    return;
  }

  if ( outputArray == NULL )
  {
    vtkGenericWarningMacro( "Output matrix is null. Cannot compute pair-wise differences." );
    return;
  }

  if ( outputArray->GetNumberOfTuples() > 0 )
  {
    vtkGenericWarningMacro( "Output array is not empty. Emptying contents" );
    outputArray->Reset();
  }

  int pointList2Length = matrix1->GetPointList2Length();
  outputArray->SetNumberOfComponents( pointList2Length );
  int pointList1Length = matrix1->GetPointList1Length();
  outputArray->SetNumberOfTuples( pointList1Length );

  for ( int pointList1Index = 0; pointList1Index < pointList1Length; pointList1Index++ )
  {
    for ( int pointList2Index = 0; pointList2Index < pointList2Length; pointList2Index++ )
    {
      double distanceFromMatrix1 = matrix1->GetDistance( pointList1Index, pointList2Index );
      double distanceFromMatrix2 = matrix2->GetDistance( pointList1Index, pointList2Index );
      double differenceOfDistances = distanceFromMatrix2 - distanceFromMatrix1;
      outputArray->SetComponent( pointList1Index, pointList2Index, differenceOfDistances );
    }
  }
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Point list 1: " << this->PointList1 << endl;
  os << indent << "Point list 2: " << this->PointList2 << endl;
}
