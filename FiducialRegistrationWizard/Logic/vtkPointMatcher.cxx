#include "vtkPointMatcher.h"
#include "vtkPointDistanceMatrix.h"
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
  this->InputSourcePoints = NULL;
  this->InputTargetPoints = NULL;
  this->MaximumDifferenceInNumberOfPoints = 2;
  this->ComputedDistanceError = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  this->TolerableDistanceErrorMultiple = 0.1;
  this->TolerableDistanceError = 0.0;
  this->AmbiguityDistanceErrorMultiple = 0.05;
  this->AmbiguityDistanceError = 0.0;
  this->MatchingAmbiguous = false;
  // outputs are never null
  this->OutputSourcePoints = vtkSmartPointer< vtkPoints >::New();
  this->OutputTargetPoints = vtkSmartPointer< vtkPoints >::New();

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
  os << indent << "ComputedDistanceError: " << this->ComputedDistanceError << std::endl;
  os << indent << "TolerableDistanceErrorMultiple: " << this->TolerableDistanceErrorMultiple << std::endl;
  os << indent << "TolerableDistanceError: " << this->TolerableDistanceError << std::endl;
  os << indent << "AmbiguityDistanceErrorMultiple: " << this->AmbiguityDistanceErrorMultiple << std::endl;
  os << indent << "AmbiguityDistanceError: " << this->AmbiguityDistanceError << std::endl;
  os << indent << "MatchingAmbiguous: " << this->MatchingAmbiguous << std::endl;
}

//------------------------------------------------------------------------------
// INPUT MUTATORS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkPointMatcher::SetInputSourcePoints( vtkPoints* points )
{
  vtkSetObjectBodyMacro( InputSourcePoints, vtkPoints, points );
}

//------------------------------------------------------------------------------
void vtkPointMatcher::SetInputTargetPoints( vtkPoints* points )
{
  vtkSetObjectBodyMacro( InputTargetPoints, vtkPoints, points );
}

//------------------------------------------------------------------------------
// OUTPUT ACCESSORS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
vtkPoints* vtkPointMatcher::GetOutputSourcePoints()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->OutputSourcePoints;
}

//------------------------------------------------------------------------------
vtkPoints* vtkPointMatcher::GetOutputTargetPoints()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->OutputTargetPoints;
}

//------------------------------------------------------------------------------
double vtkPointMatcher::GetComputedDistanceError()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->ComputedDistanceError;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::IsMatchingWithinTolerance()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }
  
  return ( this->ComputedDistanceError <= this->TolerableDistanceError );
}

//------------------------------------------------------------------------------
double vtkPointMatcher::GetTolerableDistanceError()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->TolerableDistanceError;
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
double vtkPointMatcher::GetAmbiguityDistanceError()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  return this->AmbiguityDistanceError;
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

  if ( ! this->InputsValid() )
  {
    vtkWarningMacro( "Cannot update - inputs are invalid.." );
    return;
  }
  
  double maximumDistanceInTargetPoints = vtkPointMatcher::ComputeMaximumDistanceInPointSet( this->InputTargetPoints );
  this->ComputedDistanceError = RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  this->TolerableDistanceError = maximumDistanceInTargetPoints * this->TolerableDistanceErrorMultiple;
  this->AmbiguityDistanceError = maximumDistanceInTargetPoints * this->AmbiguityDistanceErrorMultiple;
  this->MatchingAmbiguous = false;
  this->OutputSourcePoints->Reset();
  this->OutputTargetPoints->Reset();

  // check number of points, make sure point lists are already similar/same sizes
  int numberOfSourcePoints = this->InputSourcePoints->GetNumberOfPoints();
  int numberOfTargetPoints = this->InputTargetPoints->GetNumberOfPoints();

  unsigned int differenceInPointListSizes = abs( numberOfSourcePoints - numberOfTargetPoints );
  if ( differenceInPointListSizes > this->MaximumDifferenceInNumberOfPoints )
  {
    this->HandleMatchFailure();
    return;
  }

  int smallerPointListSize = vtkMath::Min( numberOfSourcePoints, numberOfTargetPoints );
  if ( numberOfSourcePoints <= MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH &&
       numberOfTargetPoints <= MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH )
  {
    // point set is small enough for brute force approach
    int minimumSubsetSize = vtkMath::Max( ( smallerPointListSize - this->MaximumDifferenceInNumberOfPoints ), ( unsigned int )MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH );
    int maximumSubsetSize = smallerPointListSize;
    vtkPointMatcher::UpdateBestMatchingForSubsetsOfPoints( minimumSubsetSize, maximumSubsetSize,
                                                           this->InputSourcePoints, this->InputTargetPoints,
                                                           this->AmbiguityDistanceError, this->MatchingAmbiguous,
                                                           this->ComputedDistanceError, this->TolerableDistanceError,
                                                           this->OutputSourcePoints, this->OutputTargetPoints );
  }
  else
  {
    vtkErrorMacro( "Point lists of length more than " << MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH << " are not yet supported." );
  }
  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::InputsValid( bool verbose )
{
  // error checking
  if ( this->InputSourcePoints == NULL )
  {
    if ( verbose )
    {
      vtkWarningMacro( "Input source point list is null." );
    }
    return false;
  }

  if ( this->InputTargetPoints == NULL )
  {
    if ( verbose )
    {
      vtkWarningMacro( "Input target point list is null." );
    }
    return false;
  }

  if ( this->TolerableDistanceErrorMultiple < 0 )
  {
    if ( verbose )
    {
      vtkWarningMacro( "Tolerable distance error multiple is less than 0. It will be made positive.");
    }
    this->TolerableDistanceErrorMultiple *= -1;
  }

  if ( this->AmbiguityDistanceErrorMultiple < 0 )
  {
    if ( verbose )
    {
      vtkWarningMacro( "Ambiguous distance error multiple is less than 0. It will be made positive.");
    }
    this->AmbiguityDistanceErrorMultiple *= -1;
  }

  return true;
}

//------------------------------------------------------------------------------
void vtkPointMatcher::HandleMatchFailure()
{
  // assume error checking has already been done on this object
  // just output the first N points of both lists
  int numberOfSourcePoints = this->InputSourcePoints->GetNumberOfPoints();
  int numberOfTargetPoints = this->InputTargetPoints->GetNumberOfPoints();
  int smallestNumberOfPoints = vtkMath::Min( numberOfSourcePoints, numberOfTargetPoints );

  if ( smallestNumberOfPoints < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH )
  {
    vtkWarningMacro( "There are not enough points for a matching." );
  }

  vtkPointMatcher::CopyFirstNPoints( this->InputSourcePoints, this->OutputSourcePoints, smallestNumberOfPoints );
  vtkPointMatcher::CopyFirstNPoints( this->InputTargetPoints, this->OutputTargetPoints, smallestNumberOfPoints );
  this->OutputChangedTime.Modified();

}

//------------------------------------------------------------------------------
void vtkPointMatcher::UpdateBestMatchingForSubsetsOfPoints( int minimumSubsetSize,
                                                            int maximumSubsetSize,
                                                            vtkPoints* unmatchedSourcePoints,
                                                            vtkPoints* unmatchedTargetPoints,
                                                            double ambiguityDistanceError,
                                                            bool& matchingAmbiguous, 
                                                            double& currentBestDistanceError,
                                                            double tolerableDistanceError,
                                                            vtkPoints* outputMatchedSourcePoints,
                                                            vtkPoints* outputMatchedTargetPoints )
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

  if ( unmatchedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Unmatched source points are null." );
    return;
  }

  int numberOfUnmatchedSourcePoints = unmatchedSourcePoints->GetNumberOfPoints();
  if ( numberOfUnmatchedSourcePoints < maximumSubsetSize )
  {
    vtkGenericWarningMacro( "Maximum subset size is " << maximumSubsetSize << " but there are only " << numberOfUnmatchedSourcePoints << " unmatched source points." );
    return;
  }

  if ( unmatchedTargetPoints == NULL )
  {
    vtkGenericWarningMacro( "Unmatched target points are null." );
    return;
  }

  int numberOfUnmatchedTargetPoints = unmatchedTargetPoints->GetNumberOfPoints();
  if ( numberOfUnmatchedTargetPoints < maximumSubsetSize )
  {
    vtkGenericWarningMacro( "Maximum subset size is " << maximumSubsetSize << " but there are only " << numberOfUnmatchedTargetPoints << " unmatched target points." );
    return;
  }

  if ( outputMatchedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Output matched source points are null." );
    return;
  }

  if ( outputMatchedTargetPoints == NULL )
  {
    vtkGenericWarningMacro( "Output matched target points are null." );
    return;
  }

  for ( int subsetSize = maximumSubsetSize; subsetSize >= minimumSubsetSize; subsetSize-- )
  {
    vtkPointMatcher::UpdateBestMatchingForNSizedSubsetsOfPoints( subsetSize,
                                                                 unmatchedSourcePoints, unmatchedTargetPoints,
                                                                 ambiguityDistanceError, matchingAmbiguous,
                                                                 currentBestDistanceError,
                                                                 outputMatchedSourcePoints, outputMatchedTargetPoints );
    if ( currentBestDistanceError <= tolerableDistanceError )
    {
      // suitable solution has been found, no need to continue searching
      break;
    }
  }
}

//------------------------------------------------------------------------------
void vtkPointMatcher::UpdateBestMatchingForNSizedSubsetsOfPoints(
  int subsetSize,
  vtkPoints* unmatchedSourcePoints,
  vtkPoints* unmatchedTargetPoints,
  double ambiguityDistanceError,
  bool& matchingAmbiguous,
  double& currentBestDistanceError,
  vtkPoints* outputMatchedSourcePoints,
  vtkPoints* outputMatchedTargetPoints )
{
  if ( unmatchedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Unmatched source points are null." );
    return;
  }

  int numberOfUnmatchedSourcePoints = unmatchedSourcePoints->GetNumberOfPoints();
  if ( numberOfUnmatchedSourcePoints < subsetSize )
  {
    vtkGenericWarningMacro( "Looking for subsets of " << subsetSize << " points, but there are only " << numberOfUnmatchedSourcePoints << " unmatched source points." );
    return;
  }

  if ( unmatchedTargetPoints == NULL )
  {
    vtkGenericWarningMacro( "Unmatched target points are null." );
    return;
  }

  int numberOfUnmatchedTargetPoints = unmatchedTargetPoints->GetNumberOfPoints();
  if ( numberOfUnmatchedTargetPoints < subsetSize )
  {
    vtkGenericWarningMacro( "Looking for subsets of " << subsetSize << " points, but there are only " << numberOfUnmatchedTargetPoints << " unmatched target points." );
    return;
  }

  // generate sets of indices for all possible combinations of both input sets
  vtkSmartPointer< vtkCombinatoricGenerator > sourcePointsCombinationGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  sourcePointsCombinationGenerator->SetCombinatoricToCombination();
  sourcePointsCombinationGenerator->SetSubsetSize( subsetSize );
  sourcePointsCombinationGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < numberOfUnmatchedSourcePoints; pointIndex++ )
  {
    sourcePointsCombinationGenerator->AddInputElement( 0, pointIndex );
  }
  sourcePointsCombinationGenerator->Update();
  std::vector< std::vector< int > > sourcePointsCombinationIndices = sourcePointsCombinationGenerator->GetOutputSets();

  vtkSmartPointer< vtkCombinatoricGenerator > targetPointsCombinationGenerator = vtkSmartPointer< vtkCombinatoricGenerator >::New();
  targetPointsCombinationGenerator->SetCombinatoricToCombination();
  targetPointsCombinationGenerator->SetSubsetSize( subsetSize );
  targetPointsCombinationGenerator->SetNumberOfInputSets( 1 );
  for ( int pointIndex = 0; pointIndex < numberOfUnmatchedTargetPoints; pointIndex++ )
  {
    targetPointsCombinationGenerator->AddInputElement( 0, pointIndex );
  }
  targetPointsCombinationGenerator->Update();
  std::vector< std::vector< int > > targetPointsCombinationIndices = targetPointsCombinationGenerator->GetOutputSets();

  // these will store the actual combinations of points themselves (not indices)
  vtkSmartPointer< vtkPoints > unmatchedSourcePointsCombination = vtkSmartPointer< vtkPoints >::New();
  unmatchedSourcePointsCombination->SetNumberOfPoints( subsetSize );
  vtkSmartPointer< vtkPoints > unmatchedTargetPointsCombination = vtkSmartPointer< vtkPoints >::New();
  unmatchedTargetPointsCombination->SetNumberOfPoints( subsetSize );
  // iterate over all combinations of both input point sets
  for ( unsigned int sourcePointsCombinationIndex = 0; sourcePointsCombinationIndex < sourcePointsCombinationIndices.size(); sourcePointsCombinationIndex++ )
  {
    for ( unsigned int targetPointsCombinationIndex = 0; targetPointsCombinationIndex < targetPointsCombinationIndices.size(); targetPointsCombinationIndex++ )
    {
      // store appropriate contents in the unmatchedSourcePointsCombination and unmatchedTargetPointsCombination variables
      for ( vtkIdType combinationPointIndex = 0; combinationPointIndex < subsetSize; combinationPointIndex++ )
      {
        vtkIdType sourcePointIndex = ( vtkIdType ) sourcePointsCombinationIndices[ sourcePointsCombinationIndex ][ combinationPointIndex ];
        double sourcePoint[ 3 ];
        unmatchedSourcePoints->GetPoint( sourcePointIndex, sourcePoint );
        unmatchedSourcePointsCombination->SetPoint( combinationPointIndex, sourcePoint );

        vtkIdType targetPointIndex = ( vtkIdType ) targetPointsCombinationIndices[ targetPointsCombinationIndex ][ combinationPointIndex ];
        double targetPoint[ 3 ];
        unmatchedTargetPoints->GetPoint( targetPointIndex, targetPoint );
        unmatchedTargetPointsCombination->SetPoint( combinationPointIndex, targetPoint );
      }
      // finally see how good this particular combination is
      vtkPointMatcher::UpdateBestMatchingForSubsetOfPoints( unmatchedSourcePointsCombination, unmatchedTargetPointsCombination,
                                                            ambiguityDistanceError, matchingAmbiguous,
                                                            currentBestDistanceError,
                                                            outputMatchedSourcePoints, outputMatchedTargetPoints );
    }
  }
}

//------------------------------------------------------------------------------
// point pair matching will be based on the distances between each pair of ordered points.
// we have two input point lists. We want to reorder the second list such that the 
// point-to-point distances are as close as possible to those in the first.
// We will permute over all possibilities (and only ever keep the best result.)
void vtkPointMatcher::UpdateBestMatchingForSubsetOfPoints(
  vtkPoints* sourceSubset, 
  vtkPoints* targetSubset,
  double ambiguityDistanceError,
  bool& matchingAmbiguous,
  double& currentBestDistanceError,
  vtkPoints* outputMatchedSourcePoints,
  vtkPoints* outputMatchedTargetPoints )
{
  // error checking
  if ( sourceSubset == NULL )
  {
    vtkGenericWarningMacro( "Source points are null." );
    return;
  }

  if ( targetSubset == NULL )
  {
    vtkGenericWarningMacro( "Target points are null." );
    return;
  }

  int numberOfPoints = sourceSubset->GetNumberOfPoints(); // sizes of both lists should be identical
  if ( numberOfPoints != targetSubset->GetNumberOfPoints() )
  {
    vtkGenericWarningMacro( "Subsets are of different sizes." );
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
  std::vector< std::vector< int > > targetSubsetIndexPermutations = combinatoricGenerator->GetOutputSets();

  // iterate over all permutations - look for the most 'suitable'
  // point matching that gives distances most similar to the reference

  // create + allocate the permuted compare list once outside
  // the loop to avoid allocation/deallocation time costs.
  vtkSmartPointer< vtkPoints > permutedTargetSubset = vtkSmartPointer< vtkPoints >::New();
  permutedTargetSubset->SetNumberOfPoints( numberOfPoints );
  int numberOfPermutations = targetSubsetIndexPermutations.size();
  for ( int permutationIndex = 0; permutationIndex < numberOfPermutations; permutationIndex++ )
  {
    // fill permutedTargetSubset with points from targetSubset,
    // in the order indicate by the permuted indices
    for ( int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++ )
    {
      int permutedPointIndex = targetSubsetIndexPermutations[ permutationIndex][ pointIndex ];
      double* permutedPoint = targetSubset->GetPoint( permutedPointIndex );
      permutedTargetSubset->SetPoint( pointIndex, permutedPoint );
    }
    double distanceError = vtkPointMatcher::ComputeRootMeanSquareDistanceBetweenRegisteredPointSets( sourceSubset, permutedTargetSubset );

    // case analysis for setting MatchingAmbiguous:
    // 1
    // Error is better (lower) than the current best,
    // but by less than Ambiguity Distance Error
    // Result => MatchingAmbiguous should be set to true
    //   - trivial justification
    // 2
    // Error is worse (higher) than the current best,
    // but by less than Ambiguity Distance Error
    // Result => MatchingAmbiguous should be set to true
    //   - trivial justification
    // 3
    // Error is better (lower) than the current best,
    // but by more than Ambiguity Distance Error
    // Result => MatchingAmbiguous should be set to false.
    //   - If there was a _previous_ error within Ambiguity Distance Error,
    //     that would have become the best. Therefore there have not been
    //     any _previous_ error within Ambiguity Distance Error.
    //   - If _later_ there is a error within Ambiguity Distance Error,
    //     then MatchingAmbiguous will be set to true by either case 1 or case 2.
    //     Cases 1 and 2 will always catch an ambiguous matching, because the search is exhaustive.
    // 4
    // Error is worse (higher) than the current best,
    // but by more than Ambiguity Distance Error
    // Result => do nothing
    //   - This result does not matter. It *cannot* be within Ambiguity Distance Error
    //     of the overall (final) best error

    // flag ambiguous if suitability within some threshold of the best result so far
    double differenceComparedToBest = currentBestDistanceError - distanceError;
    bool withinAmbiguityError = ( fabs( differenceComparedToBest ) <= ambiguityDistanceError );

    // is this the best matching?
    if ( distanceError < currentBestDistanceError )
    {
      if ( withinAmbiguityError )
      {
        // case 1, described above
        matchingAmbiguous = true;
      }
      else
      {
        // case 3, described above
        matchingAmbiguous = false;
      }
      currentBestDistanceError = distanceError;
      outputMatchedSourcePoints->DeepCopy( sourceSubset );
      outputMatchedTargetPoints->DeepCopy( permutedTargetSubset );
    }
    else if ( withinAmbiguityError )
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
double vtkPointMatcher::ComputeMaximumDistanceInPointSet( vtkPoints* points )
{
  if ( points == NULL )
  {
    vtkGenericWarningMacro( "Points are null. Returning 0." );
    return 0.0;
  }

  int numberOfPoints = points->GetNumberOfPoints();
  if ( numberOfPoints == 0 )
  {
    vtkGenericWarningMacro( "Points list is of size 0. Returning 0." );
    return 0.0;
  }

  vtkSmartPointer< vtkPointDistanceMatrix > pointDistanceMatrix = vtkSmartPointer< vtkPointDistanceMatrix >::New();
  pointDistanceMatrix->SetPointList1( points );
  pointDistanceMatrix->SetPointList2( points );
  pointDistanceMatrix->Update();
  double maximumDistance = pointDistanceMatrix->GetMaximumDistance();
  return maximumDistance;
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

