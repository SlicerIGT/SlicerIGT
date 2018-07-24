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
  this->MaximumDistance = VTK_DOUBLE_MIN;
  this->MinimumDistance = VTK_DOUBLE_MAX;
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

  if ( this->DistanceMatrix->GetNumberOfTuples() == 0 )
  {
    vtkWarningMacro( "Matrix has no contents. Returning 0." )
    return 0.0;
  }

  int tupleIndex = pointList1Index;
  int numberOfTuples = this->DistanceMatrix->GetNumberOfTuples();
  if ( tupleIndex < 0 || tupleIndex >= numberOfTuples )
  {
    vtkWarningMacro( "Point index of first list " << tupleIndex << " is outside the range 0 to " << ( numberOfTuples - 1 ) << ". Returning 0." )
    return 0.0;
  }

  int componentIndex = pointList2Index;
  int numberOfComponents = this->DistanceMatrix->GetNumberOfComponents();
  if ( componentIndex < 0 || componentIndex >= numberOfComponents )
  {
    vtkWarningMacro( "Point index of secondList list " << componentIndex << " is outside the range 0 to " << ( numberOfComponents - 1 ) << ". Returning 0." )
    return 0.0;
  }

  return this->DistanceMatrix->GetComponent( tupleIndex, componentIndex );
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::GetDistances( vtkDoubleArray* outputArray )
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  if ( outputArray == NULL )
  {
    vtkWarningMacro( "Output distances array is null." );
    return;
  }

  if ( this->DistanceMatrix->GetNumberOfTuples() == 0 )
  {
    vtkWarningMacro( "Matrix has no contents." )
    return;
  }

  outputArray->Reset();
  int numberOfTuples = this->DistanceMatrix->GetNumberOfTuples(); // aka point list 1 index
  int numberOfComponents = this->DistanceMatrix->GetNumberOfComponents(); // aka point list 2 index
  for ( int tupleIndex = 0; tupleIndex < numberOfTuples; tupleIndex++ )
  {
    for ( int componentIndex = 0; componentIndex < numberOfComponents; componentIndex++ )
    {
      double currentDistance = this->DistanceMatrix->GetComponent( tupleIndex, componentIndex );
      outputArray->InsertNextTuple1( currentDistance );
    }
  }
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList1( vtkPoints* points )
{
  this->PointList1 = points;
  this->ResetDistances();
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::SetPointList2( vtkPoints* points )
{
  this->PointList2 = points ;
  this->ResetDistances();
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::ResetDistances()
{
  this->DistanceMatrix->Reset();
  this->MaximumDistance = VTK_DOUBLE_MIN;
  this->MinimumDistance = VTK_DOUBLE_MAX;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::Update()
{
  if ( this->InputsContainErrors() )
  {
    vtkWarningMacro( "Cannot update." )
    return;
  }

  int pointList2Length = this->PointList2->GetNumberOfPoints();
  this->DistanceMatrix->SetNumberOfComponents( pointList2Length );
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
      if ( distance > this->MaximumDistance )
      {
        this->MaximumDistance = distance;
      }
      if ( distance < this->MinimumDistance )
      {
        this->MinimumDistance = distance;
      }
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
bool vtkPointDistanceMatrix::InputsContainErrors( bool verbose )
{
  if ( this->PointList1 == NULL )
  {
    if ( verbose )
    {
      vtkWarningMacro( "Point list 1 is null." );
    }
    return true;
  }

  int numberOfPointsInList1 = this->PointList1->GetNumberOfPoints();
  if ( numberOfPointsInList1 == 0 )
  {
    if ( verbose )
    {
      vtkWarningMacro( "There are no points in list 1.");
    }
    return true;
  }

  if ( this->PointList2 == NULL )
  {
    if ( verbose )
    {
      vtkWarningMacro( "Point list 2 is null." );
    }
    return true;
  }

  int numberOfPointsInList2 = this->PointList2->GetNumberOfPoints();
  if ( numberOfPointsInList2 == 0 )
  {
    if ( verbose )
    {
      vtkWarningMacro( "There are no points in list 2.");
    }
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
void vtkPointDistanceMatrix::PrintSelf(ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Point list 1 length: " << ( ( this->PointList1 != NULL ) ? this->PointList1->GetNumberOfPoints() : 0 ) << endl;
  os << indent << "Point list 2 length: " << ( ( this->PointList2 != NULL ) ? this->PointList2->GetNumberOfPoints() : 0 ) << endl;
}
