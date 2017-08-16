#include "vtkPointDistanceMatrix.h"

#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h> //for new() macro

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkPointDistanceMatrix );

//------------------------------------------------------------------------------
vtkPointDistanceMatrix::vtkPointDistanceMatrix()
{
  pointList1 = vtkSmartPointer< vtkPoints >::New();
  pointList2 = vtkSmartPointer< vtkPoints >::New();
  distanceMatrix = vtkSmartPointer< vtkDoubleArray >::New();
  UpdateNeededFlag = 0;
}

//------------------------------------------------------------------------------
vtkPointDistanceMatrix::~vtkPointDistanceMatrix()
{
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList1( vtkPoints* points )
{
  if ( points == NULL )
  {
    vtkWarningMacro( "Input point list is null. Aborting set operation." );
    return;
  }

  pointList1->DeepCopy( points );
  UpdateNeededFlag = 1;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList2( vtkPoints* points )
{
  if ( points == NULL )
  {
    vtkWarningMacro( "Input point list is null. Aborting set operation." );
    return;
  }

  pointList2->DeepCopy( points );
  UpdateNeededFlag = 1;
}

//------------------------------------------------------------------------------
int vtkPointDistanceMatrix::GetPointList1Length()
{
  return pointList1->GetNumberOfPoints();
}

//------------------------------------------------------------------------------
int vtkPointDistanceMatrix::GetPointList2Length()
{
  return pointList2->GetNumberOfPoints();
}

//------------------------------------------------------------------------------
double vtkPointDistanceMatrix::GetElement( int pointList1Index, int pointList2Index )
{
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

  if ( UpdateNeededFlag  )
  {
    Update();
  }

  return distanceMatrix->GetComponent( pointList1Index, pointList2Index );
}

//------------------------------------------------------------------------------
double vtkPointDistanceMatrix::GetMinimumElement()
{
  if ( distanceMatrix->GetNumberOfTuples() == 0 )
  {
    vtkGenericWarningMacro( "Matrix has no contents. Returning 0." )
    return 0;
  }

  int pointList1Length = GetPointList1Length();
  int pointList2Length = GetPointList2Length();
  double minElement = GetElement( 0, 0 );
  for ( int pointList1Index = 0; pointList1Index < pointList1Length; pointList1Index++ )
  {
    for ( int pointList2Index = 0; pointList2Index < pointList2Length; pointList2Index++ )
    {
      double currentElement = GetElement( pointList1Index, pointList2Index );
      if ( currentElement < minElement )
      {
        minElement = currentElement;
      }
    }
  }
  return minElement;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::Update()
{
  int pointList2Length = GetPointList2Length();
  distanceMatrix->SetNumberOfComponents( pointList2Length );
  int pointList1Length = GetPointList1Length();
  distanceMatrix->SetNumberOfTuples( pointList1Length );
  for ( int pointList1Index = 0; pointList1Index < pointList1Length; pointList1Index++ )
  {
    for ( int pointList2Index = 0; pointList2Index < pointList2Length; pointList2Index++ )
    {
      double pointInList1[ 3 ] = { 0, 0, 0 }; // temporary values
      pointList1->GetPoint( pointList1Index, pointInList1 );
      double pointInList2[ 3 ] = { 0, 0, 0 }; // temporary values
      pointList2->GetPoint( pointList2Index, pointInList2 );
      double pointDifference[ 3 ] = { 0, 0, 0 }; // temporary values
      vtkMath::Subtract( pointInList1, pointInList2, pointDifference );
      double distance = vtkMath::Norm( pointDifference );
      distanceMatrix->SetComponent( pointList1Index, pointList2Index, distance );
    }
  }

  UpdateNeededFlag = 0;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::ComputeElementWiseDifference( vtkPointDistanceMatrix* matrix1, vtkPointDistanceMatrix* matrix2, vtkDoubleArray* outputArray )
{
  if ( matrix1 == NULL )
  {
    vtkGenericWarningMacro( "Input first matrix is null. Cannot compute element-wise differences." );
    return;
  }

  if ( matrix2 == NULL )
  {
    vtkGenericWarningMacro( "Input second matrix is null. Cannot compute element-wise differences." );
    return;
  }

  if ( matrix1->GetPointList1Length() != matrix2->GetPointList1Length() )
  {
    vtkGenericWarningMacro( "Input matrices have different numbers of points for first list. Cannot compute element-wise differences." );
    return;
  }

  if ( matrix1->GetPointList2Length() != matrix2->GetPointList2Length() )
  {
    vtkGenericWarningMacro( "Input matrices have different numbers of points for second list. Cannot compute element-wise differences." );
    return;
  }

  if ( outputArray == NULL )
  {
    vtkGenericWarningMacro( "Output matrix is null. Cannot compute element-wise differences." );
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
      double elementFromMatrix1 = matrix1->GetElement( pointList1Index, pointList2Index );
      double elementFromMatrix2 = matrix2->GetElement( pointList1Index, pointList2Index );
      double differenceOfElements = elementFromMatrix2 - elementFromMatrix1;
      outputArray->SetComponent( pointList1Index, pointList2Index, differenceOfElements );
    }
  }
}

void vtkPointDistanceMatrix::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Point list 1: " << pointList1 << endl;
  os << indent << "Point list 2: " << pointList2 << endl;
}
