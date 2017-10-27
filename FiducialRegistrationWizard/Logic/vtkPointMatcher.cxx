#include "vtkPointMatcher.h"
#include "vtkCombinatoricGenerator.h"
#include <vtkMath.h>

#define RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR VTK_DOUBLE_MAX
#define MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH 3

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkPointMatcher );

//------------------------------------------------------------------------------
vtkPointMatcher::vtkPointMatcher()
{
  this->InputPointList1 = NULL;
  this->InputPointList2 = NULL;
  this->MaximumDifferenceInNumberOfPoints = 2;
  this->TolerableRootMeanSquareDistanceErrorMm = 10.0;
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  this->AmbiguityThresholdDistanceMm = 5.0;
  this->MatchingAmbiguous = false;
  // outputs are never null
  this->OutputPointList1 = vtkSmartPointer< vtkPoints >::New();
  this->OutputPointList2 = vtkSmartPointer< vtkPoints >::New();

  // timestamps for input and output are the same, initially
  this->Modified();
  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
vtkPointMatcher::~vtkPointMatcher()
{
}

//------------------------------------------------------------------------------
void vtkPointMatcher::PrintSelf( std::ostream &os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
  
  os << indent << "MaximumDifferenceInNumberOfPoints: " << this->MaximumDifferenceInNumberOfPoints << std::endl;
  os << indent << "TolerableRootMeanSquareDistanceErrorMm: " << this->TolerableRootMeanSquareDistanceErrorMm << std::endl;
  os << indent << "ComputedRootMeanSquareDistanceErrorMm: " << this->ComputedRootMeanSquareDistanceErrorMm << std::endl;
  os << indent << "IsMatchingWithinTolerance: " << this->IsMatchingWithinTolerance() << std::endl;
  os << indent << "IsMatchingAmbiguous" << this->IsMatchingAmbiguous() << std::endl;
  os << indent << "UpdateNeeded: " << this->UpdateNeeded() << std::endl;
}

//------------------------------------------------------------------------------
// INPUT MUTATORS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkPointMatcher::SetInputPointList1( vtkPoints* points )
{
  vtkSetObjectBodyMacro( InputPointList1, vtkPoints, points );
  // mean distance error has not been computed yet. Set to maximum possible value for now
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
}

//------------------------------------------------------------------------------
void vtkPointMatcher::SetInputPointList2( vtkPoints* points )
{
  vtkSetObjectBodyMacro( InputPointList2, vtkPoints, points );
  // mean distance error has not been computed yet. Set to maximum possible value for now
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
}

//------------------------------------------------------------------------------
void vtkPointMatcher::SetMaximumDifferenceInNumberOfPoints( unsigned int numberOfPoints )
{
  this->MaximumDifferenceInNumberOfPoints = numberOfPoints;
  this->Modified();
  // mean distance error has not been computed yet. Set to maximum possible value for now
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
}

//------------------------------------------------------------------------------
void vtkPointMatcher::SetTolerableRootMeanSquareDistanceErrorMm( double errorMm )
{
  this->TolerableRootMeanSquareDistanceErrorMm = errorMm;
  this->Modified();
  // mean distance error has not been computed yet. Set to maximum possible value for now
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
}

//------------------------------------------------------------------------------
void vtkPointMatcher::SetAmbiguityThresholdDistanceMm( double thresholdMm )
{
  this->AmbiguityThresholdDistanceMm = thresholdMm;
  this->Modified();
  // mean distance error has not been computed yet. Set to maximum possible value for now
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
}

//------------------------------------------------------------------------------
// OUTPUT ACCESSORS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
vtkPoints* vtkPointMatcher::GetOutputPointList1()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->OutputPointList1;
}

//------------------------------------------------------------------------------
vtkPoints* vtkPointMatcher::GetOutputPointList2()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->OutputPointList2;
}

//------------------------------------------------------------------------------
double vtkPointMatcher::GetComputedRootMeanSquareDistanceErrorMm()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->ComputedRootMeanSquareDistanceErrorMm;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::IsMatchingWithinTolerance()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }
  
  return ( this->ComputedRootMeanSquareDistanceErrorMm <= this->TolerableRootMeanSquareDistanceErrorMm );
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::IsMatchingAmbiguous()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->MatchingAmbiguous;
}

//------------------------------------------------------------------------------
// LOGIC
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkPointMatcher::Update()
{
  if ( !this->UpdateNeeded() )
  {
    return;
  }
  
  // check number of points, make sure point lists are already similar/same sizes
  if ( this->InputPointList1 == NULL )
  {
    vtkWarningMacro( "Input point list 1 is null. Cannot update." );
    return;
  }
  int pointList1Size = this->InputPointList1->GetNumberOfPoints();

  if ( this->InputPointList2 == NULL )
  {
    vtkWarningMacro( "Input point list 2 is null. Cannot update." );
    return;
  }
  int pointList2Size = this->InputPointList2->GetNumberOfPoints();
  
  unsigned int differenceInPointListSizes = abs( pointList1Size - pointList2Size );
  if ( differenceInPointListSizes > this->MaximumDifferenceInNumberOfPoints )
  {
    // no matching to do... though there should be a more elegant way to handle this
    // situation that produces at least some kind of matching, even if it is a bad one.
    // TODO: Copy inputs to outputs, then truncate the longer one.
    this->OutputChangedTime.Modified();
    return;
  }

  // determine which list is larger, which list is smaller
  vtkPoints* largerPointsList = NULL; // temporary value
  vtkPoints* smallerPointsList = NULL; // temporary value
  if ( pointList1Size > pointList2Size )
  {
    largerPointsList = this->InputPointList1;
    smallerPointsList = this->InputPointList2;
  }
  else
  {
    largerPointsList = this->InputPointList2;
    smallerPointsList = this->InputPointList1;
  }
  int smallerPointListSize = smallerPointsList->GetNumberOfPoints();
  int largerPointListSize = largerPointsList->GetNumberOfPoints();

  // iterate through all valid numbers of points, based on the sizes
  // of the inputs, and the maximum point difference specified
  int minimumNumberOfPointsToMatch = vtkMath::Max( ( largerPointListSize - this->MaximumDifferenceInNumberOfPoints ), ( unsigned int )MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH );
  for ( int numberOfPoints = smallerPointListSize; numberOfPoints >= minimumNumberOfPointsToMatch; numberOfPoints-- )
  {
    this->UpdateBestMatchingForAllSubsetsOfPoints( numberOfPoints );
    if ( this->ComputedRootMeanSquareDistanceErrorMm <= this->TolerableRootMeanSquareDistanceErrorMm )
    {
      // suitable solution has been found, no need to continue searching
      break;
    }
  }

  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
void vtkPointMatcher::UpdateBestMatchingForAllSubsetsOfPoints( int sizeOfSubset )
{
  // generate sets of indices for all possible combinations of both input sets
  vtkSmartPointer< vtkCombinatoricGenerator > pointList1CombinationGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  pointList1CombinationGenerator->SetCombinatoricToCombination();
  pointList1CombinationGenerator->SetSubsetSize( sizeOfSubset );
  pointList1CombinationGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < this->InputPointList1->GetNumberOfPoints(); pointIndex++ )
  {
    pointList1CombinationGenerator->AddInputElement( 0, pointIndex );
  }
  pointList1CombinationGenerator->Update();
  std::vector< std::vector< int > > pointList1CombinationIndices = pointList1CombinationGenerator->GetOutputSets();

  vtkSmartPointer< vtkCombinatoricGenerator > pointList2CombinationGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  pointList2CombinationGenerator->SetCombinatoricToCombination();
  pointList2CombinationGenerator->SetSubsetSize( sizeOfSubset );
  pointList2CombinationGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < this->InputPointList2->GetNumberOfPoints(); pointIndex++ )
  {
    pointList2CombinationGenerator->AddInputElement( 0, pointIndex );
  }
  pointList2CombinationGenerator->Update();
  std::vector< std::vector< int > > pointList2CombinationIndices = pointList2CombinationGenerator->GetOutputSets();

  // these will store the actual combinations of points themselves (not indices)
  vtkSmartPointer< vtkPoints > pointList1Combination = vtkSmartPointer< vtkPoints >::New();
  pointList1Combination->SetNumberOfPoints( sizeOfSubset );
  vtkSmartPointer< vtkPoints > pointList2Combination = vtkSmartPointer< vtkPoints >::New();
  pointList2Combination->SetNumberOfPoints( sizeOfSubset );

  // iterate over all combinations of both input point sets
  for ( unsigned int pointList1CombinationIndex = 0; pointList1CombinationIndex < pointList1CombinationIndices.size(); pointList1CombinationIndex++ )
  {
    for ( unsigned int pointList2CombinationIndex = 0; pointList2CombinationIndex < pointList2CombinationIndices.size(); pointList2CombinationIndex++ )
    {
      // store appropriate contents in the pointList1Combination and pointList2Combination variables
      for ( vtkIdType pointIndex = 0; pointIndex < sizeOfSubset; pointIndex++ )
      {
        vtkIdType point1Index = ( vtkIdType ) pointList1CombinationIndices[ pointList1CombinationIndex ][ pointIndex ];
        double* pointFromList1 = this->InputPointList1->GetPoint( point1Index );
        pointList1Combination->SetPoint( pointIndex, pointFromList1 );

        vtkIdType point2Index = ( vtkIdType ) pointList2CombinationIndices[ pointList2CombinationIndex ][ pointIndex ];
        double* pointFromList2 = this->InputPointList2->GetPoint( point2Index );
        pointList2Combination->SetPoint( pointIndex, pointFromList2 );
      }
      // finally see how good this particular combination is
      this->UpdateBestMatchingForSubsetOfPoints( pointList1Combination, pointList2Combination );
    }
  }
}

//------------------------------------------------------------------------------
// point pair matching will be based on the distances between each pair of ordered points.
// we have an input reference point list and a compare point list. We want to reorder
// the compare list such that the point-to-point distances are as close as possible to those 
// in the reference list. We will permute over all possibilities (and only ever keep the best result.)
void vtkPointMatcher::UpdateBestMatchingForSubsetOfPoints( vtkPoints* pointSubset1, vtkPoints* pointSubset2 )
{
  // error checking
  if ( pointSubset1 == NULL )
  {
    vtkGenericWarningMacro( "Reference point set is NULL. This is a coding error. Please report." );
    return;
  }

  if ( pointSubset2 == NULL )
  {
    vtkGenericWarningMacro( "Compare point set is NULL. This is a coding error. Please report." );
    return;
  }

  int numberOfPoints = pointSubset1->GetNumberOfPoints(); // sizes of both lists should be identical
  if ( numberOfPoints != pointSubset2->GetNumberOfPoints() )
  {
    vtkGenericWarningMacro( "Point sets are of different sizes. This is a coding error. Please report." );
    return;
  }

  // Point distance matrix
  vtkSmartPointer< vtkPointDistanceMatrix > pointSubset1DistanceMatrix = vtkSmartPointer< vtkPointDistanceMatrix >::New();
  pointSubset1DistanceMatrix->SetPointList1( pointSubset1 );
  pointSubset1DistanceMatrix->SetPointList2( pointSubset1 ); // distances to itself
  pointSubset1DistanceMatrix->Update();

  // compute the permutations, store them in a vtkIntArray.
  vtkSmartPointer< vtkCombinatoricGenerator > combinatoricGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  combinatoricGenerator->SetCombinatoricToPermutation();
  combinatoricGenerator->SetSubsetSize( numberOfPoints );
  combinatoricGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++ )
  {
    combinatoricGenerator->AddInputElement( 0, pointIndex );
  }
  combinatoricGenerator->Update();
  std::vector< std::vector< int > > pointSubset2IndexPermutations = combinatoricGenerator->GetOutputSets();

  // iterate over all permutations - look for the most 'suitable'
  // point matching that gives distances most similar to the reference

  // create + allocate the permuted compare list once outside
  // the loop to avoid allocation/deallocation time costs.
  vtkSmartPointer< vtkPoints > permutedPointSubset2 = vtkSmartPointer< vtkPoints >::New();
  permutedPointSubset2->DeepCopy( pointSubset2 ); // fill it with placeholder data, same size as compareList
  vtkSmartPointer< vtkPointDistanceMatrix > permutedPointSubset2DistanceMatrix = vtkSmartPointer< vtkPointDistanceMatrix >::New();
  int numberOfPermutations = pointSubset2IndexPermutations.size();
  for ( int permutationIndex = 0; permutationIndex < numberOfPermutations; permutationIndex++ )
  {
    // fill permutedPointSubset2 with points from pointSubset2,
    // in the order indicate by the permuted indices
    for ( int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++ )
    {
      int permutedPointIndex = pointSubset2IndexPermutations[ permutationIndex][ pointIndex ];
      double* permutedPoint = pointSubset2->GetPoint( permutedPointIndex );
      permutedPointSubset2->SetPoint( pointIndex, permutedPoint );
    }
    permutedPointSubset2DistanceMatrix->SetPointList1( permutedPointSubset2 );
    permutedPointSubset2DistanceMatrix->SetPointList2( permutedPointSubset2 );
    permutedPointSubset2DistanceMatrix->Update();
    double rootMeanSquareDistanceErrorMm = this->ComputeRootMeanSquareistanceErrors( pointSubset1DistanceMatrix, permutedPointSubset2DistanceMatrix );

    // case analysis for setting MatchingAmbiguous:
    // let ComputedRootMeanSquareDistanceErrorMm store the distance error for the *best* matching
    // let rootMeanSquareDistanceErrorMm store the distance error for the *current* matching
    // use AmbiguityThresholdDistanceMm and MatchingAmbiguous as described in the header file
    // 1
    // rootMeanSquareDistanceErrorMm is better (lower) than ComputedRootMeanSquareDistanceErrorMm,
    // but by less than AmbiguityThresholdDistanceMm
    // Result => MatchingAmbiguous should be set to true
    //   - trivial justification
    // 2
    // rootMeanSquareDistanceErrorMm is worse (higher) than ComputedRootMeanSquareDistanceErrorMm,
    // but by less than AmbiguityThresholdDistanceMm
    // Result => MatchingAmbiguous should be set to true
    //   - trivial justification
    // 3
    // rootMeanSquareDistanceErrorMm is better (lower) than ComputedRootMeanSquareDistanceErrorMm,
    // but by more than AmbiguityThresholdDistanceMm
    // Result => MatchingAmbiguous should be set to false.
    //   - If there was a _previous_ rootMeanSquareDistanceErrorMm within AmbiguityThresholdDistanceMm,
    //     that would have become ComputedRootMeanSquareDistanceErrorMm. Therefore there have not been
    //     any _previous_ rootMeanSquareDistanceErrorMm within AmbiguityThresholdDistanceMm.
    //   - If _later_ there is a rootMeanSquareDistanceErrorMm within AmbiguityThresholdDistanceMm,
    //     then MatchingAmbiguous will be set to true by either case 1 or case 2.
    //     Cases 1 and 2 will always catch an ambiguous matching, because the search is exhaustive.
    // 4
    // rootMeanSquareDistanceErrorMm is worse (higher) than ComputedRootMeanSquareDistanceErrorMm,
    // but by more than AmbiguityThresholdDistanceMm
    // Result => do nothing
    //   - This result does not matter. It *cannot* be within AmbiguityThresholdDistanceMm
    //     of the best (final) ComputedRootMeanSquareDistanceErrorMm

    // flag ambiguous if suitability within some threshold of the best result so far
    double differenceComparedToBestMm = this->ComputedRootMeanSquareDistanceErrorMm - rootMeanSquareDistanceErrorMm;
    if ( fabs( differenceComparedToBestMm ) <= this->AmbiguityThresholdDistanceMm )
    {
      this->MatchingAmbiguous = true;
    }

    // is this the best matching?
    if ( rootMeanSquareDistanceErrorMm < this->ComputedRootMeanSquareDistanceErrorMm )
    {
      // if this is the new best matching, AND it is outside threshold of the current best, flag as NOT ambiguous
      if ( fabs( differenceComparedToBestMm ) > this->AmbiguityThresholdDistanceMm )
      {
        this->MatchingAmbiguous = false;
      }
      this->ComputedRootMeanSquareDistanceErrorMm = rootMeanSquareDistanceErrorMm;
      this->OutputPointList1->DeepCopy( pointSubset1 );
      this->OutputPointList2->DeepCopy( permutedPointSubset2 );
    }
  }
}

//------------------------------------------------------------------------------
double vtkPointMatcher::ComputeRootMeanSquareistanceErrors( vtkPointDistanceMatrix* distanceMatrix1, vtkPointDistanceMatrix* distanceMatrix2 )
{
  if ( distanceMatrix1 == NULL || distanceMatrix2 == NULL )
  {
    vtkGenericWarningMacro( "One of the input distance matrices is null. Cannot compute similarity. Returning default value " << RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR << "." );
    return RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  }

  vtkSmartPointer< vtkDoubleArray > distanceErrorMatrix = vtkSmartPointer< vtkDoubleArray >::New();
  vtkPointDistanceMatrix::ComputePairWiseDifferences( distanceMatrix2, distanceMatrix1, distanceErrorMatrix );

  double sumOfSquaredDistanceErrors = 0;
  int numberOfColumns = distanceErrorMatrix->GetNumberOfTuples();
  int numberOfRows = distanceErrorMatrix->GetNumberOfComponents();
  for ( int columnIndex = 0; columnIndex < numberOfColumns; columnIndex++ )
  {
    for ( int rowIndex = 0; rowIndex < numberOfRows; rowIndex++ )
    {
      double currentDistanceError = distanceErrorMatrix->GetComponent( columnIndex, rowIndex );
      sumOfSquaredDistanceErrors += ( currentDistanceError * currentDistanceError );
    }
  }

  int numberOfDistances = numberOfColumns * numberOfRows;
  double meanOfSquaredDistanceErrors = sumOfSquaredDistanceErrors / numberOfDistances;
  double rootMeanSquareistanceErrors = sqrt( meanOfSquaredDistanceErrors );

  return rootMeanSquareistanceErrors;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::UpdateNeeded()
{
  return ( this->GetMTime() > this->OutputChangedTime );
}

