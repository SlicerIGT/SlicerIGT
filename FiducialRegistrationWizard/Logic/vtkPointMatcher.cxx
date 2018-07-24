#include "vtkPointMatcher.h"
#include "vtkCombinatoricGenerator.h"
#include <vtkDoubleArray.h>
#include <vtkLandmarkTransform.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMath.h>

#define RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR VTK_DOUBLE_MAX
#define MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH 3
#define MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH 8

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
  
  this->ComputedRootMeanSquareDistanceErrorMm = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  this->TolerableRootMeanSquareDistanceErrorMm; // TODO: This should be computed as a fraction of average distance between points
  this->MatchingAmbiguous = false;
  this->OutputPointList1->Reset();
  this->OutputPointList2->Reset();

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
    this->HandleMatchFailure();
    return;
  }

  int numberOfPointsInList1 = this->InputPointList1->GetNumberOfPoints();
  int numberOfPointsInList2 = this->InputPointList2->GetNumberOfPoints();
  int smallerPointListSize = vtkMath::Min( numberOfPointsInList1, numberOfPointsInList2 );
  if ( numberOfPointsInList1 <= MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH &&
       numberOfPointsInList2 <= MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH )
  {
    // point set is small enough for brute force approach
    int minimumSubsetSize = vtkMath::Max( ( smallerPointListSize - this->MaximumDifferenceInNumberOfPoints ), ( unsigned int )MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH );
    int maximumSubsetSize = smallerPointListSize;
    vtkPointMatcher::UpdateBestMatchingForSubsetsOfPoints( minimumSubsetSize, maximumSubsetSize,
                                                            this->InputPointList1, this->InputPointList2,
                                                            this->AmbiguityThresholdDistanceMm, this->MatchingAmbiguous,
                                                            this->ComputedRootMeanSquareDistanceErrorMm, this->TolerableRootMeanSquareDistanceErrorMm,
                                                            this->OutputPointList1, this->OutputPointList2 );
  }
  else
  {
    vtkErrorMacro( "Point lists of length more than " << MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH << " are not yet supported." )
  }
  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
void vtkPointMatcher::HandleMatchFailure()
{
  if ( this->InputPointList1 == NULL )
  {
    vtkWarningMacro( "Input point list 1 is null." );
    return;
  }

  if ( this->InputPointList2 == NULL )
  {
    vtkWarningMacro( "Input point list 2 is null." );
    return;
  }

  // matching failed... just output the first N points of both lists
  int numberOfPointsInList1 = this->InputPointList1->GetNumberOfPoints();
  int numberOfPointsInList2 = this->InputPointList1->GetNumberOfPoints();
  int smallestNumberOfPoints = vtkMath::Min( numberOfPointsInList1, numberOfPointsInList2 );

  if ( smallestNumberOfPoints < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH )
  {
    vtkWarningMacro( "There are not enough points for a matching." );
  }

  vtkPointMatcher::CopyFirstNPoints( this->InputPointList1, this->OutputPointList1, smallestNumberOfPoints );
  vtkPointMatcher::CopyFirstNPoints( this->InputPointList2, this->OutputPointList2, smallestNumberOfPoints );
  this->OutputChangedTime.Modified();

}

//------------------------------------------------------------------------------
void vtkPointMatcher::UpdateBestMatchingForSubsetsOfPoints( int minimumSubsetSize,
                                                            int maximumSubsetSize,
                                                            vtkPoints* unmatchedPointList1,
                                                            vtkPoints* unmatchedPointList2,
                                                            double ambiguityThresholdDistanceMm,
                                                            bool& matchingAmbiguous, 
                                                            double& computedRootMeanSquareDistanceErrorMm,
                                                            double tolerableRootMeanSquareDistanceErrorMm,
                                                            vtkPoints* outputMatchedPointList1,
                                                            vtkPoints* outputMatchedPointList2 )
{
  // lots of error checking
  if ( maximumSubsetSize < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH )
  {
    vtkGenericWarningMacro( "Maximum subset size " << minimumSubsetSize << " must be at least " << MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH << ". Setting to " << MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH << "." );
    maximumSubsetSize = MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH;
  }

  if ( minimumSubsetSize < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH )
  {
    vtkGenericWarningMacro( "Minimum subset size " << minimumSubsetSize << " must be at least " << MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH << ". Setting to " << MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH << "." );
    minimumSubsetSize = MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH;
  }

  if ( maximumSubsetSize < minimumSubsetSize )
  {
    vtkGenericWarningMacro( "Maximum subset size " << maximumSubsetSize << " must be greater than or equal to minimum subset size " << minimumSubsetSize << ". Setting to " << minimumSubsetSize << "." );
    maximumSubsetSize = minimumSubsetSize;
  }

  if ( unmatchedPointList1 == NULL )
  {
    vtkGenericWarningMacro( "Unmatched point list 1 is null." );
    return;
  }

  int numberOfPointsInUnmatched1 = unmatchedPointList1->GetNumberOfPoints();
  if ( numberOfPointsInUnmatched1 < maximumSubsetSize )
  {
    vtkGenericWarningMacro( "Maximum subset size is " << maximumSubsetSize << " but the unmatched point list 1 has only " << numberOfPointsInUnmatched1 << " points." );
    return;
  }

  if ( unmatchedPointList2 == NULL )
  {
    vtkGenericWarningMacro( "Unmatched point list 2 is null." );
    return;
  }

  int numberOfPointsInUnmatched2 = unmatchedPointList2->GetNumberOfPoints();
  if ( numberOfPointsInUnmatched2 < maximumSubsetSize )
  {
    vtkGenericWarningMacro( "Maximum subset size is " << maximumSubsetSize << " but the unmatched point list 2 has only " << numberOfPointsInUnmatched2 << " points." );
    return;
  }

  if ( outputMatchedPointList1 == NULL )
  {
    vtkGenericWarningMacro( "Output matched point list 1 is null." );
    return;
  }

  if ( outputMatchedPointList2 == NULL )
  {
    vtkGenericWarningMacro( "Output matched point list 2 is null." );
    return;
  }

  for ( int subsetSize = maximumSubsetSize; subsetSize >= minimumSubsetSize; subsetSize-- )
  {
    vtkPointMatcher::UpdateBestMatchingForNSizedSubsetsOfPoints( subsetSize,
                                                                 unmatchedPointList1, unmatchedPointList2,
                                                                 ambiguityThresholdDistanceMm, matchingAmbiguous,
                                                                 computedRootMeanSquareDistanceErrorMm,
                                                                 outputMatchedPointList1, outputMatchedPointList2 );
    if ( computedRootMeanSquareDistanceErrorMm <= tolerableRootMeanSquareDistanceErrorMm )
    {
      // suitable solution has been found, no need to continue searching
      break;
    }
  }
}

//------------------------------------------------------------------------------
void vtkPointMatcher::UpdateBestMatchingForNSizedSubsetsOfPoints(
  int subsetSize,
  vtkPoints* pointList1,
  vtkPoints* pointList2,
  double ambiguityThresholdDistanceMm,
  bool& matchingAmbiguous,
  double& computedRootMeanSquareDistanceErrorMm,
  vtkPoints* outputMatchedPointList1,
  vtkPoints* outputMatchedPointList2 )
{
  if ( pointList1 == NULL )
  {
    vtkGenericWarningMacro( "Point list 1 is null." );
    return;
  }

  int pointList1NumberOfPoints = pointList1->GetNumberOfPoints();
  if ( pointList1NumberOfPoints < subsetSize )
  {
    vtkGenericWarningMacro( "Looking for subsets of " << subsetSize << " points, but list 1 has only " << pointList1NumberOfPoints << " points." );
    return;
  }

  if ( pointList2 == NULL )
  {
    vtkGenericWarningMacro( "Point list 2 is null." );
    return;
  }

  int pointList2NumberOfPoints = pointList2->GetNumberOfPoints();
  if ( pointList2NumberOfPoints < subsetSize )
  {
    vtkGenericWarningMacro( "Looking for subsets of " << subsetSize << " points, but list 2 has only " << pointList2NumberOfPoints << " points." );
    return;
  }

  // generate sets of indices for all possible combinations of both input sets
  vtkSmartPointer< vtkCombinatoricGenerator > pointList1CombinationGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  pointList1CombinationGenerator->SetCombinatoricToCombination();
  pointList1CombinationGenerator->SetSubsetSize( subsetSize );
  pointList1CombinationGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < pointList1NumberOfPoints; pointIndex++ )
  {
    pointList1CombinationGenerator->AddInputElement( 0, pointIndex );
  }
  pointList1CombinationGenerator->Update();
  std::vector< std::vector< int > > pointList1CombinationIndices = pointList1CombinationGenerator->GetOutputSets();

  vtkSmartPointer< vtkCombinatoricGenerator > pointList2CombinationGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  pointList2CombinationGenerator->SetCombinatoricToCombination();
  pointList2CombinationGenerator->SetSubsetSize( subsetSize );
  pointList2CombinationGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < pointList2NumberOfPoints; pointIndex++ )
  {
    pointList2CombinationGenerator->AddInputElement( 0, pointIndex );
  }
  pointList2CombinationGenerator->Update();
  std::vector< std::vector< int > > pointList2CombinationIndices = pointList2CombinationGenerator->GetOutputSets();

  // these will store the actual combinations of points themselves (not indices)
  vtkSmartPointer< vtkPoints > pointList1Combination = vtkSmartPointer< vtkPoints >::New();
  pointList1Combination->SetNumberOfPoints( subsetSize );
  vtkSmartPointer< vtkPoints > pointList2Combination = vtkSmartPointer< vtkPoints >::New();
  pointList2Combination->SetNumberOfPoints( subsetSize );

  // iterate over all combinations of both input point sets
  for ( unsigned int pointList1CombinationIndex = 0; pointList1CombinationIndex < pointList1CombinationIndices.size(); pointList1CombinationIndex++ )
  {
    for ( unsigned int pointList2CombinationIndex = 0; pointList2CombinationIndex < pointList2CombinationIndices.size(); pointList2CombinationIndex++ )
    {
      // store appropriate contents in the pointList1Combination and pointList2Combination variables
      for ( vtkIdType pointIndex = 0; pointIndex < subsetSize; pointIndex++ )
      {
        vtkIdType point1Index = ( vtkIdType ) pointList1CombinationIndices[ pointList1CombinationIndex ][ pointIndex ];
        double* pointFromList1 = pointList1->GetPoint( point1Index );
        pointList1Combination->SetPoint( pointIndex, pointFromList1 );

        vtkIdType point2Index = ( vtkIdType ) pointList2CombinationIndices[ pointList2CombinationIndex ][ pointIndex ];
        double* pointFromList2 = pointList2->GetPoint( point2Index );
        pointList2Combination->SetPoint( pointIndex, pointFromList2 );
      }
      // finally see how good this particular combination is
      vtkPointMatcher::UpdateBestMatchingForSubsetOfPoints( pointList1Combination, pointList2Combination,
                                                            ambiguityThresholdDistanceMm, matchingAmbiguous,
                                                            computedRootMeanSquareDistanceErrorMm,
                                                            outputMatchedPointList1, outputMatchedPointList2 );
    }
  }
}

//------------------------------------------------------------------------------
// point pair matching will be based on the distances between each pair of ordered points.
// we have two input point lists. We want to reorder the second list such that the 
// point-to-point distances are as close as possible to those in the first.
// We will permute over all possibilities (and only ever keep the best result.)
void vtkPointMatcher::UpdateBestMatchingForSubsetOfPoints(
  vtkPoints* pointSubset1, 
  vtkPoints* pointSubset2,
  double ambiguityThresholdDistanceMm,
  bool& matchingAmbiguous,
  double& computedRootMeanSquareDistanceErrorMm,
  vtkPoints* outputMatchedPointList1,
  vtkPoints* outputMatchedPointList2 )
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
    double rootMeanSquareDistanceErrorMm = vtkPointMatcher::ComputeRootMeanSquareDistanceBetweenRegisteredPointSets( pointSubset1, permutedPointSubset2 );

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
    double differenceComparedToBestMm = computedRootMeanSquareDistanceErrorMm - rootMeanSquareDistanceErrorMm;
    bool withinAmbiguityThresholdDistanceMm = ( fabs( differenceComparedToBestMm ) <= ambiguityThresholdDistanceMm );

    // is this the best matching?
    if ( rootMeanSquareDistanceErrorMm < computedRootMeanSquareDistanceErrorMm )
    {
      if ( withinAmbiguityThresholdDistanceMm )
      {
        // case 1, described above
        matchingAmbiguous = true;
      }
      else
      {
        // case 3, described above
        matchingAmbiguous = false;
      }
      computedRootMeanSquareDistanceErrorMm = rootMeanSquareDistanceErrorMm;
      outputMatchedPointList1->DeepCopy( pointSubset1 );
      outputMatchedPointList2->DeepCopy( permutedPointSubset2 );
    }
    else if ( withinAmbiguityThresholdDistanceMm )
    {
      // case 2, described above
      matchingAmbiguous = true;
    }
  }
}

//------------------------------------------------------------------------------
double vtkPointMatcher::ComputeRootMeanSquareDistanceBetweenRegisteredPointSets( vtkPoints* sourcePoints, vtkPoints* targetPoints )
{
  if ( sourcePoints == NULL || targetPoints == NULL )
  {
    vtkGenericWarningMacro( "At least one of the input points point lists is null. Returning default value " << RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR << "." );
    return RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  }

  vtkSmartPointer< vtkLandmarkTransform > landmarkTransform = vtkSmartPointer< vtkLandmarkTransform >::New();
  landmarkTransform->SetSourceLandmarks( sourcePoints );
  landmarkTransform->SetTargetLandmarks( targetPoints );
  landmarkTransform->Update();

  vtkSmartPointer< vtkPolyData > sourcePointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  sourcePointsPolyData->SetPoints( sourcePoints );

  vtkSmartPointer< vtkTransformPolyDataFilter > transformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  transformFilter->SetTransform( landmarkTransform );
  transformFilter->SetInputData( sourcePointsPolyData );
  transformFilter->Update();

  vtkPolyData* transformedSourcePointsPolyData = transformFilter->GetOutput();
  if ( transformedSourcePointsPolyData == NULL )
  {
    vtkGenericWarningMacro( "Transformed source points poly data is null Returning default value " << RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR << "." );
    return RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  }

  vtkPoints* transformedSourcePoints = transformedSourcePointsPolyData->GetPoints();
  if ( transformedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Transformed source point list is null. Returning default value " << RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR << "." );
    return RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  }

  int numberOfPoints = targetPoints->GetNumberOfPoints();
  if ( transformedSourcePoints->GetNumberOfPoints() != numberOfPoints )
  {
    vtkGenericWarningMacro( "Point lists are not of same size " << transformedSourcePoints->GetNumberOfPoints() << " and " << numberOfPoints << ". Returning default value " << RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR << "." );
    return RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  }

  double sumOfSquaredDistances = 0.0;
  for ( int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++ )
  {
    double transformedSourcePoint[ 3 ];
    transformedSourcePoints->GetPoint( pointIndex, transformedSourcePoint );
    double targetPoint[ 3 ];
    targetPoints->GetPoint( pointIndex, targetPoint );
    double squaredDistance = vtkMath::Distance2BetweenPoints( transformedSourcePoint, targetPoint );
    sumOfSquaredDistances += squaredDistance;
  }
  double meanOfSquaredDistances = sumOfSquaredDistances / numberOfPoints;
  double rootMeanSquareistanceErrors = sqrt( meanOfSquaredDistances );
  return rootMeanSquareistanceErrors;
}

//------------------------------------------------------------------------------
void vtkPointMatcher::CopyFirstNPoints( vtkPoints* inputList, vtkPoints* outputList, int n )
{
  if ( inputList == NULL )
  {
    vtkGenericWarningMacro( "Input list is null." );
    return;
  }

  int numberOfInputPoints = inputList->GetNumberOfPoints();
  if ( n > numberOfInputPoints )
  {
    vtkGenericWarningMacro( "Call to copy " << n << " points, but there are only " << numberOfInputPoints << " points in the list." );
    return;
  }

  if ( outputList == NULL )
  {
    vtkGenericWarningMacro( "Input list is null." );
    return;
  }

  outputList->Reset();
  for ( int pointIndex = 0; pointIndex < n; pointIndex++ )
  {
    double point1[ 3 ];
    inputList->GetPoint( pointIndex, point1 );
    outputList->InsertNextPoint( point1 );
  }
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::UpdateNeeded()
{
  return ( this->GetMTime() > this->OutputChangedTime );
}

