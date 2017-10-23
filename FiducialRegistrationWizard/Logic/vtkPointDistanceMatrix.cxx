#include "vtkPointDistanceMatrix.h"

#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h> //for vtkStandardNewMacro() macro

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkPointDistanceMatrix );

//------------------------------------------------------------------------------
vtkPointDistanceMatrix::vtkPointDistanceMatrix()
{
  this->PointList1 = NULL;
  this->PointList2 = NULL;
  this->DistanceMatrix = vtkSmartPointer< vtkDoubleArray >::New();
}

//------------------------------------------------------------------------------
vtkPointDistanceMatrix::~vtkPointDistanceMatrix()
{
}

//------------------------------------------------------------------------------
vtkPoints* vtkPointDistanceMatrix::GetPointList1()
{
  return this->PointList1;
}

//------------------------------------------------------------------------------
vtkPoints* vtkPointDistanceMatrix::GetPointList2()
{
  return this->PointList2;
}

//------------------------------------------------------------------------------
double vtkPointDistanceMatrix::GetDistance( int pointList1Index, int pointList2Index )
{
  if ( UpdateNeeded()  )
  {
    Update();
  }

  if ( this->PointList1 == NULL )
  {
    vtkWarningMacro( "Point list 1 is null. Returning 0." )
    return 0.0;
  }

  int pointList1Length = this->PointList1->GetNumberOfPoints();
  if ( pointList1Index >= pointList1Length )
  {
    vtkWarningMacro( "Point of list 1 is outside the range. Returning 0." )
    return 0.0;
  }
  
  if ( this->PointList2 == NULL )
  {
    vtkWarningMacro( "Point list 2 is null. Returning 0." )
    return 0.0;
  }

  int pointList2Length = this->PointList2->GetNumberOfPoints();
  if ( pointList2Index >= pointList2Length )
  {
    vtkWarningMacro( "Point of list 2 is outside the range. Returning 0." )
    return 0.0;
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

  if ( this->PointList1 == NULL )
  {
    vtkWarningMacro( "Point list 1 is null. Returning 0." )
    return 0.0;
  }
  int pointList1Length = this->PointList1->GetNumberOfPoints();

  if ( this->PointList2 == NULL )
  {
    vtkWarningMacro( "Point list 2 is null. Returning 0." )
    return 0.0;
  }
  int pointList2Length = this->PointList2->GetNumberOfPoints();
  
  if ( this->DistanceMatrix->GetNumberOfTuples() == 0 )
  {
    vtkGenericWarningMacro( "Matrix has no contents. Returning 0." )
    return 0.0;
  }

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
  this->PointList1 = points;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList2( vtkPoints* points )
{
  this->PointList2 = points ;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::Update()
{
  
  if ( this->PointList2 == NULL )
  {
    vtkWarningMacro( "Point list 2 is null. Cannot update." )
    return;
  }
  int pointList2Length = this->PointList2->GetNumberOfPoints();
  this->DistanceMatrix->SetNumberOfComponents( pointList2Length );
  // (number of components must be set before number of tuples)

  if ( this->PointList1 == NULL )
  {
    vtkWarningMacro( "Point list 1 is null. Cannot update." )
    return;
  }
  int pointList1Length = this->PointList1->GetNumberOfPoints();
  this->DistanceMatrix->SetNumberOfTuples( pointList1Length );

  for ( int pointList1Index = 0; pointList1Index < pointList1Length; pointList1Index++ )
  {
    for ( int pointList2Index = 0; pointList2Index < pointList2Length; pointList2Index++ )
    {
      double pointInList1[ 3 ];
      this->PointList1->GetPoint( pointList1Index, pointInList1 );
      double pointInList2[ 3 ];
      this->PointList2->GetPoint( pointList2Index, pointInList2 );
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
  if ( this->PointList1 == NULL || this->PointList2 == NULL )
  {
    vtkWarningMacro( "At least one point list is null. Reporting that update is necessary." );
    // this is an error case, but we have to return something...
    return true;
  }

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
    vtkGenericWarningMacro( "Input first matrix is null. No differences computed." );
    return;
  }
  
  vtkPoints* matrix1PointList1 = matrix1->GetPointList1();
  if ( matrix1PointList1 == NULL )
  {
    vtkGenericWarningMacro( "Point list 1 from matrix 1 is null. No differences computed." );
    return;
  }

  vtkPoints* matrix1PointList2 = matrix1->GetPointList2();
  if ( matrix1PointList2 == NULL )
  {
    vtkGenericWarningMacro( "Point list 2 from matrix 1 is null. No differences computed." );
    return;
  }

  if ( matrix2 == NULL )
  {
    vtkGenericWarningMacro( "Input second matrix is null. No differences computed." );
    return;
  }

  vtkPoints* matrix2PointList1 = matrix2->GetPointList1();
  if ( matrix2PointList1 == NULL )
  {
    vtkGenericWarningMacro( "Point list 1 from matrix 2 is null. No differences computed." );
    return;
  }

  vtkPoints* matrix2PointList2 = matrix2->GetPointList2();
  if ( matrix2PointList2 == NULL )
  {
    vtkGenericWarningMacro( "Point list 2 from matrix 2 is null. No differences computed." );
    return;
  }

  if ( matrix1PointList1->GetNumberOfPoints() != matrix2PointList1->GetNumberOfPoints() )
  {
    vtkGenericWarningMacro( "Input matrices have different numbers of points for first list. Cannot compute pair-wise differences." );
    return;
  }

  if ( matrix1PointList2->GetNumberOfPoints() != matrix2PointList2->GetNumberOfPoints() )
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

  int pointList1Length = matrix1PointList1->GetNumberOfPoints();
  int pointList2Length = matrix1PointList2->GetNumberOfPoints();
  outputArray->SetNumberOfComponents( pointList2Length );
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

  os << indent << "Point list 1 length: " << ( ( this->PointList1 != NULL ) ? this->PointList1->GetNumberOfPoints() : 0 ) << endl;
  os << indent << "Point list 2 length: " << ( ( this->PointList2 != NULL ) ? this->PointList2->GetNumberOfPoints() : 0 ) << endl;
}
