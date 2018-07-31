#include "vtkPointMatcher.h"
#include "vtkPointDistanceMatrix.h"
#include "vtkCombinatoricGenerator.h"
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMath.h>

#define RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR VTK_DOUBLE_MAX
#define MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH 3
#define MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH 5

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

  int numberOfSourcePoints = this->InputSourcePoints->GetNumberOfPoints();
  int numberOfTargetPoints = this->InputTargetPoints->GetNumberOfPoints();
  unsigned int differenceInPointListSizes = abs( numberOfSourcePoints - numberOfTargetPoints );
  bool matchingSuccessful = false;
  if ( numberOfSourcePoints < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH ||
       numberOfTargetPoints < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH ||
       differenceInPointListSizes > this->MaximumDifferenceInNumberOfPoints )
  {
    // failure cases
    matchingSuccessful = false;
  }
  else if ( numberOfSourcePoints <= MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH &&
            numberOfTargetPoints <= MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH )
  {
    matchingSuccessful = this->MatchPointsExhaustively();
  }
  else
  {
    matchingSuccessful = this->MatchPointsGenerally();
  }

  if ( !matchingSuccessful )
  {
    this->HandleMatchFailure();
  }
  
  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::MatchPointsExhaustively()
{
  int numberOfSourcePoints = this->InputSourcePoints->GetNumberOfPoints();
  int numberOfTargetPoints = this->InputTargetPoints->GetNumberOfPoints();
  int smallerPointListSize = vtkMath::Min( numberOfSourcePoints, numberOfTargetPoints );
  int minimumSubsetSize = vtkMath::Max( ( smallerPointListSize - this->MaximumDifferenceInNumberOfPoints ), ( unsigned int )MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH );
  int maximumSubsetSize = smallerPointListSize;
  vtkPointMatcher::UpdateBestMatchingForSubsetsOfPoints( minimumSubsetSize, maximumSubsetSize,
                                                          this->InputSourcePoints, this->InputTargetPoints,
                                                          this->AmbiguityDistanceError, this->MatchingAmbiguous,
                                                          this->ComputedDistanceError, this->TolerableDistanceError,
                                                          this->OutputSourcePoints, this->OutputTargetPoints );
  return true; // search is exhaustive, so it *will* find the best match
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::MatchPointsGenerally()
{
  bool matchingSuccessful = false;

  // try any algorithms here in turn until one is successful
  matchingSuccessful = this->MatchPointsGenerallyUsingSubsample();
  if ( matchingSuccessful )
  {
    return true;
  }

  matchingSuccessful = this->MatchPointsGenerallyUsingICP();
  if ( matchingSuccessful )
  {
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::MatchPointsGenerallyUsingSubsample()
{
  // Select a subset of points according to the 'uniqueness' of their geometry relative to other points
  int numberOfSourcePoints = this->InputSourcePoints->GetNumberOfPoints();
  int numberOfTargetPoints = this->InputTargetPoints->GetNumberOfPoints();
  int smallerPointListSize = vtkMath::Min( numberOfSourcePoints, numberOfTargetPoints );
  int numberOfPointsToUseForInitialRegistration = vtkMath::Min( smallerPointListSize, MAXIMUM_NUMBER_OF_POINTS_NEEDED_FOR_DETERMINISTIC_MATCH );

  vtkSmartPointer< vtkPoints > unmatchedSourcePointsSortedByUniqueness = vtkSmartPointer< vtkPoints >::New();
  vtkPointMatcher::ReorderPointsAccordingToUniqueGeometry( this->InputSourcePoints, unmatchedSourcePointsSortedByUniqueness );
  vtkSmartPointer< vtkPoints > unmatchedReducedSourcePoints = vtkSmartPointer< vtkPoints >::New();
  vtkPointMatcher::CopyFirstNPoints( unmatchedSourcePointsSortedByUniqueness, unmatchedReducedSourcePoints, numberOfPointsToUseForInitialRegistration );
  
  vtkSmartPointer< vtkPoints > unmatchedTargetPointsSortedByUniqueness = vtkSmartPointer< vtkPoints >::New();
  vtkPointMatcher::ReorderPointsAccordingToUniqueGeometry( this->InputTargetPoints, unmatchedTargetPointsSortedByUniqueness );
  vtkSmartPointer< vtkPoints > unmatchedReducedTargetPoints = vtkSmartPointer< vtkPoints >::New();
  vtkPointMatcher::CopyFirstNPoints( unmatchedTargetPointsSortedByUniqueness, unmatchedReducedTargetPoints, numberOfPointsToUseForInitialRegistration );

  // Compute correspondence between those points
  int minimumSubsetSize = vtkMath::Max( ( numberOfPointsToUseForInitialRegistration - this->MaximumDifferenceInNumberOfPoints ), ( unsigned int )MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH );
  int maximumSubsetSize = numberOfPointsToUseForInitialRegistration;
  vtkSmartPointer< vtkPoints > initiallyMatchedReducedSourcePoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkPoints > initiallyMatchedReducedTargetPoints = vtkSmartPointer< vtkPoints >::New();
  double bestDistanceError = VTK_DOUBLE_MAX;
  double tolerableDistanceErrorForSubsets = 0.0; // will keep searching for the best fit, no early exits when dealing with subsets
  bool matchingAmbiguous = false;
  vtkPointMatcher::UpdateBestMatchingForSubsetsOfPoints( minimumSubsetSize, maximumSubsetSize,
                                                          unmatchedReducedSourcePoints, unmatchedReducedTargetPoints,
                                                          this->AmbiguityDistanceError, matchingAmbiguous,
                                                          bestDistanceError, tolerableDistanceErrorForSubsets,
                                                          initiallyMatchedReducedSourcePoints, initiallyMatchedReducedTargetPoints );

  // Compute initial registration based on this correspondence
  vtkSmartPointer< vtkLandmarkTransform > initialRegistrationTransform = vtkSmartPointer< vtkLandmarkTransform >::New();
  initialRegistrationTransform->SetSourceLandmarks( initiallyMatchedReducedSourcePoints );
  initialRegistrationTransform->SetTargetLandmarks( initiallyMatchedReducedTargetPoints );
  initialRegistrationTransform->SetModeToRigidBody();
  initiallyMatchedReducedTargetPoints->Modified();
  initialRegistrationTransform->Update();

  vtkSmartPointer< vtkPolyData > unmatchedSourcePointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  vtkPointMatcher::GeneratePolyDataFromPoints( this->InputSourcePoints, unmatchedSourcePointsPolyData );

  vtkSmartPointer< vtkTransformPolyDataFilter > initialRegistrationTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  initialRegistrationTransformFilter->SetTransform( initialRegistrationTransform );
  initialRegistrationTransformFilter->SetInputData( unmatchedSourcePointsPolyData );
  initialRegistrationTransformFilter->Update();

  vtkPolyData* initiallyRegisteredSourcePointsPolyData = vtkPolyData::SafeDownCast( initialRegistrationTransformFilter->GetOutput() );
  if ( initiallyRegisteredSourcePointsPolyData == NULL )
  {
    vtkWarningMacro( "Initially registered source points poly data is null." );
    return false;
  }

  vtkSmartPointer< vtkPolyData > unmatchedTargetPointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  vtkPointMatcher::GeneratePolyDataFromPoints( this->InputTargetPoints, unmatchedTargetPointsPolyData );

  // Do some ICP to refine the result
  vtkSmartPointer< vtkIterativeClosestPointTransform > icpTransform = vtkSmartPointer< vtkIterativeClosestPointTransform >::New();
  icpTransform->GetLandmarkTransform()->SetModeToRigidBody();
  icpTransform->StartByMatchingCentroidsOn();
  icpTransform->SetSource( initiallyRegisteredSourcePointsPolyData );
  icpTransform->SetTarget( unmatchedTargetPointsPolyData );
  icpTransform->Update();

  vtkSmartPointer< vtkGeneralTransform > concatenatedAlignment = vtkSmartPointer< vtkGeneralTransform >::New();
  concatenatedAlignment->Identity();
  concatenatedAlignment->Concatenate( initialRegistrationTransform );
  concatenatedAlignment->Concatenate( icpTransform );

  vtkSmartPointer< vtkPoints > matchedSourcePoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkPoints > matchedTargetPoints = vtkSmartPointer< vtkPoints >::New();
  double thresholdDistance2ForOutlier = this->Distance2ForOutlierRemovalAfterInitialRegistration();
  bool matchingSuccessful = vtkPointMatcher::ComputePointMatchingBasedOnRegistration( concatenatedAlignment,
                                                                                      this->InputSourcePoints, this->InputTargetPoints,
                                                                                      thresholdDistance2ForOutlier, this->MaximumDifferenceInNumberOfPoints,
                                                                                      matchedSourcePoints, matchedTargetPoints );
  if ( !matchingSuccessful )
  {
    return false;
  }

  double distanceError = vtkPointMatcher::ComputeRegistrationRootMeanSquareError( matchedSourcePoints, matchedTargetPoints );
  if ( distanceError > this->TolerableDistanceError )
  {
    return false;
  }

  this->MatchingAmbiguous = matchingAmbiguous;
  this->ComputedDistanceError = distanceError;
  this->OutputSourcePoints->DeepCopy( matchedSourcePoints );
  this->OutputTargetPoints->DeepCopy( matchedTargetPoints );
  return true;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::MatchPointsGenerallyUsingICP()
{
  // try ICP from several different starting orientations
  // represented as axis-angle combinations
  const int numberOfAxes = 13;
  const double axes[ 39 ] = // 13 axes
  { 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    0.7071, 0.7071, 0.0,
    0.7071, -0.7071, 0.0,
    0.0, 0.7071, 0.7071,
    0.0, 0.7071, -0.7071,
    0.7071, 0.0, 0.7071,
    0.7071, 0.0, -0.7071,
    0.5745, 0.5745, 0.5745,
    0.5745, -0.5745, 0.5745,
    0.5745, -0.5745, -0.5745,
    0.5745, 0.5745, -0.5745
  };
  const int numberOfAngles = 8;
  const double angles[ 8 ] = // 8 angles
  {
    0,
    45,
    90,
    135,
    180,
    225,
    270,
    315
  };

  vtkSmartPointer< vtkPolyData > unmatchedSourcePointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  bool polyDataGenerated = vtkPointMatcher::GeneratePolyDataFromPoints( this->InputSourcePoints, unmatchedSourcePointsPolyData );
  if ( !polyDataGenerated )
  {
    vtkGenericWarningMacro( "Unable to generate poly data from source points" );
    return false;
  }

  vtkSmartPointer< vtkPolyData > unmatchedTargetPointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  polyDataGenerated = vtkPointMatcher::GeneratePolyDataFromPoints( this->InputTargetPoints, unmatchedTargetPointsPolyData );
  if ( !polyDataGenerated )
  {
    vtkGenericWarningMacro( "Unable to generate poly data from target points" );
    return false;
  }

  // re-used variables
  vtkSmartPointer< vtkTransform > initialAlignmentTransform = vtkSmartPointer< vtkTransform >::New();
  vtkSmartPointer< vtkTransformPolyDataFilter > initialAlignmentransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  vtkSmartPointer< vtkIterativeClosestPointTransform > icpTransform = vtkSmartPointer< vtkIterativeClosestPointTransform >::New();
  icpTransform->GetLandmarkTransform()->SetModeToRigidBody();
  icpTransform->StartByMatchingCentroidsOn();
  vtkSmartPointer< vtkGeneralTransform > concatenatedAlignment = vtkSmartPointer< vtkGeneralTransform >::New();
  double thresholdDistance2ForOutlier = this->Distance2ForOutlierRemovalAfterInitialRegistration();
  vtkSmartPointer< vtkPoints > matchedSourcePoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkPoints > matchedTargetPoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkPoints > bestMatchedSourcePoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkPoints > bestMatchedTargetPoints = vtkSmartPointer< vtkPoints >::New();
  double bestDistanceError = VTK_DOUBLE_MAX;
  bool matchingAmbiguous;
  for ( int axisIndex = 0; axisIndex < numberOfAxes; axisIndex++ )
  {
    double axis[ 3 ];
    axis[ 0 ] = axes[ axisIndex * 3 ];
    axis[ 1 ] = axes[ axisIndex * 3 + 1 ];
    axis[ 2 ] = axes[ axisIndex * 3 + 2 ];
    for ( int angleIndex = 0; angleIndex < numberOfAngles; angleIndex++ )
    {
      double angle = angles[ angleIndex ];
      initialAlignmentTransform->Identity();
      initialAlignmentTransform->RotateWXYZ( angle, axis );
  
      initialAlignmentransformFilter->SetTransform( initialAlignmentTransform );
      initialAlignmentransformFilter->SetInputData( unmatchedSourcePointsPolyData );
      initialAlignmentransformFilter->Update();

      vtkPolyData* initiallyAlignedSourcePointsPolyData = vtkPolyData::SafeDownCast( initialAlignmentransformFilter->GetOutput() );
      if ( initiallyAlignedSourcePointsPolyData == NULL )
      {
        vtkGenericWarningMacro( "Initially aligned points poly data is null." );
        return false;
      }

      icpTransform->SetSource( initiallyAlignedSourcePointsPolyData );
      icpTransform->SetTarget( unmatchedTargetPointsPolyData );
      icpTransform->Update();

      concatenatedAlignment->Identity();
      concatenatedAlignment->Concatenate( initialAlignmentTransform );
      concatenatedAlignment->Concatenate( icpTransform );
      bool matchingSuccessful = vtkPointMatcher::ComputePointMatchingBasedOnRegistration( concatenatedAlignment,
                                                                                          this->InputSourcePoints, this->InputTargetPoints,
                                                                                          thresholdDistance2ForOutlier, this->MaximumDifferenceInNumberOfPoints,
                                                                                          matchedSourcePoints, matchedTargetPoints );
      if ( !matchingSuccessful )
      {
        continue;
      }

      double currentDistanceError = vtkPointMatcher::ComputeRegistrationRootMeanSquareError( matchedSourcePoints, matchedTargetPoints );
      vtkPointMatcher::UpdateAmbiguityFlag( currentDistanceError, bestDistanceError, this->AmbiguityDistanceError, matchingAmbiguous );
      if ( currentDistanceError == bestDistanceError )
      {
        bestMatchedSourcePoints->DeepCopy( matchedSourcePoints );
        bestMatchedTargetPoints->DeepCopy( matchedTargetPoints );
      }
    }
  }

  if ( bestDistanceError > this->TolerableDistanceError )
  {
    return false;
  }
  
  this->MatchingAmbiguous = matchingAmbiguous;
  this->ComputedDistanceError = bestDistanceError;
  this->OutputSourcePoints->DeepCopy( bestMatchedSourcePoints );
  this->OutputTargetPoints->DeepCopy( bestMatchedTargetPoints );
  return true;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::InputsValid( bool verbose )
{
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
  this->ComputedDistanceError = vtkPointMatcher::ComputeRegistrationRootMeanSquareError( this->OutputSourcePoints, this->OutputTargetPoints );
}

//------------------------------------------------------------------------------
double vtkPointMatcher::Distance2ForOutlierRemovalAfterInitialRegistration()
{
  const double thresholdDistanceMultiplier = 3; // Necessary to be a bit lenient, the initial registration is not very accurate. 3 is chosen arbitrarily
  double thresholdDistance2ForOutlier = pow( this->TolerableDistanceError, 2 ) * thresholdDistanceMultiplier;
  return thresholdDistance2ForOutlier;
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
    double distanceError = vtkPointMatcher::ComputeRegistrationRootMeanSquareError( sourceSubset, permutedTargetSubset );

    vtkPointMatcher::UpdateAmbiguityFlag( distanceError, currentBestDistanceError, ambiguityDistanceError, matchingAmbiguous );
    if ( distanceError == currentBestDistanceError )
    {
      outputMatchedSourcePoints->DeepCopy( sourceSubset );
      outputMatchedTargetPoints->DeepCopy( permutedTargetSubset );
    }
  }
}

//------------------------------------------------------------------------------
void vtkPointMatcher::UpdateAmbiguityFlag( double currentDistance, double& bestDistance, double ambiguityDistance, bool& ambiguityFlag )
{
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
  double differenceComparedToBest = bestDistance - currentDistance;
  bool withinAmbiguityError = ( fabs( differenceComparedToBest ) <= ambiguityDistance );

  // is this the best matching?
  if ( currentDistance < bestDistance )
  {
    if ( withinAmbiguityError )
    {
      // case 1, described above
      ambiguityFlag = true;
    }
    else
    {
      // case 3, described above
      ambiguityFlag = false;
    }
    bestDistance = currentDistance;
  }
  else if ( withinAmbiguityError )
  {
    // case 2, described above
    ambiguityFlag = true;
  }
}

//------------------------------------------------------------------------------
double vtkPointMatcher::ComputeRegistrationRootMeanSquareError( vtkPoints* sourcePoints, vtkPoints* targetPoints )
{
  if ( sourcePoints == NULL || targetPoints == NULL )
  {
    vtkGenericWarningMacro( "At least one of the input points point lists is null. Returning default value " << RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR << "." );
    return RESET_VALUE_COMPUTED_ROOT_MEAN_DISTANCE_ERROR;
  }

  vtkSmartPointer< vtkLandmarkTransform > landmarkTransform = vtkSmartPointer< vtkLandmarkTransform >::New();
  landmarkTransform->SetSourceLandmarks( sourcePoints );
  landmarkTransform->SetTargetLandmarks( targetPoints );
  landmarkTransform->SetModeToRigidBody();
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
bool vtkPointMatcher::ComputePointMatchingBasedOnRegistration( vtkAbstractTransform* registration,
                                                               vtkPoints* unmatchedSourcePoints,
                                                               vtkPoints* unmatchedTargetPoints,
                                                               double thresholdDistance2ForOutlier,
                                                               unsigned int maximumOutlierCount,
                                                               vtkPoints* matchedSourcePoints,
                                                               vtkPoints* matchedTargetPoints )
{
  if ( registration == NULL )
  {
    vtkGenericWarningMacro( "Registration is null." );
    return false;
  }

  if ( unmatchedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Unmatched source points are null." );
    return false;
  }

  if ( unmatchedTargetPoints == NULL )
  {
    vtkGenericWarningMacro( "Unmatched target points are null." );
    return false;
  }

  if ( matchedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Matched source points are null." );
    return false;
  }

  if ( matchedTargetPoints == NULL )
  {
    vtkGenericWarningMacro( "Matched target points are null." );
    return false;
  }

  vtkSmartPointer< vtkPolyData > unmatchedSourcePointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  unmatchedSourcePointsPolyData->SetPoints( unmatchedSourcePoints );

  vtkSmartPointer< vtkTransformPolyDataFilter > registrationTransformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
  registrationTransformFilter->SetTransform( registration );
  registrationTransformFilter->SetInputData( unmatchedSourcePointsPolyData );
  registrationTransformFilter->Update();

  // get the transformed source points
  vtkPolyData* registeredUnmatchedSourcePointsPolyData = vtkPolyData::SafeDownCast( registrationTransformFilter->GetOutput() );
  if ( registeredUnmatchedSourcePointsPolyData == NULL )
  {
    vtkGenericWarningMacro( "Registered source points poly data is null." );
    return false;
  }

  vtkPoints* registeredUnmatchedSourcePoints = registeredUnmatchedSourcePointsPolyData->GetPoints();
  if ( registeredUnmatchedSourcePoints == NULL )
  {
    vtkGenericWarningMacro( "Registered source points are null." );
    return false;
  }

  vtkSmartPointer< vtkPolyData > unmatchedTargetPointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  bool polyDataGenerated = vtkPointMatcher::GeneratePolyDataFromPoints( unmatchedTargetPoints, unmatchedTargetPointsPolyData );
  if ( !polyDataGenerated )
  {
    vtkGenericWarningMacro( "Unable to generate poly data from points" );
    return false;
  }

  // create the matched list, while removing outliers
  matchedSourcePoints->Reset();
  matchedTargetPoints->Reset();
  vtkSmartPointer< vtkPointLocator > pointLocator = vtkSmartPointer< vtkPointLocator >::New();
  pointLocator->SetDataSet( unmatchedTargetPointsPolyData );
  pointLocator->BuildLocator();
  int numberOfSourcePoints = registeredUnmatchedSourcePoints->GetNumberOfPoints();
  unsigned int outlierCount = 0;
  for ( int sourcePointIndex = 0; sourcePointIndex < numberOfSourcePoints; sourcePointIndex++ )
  {
    double registeredUnmatchedSourcePoint[ 3 ];
    registeredUnmatchedSourcePoints->GetPoint( sourcePointIndex, registeredUnmatchedSourcePoint );
    vtkIdType matchedTargetPointIndex = pointLocator->FindClosestPoint( registeredUnmatchedSourcePoint );
    double matchedTargetPoint[ 3 ];
    unmatchedTargetPoints->GetPoint( matchedTargetPointIndex, matchedTargetPoint );
    double distance2 = vtkMath::Distance2BetweenPoints( registeredUnmatchedSourcePoint, matchedTargetPoint );
    if ( distance2 < thresholdDistance2ForOutlier )
    {
      matchedTargetPoints->InsertNextPoint( matchedTargetPoint );
      double matchedSourcePoint[ 3 ];
      unmatchedSourcePoints->GetPoint( sourcePointIndex, matchedSourcePoint );
      matchedSourcePoints->InsertNextPoint( matchedSourcePoint );
    }
    else
    {
      outlierCount += 1;
    }
    if ( outlierCount > maximumOutlierCount )
    {
      return false;
    }
  }

  // if there are not enough points in the output after outlier removal, then this is not a valid solution
  int numberOfMatchedPoints = matchedSourcePoints->GetNumberOfPoints();
  if ( numberOfMatchedPoints < MINIMUM_NUMBER_OF_POINTS_NEEDED_TO_MATCH )
  {
    return false;
  }
  
  return true;
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
void vtkPointMatcher::ReorderPointsAccordingToUniqueGeometry( vtkPoints* inputUnsortedPointList, vtkPoints* outputSortedPointList )
{
  if ( inputUnsortedPointList == NULL )
  {
    vtkGenericWarningMacro( "Input point list is null." );
    return;
  }

  if ( outputSortedPointList == NULL )
  {
    vtkGenericWarningMacro( "Output sorted point list is null." );
    return;
  }

  // Figure out which points are the most 'unique'
  vtkSmartPointer< vtkDoubleArray > pointUniquenesses = vtkSmartPointer< vtkDoubleArray >::New();
  vtkPointMatcher::ComputeUniquenessesForPoints( inputUnsortedPointList, pointUniquenesses );

  // sanity check
  int numberOfPoints = inputUnsortedPointList->GetNumberOfPoints();
  int numberOfUniquenesses = pointUniquenesses->GetNumberOfTuples();
  if ( numberOfUniquenesses != numberOfPoints )
  {
    vtkGenericWarningMacro( "Number of points " << numberOfPoints << " does not match number of uniquenesses " << numberOfUniquenesses << ". This is a programming error, please report it." );
    return;
  }

  // sort list
  outputSortedPointList->DeepCopy( inputUnsortedPointList );
  for ( int currentPointIndex = 0; currentPointIndex < numberOfPoints; currentPointIndex++ )
  {
    double currentUniqueness = pointUniquenesses->GetComponent( currentPointIndex, 0 );
    for ( int otherPointIndex = currentPointIndex + 1; otherPointIndex < numberOfPoints; otherPointIndex++ )
    {
      double otherUniqueness = pointUniquenesses->GetComponent( otherPointIndex, 0 );
      if ( otherUniqueness > currentUniqueness )
      {
        // swap uniquenesses
        pointUniquenesses->SetComponent( otherPointIndex, 0, currentUniqueness );
        pointUniquenesses->SetComponent( currentPointIndex, 0, otherUniqueness );
        // swap points
        double currentPoint[ 3 ];
        outputSortedPointList->GetPoint( currentPointIndex, currentPoint );
        double otherPoint[ 3 ];
        outputSortedPointList->GetPoint( otherPointIndex, otherPoint );
        outputSortedPointList->SetPoint( currentPointIndex, otherPoint );
        outputSortedPointList->SetPoint( otherPointIndex, currentPoint );
      }
    }
  }
}

//------------------------------------------------------------------------------
void vtkPointMatcher::ComputeUniquenessesForPoints( vtkPoints* points, vtkDoubleArray* uniquenesses )
{
  if ( points == NULL )
  {
    vtkGenericWarningMacro( "Points are null." );
    return;
  }

  if ( uniquenesses == NULL )
  {
    vtkGenericWarningMacro( "Uniquenesses are null." );
    return;
  }

  vtkSmartPointer< vtkPointDistanceMatrix > pointDistanceMatrix = vtkSmartPointer< vtkPointDistanceMatrix >::New();
  pointDistanceMatrix->SetPointList1( points );
  pointDistanceMatrix->SetPointList2( points ); // distances to self
  pointDistanceMatrix->Update();

  vtkSmartPointer< vtkDoubleArray > allDistancesArray = vtkSmartPointer< vtkDoubleArray >::New();
  pointDistanceMatrix->GetDistances( allDistancesArray );
  double maximumDistance = pointDistanceMatrix->GetMaximumDistance();

  uniquenesses->Reset();
  int numberOfPoints = points->GetNumberOfPoints();
  for ( int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++ )
  {
    // heuristic measure for point uniqueness:
    // sum of point-to-point distance uniquenesses
    double sumOfDistanceUniquenesses = 0;
    for ( int otherPointIndex = 0; otherPointIndex < numberOfPoints; otherPointIndex++ )
    {
      double currentDistance = pointDistanceMatrix->GetDistance( pointIndex, otherPointIndex );
      sumOfDistanceUniquenesses += vtkPointMatcher::ComputeUniquenessForDistance( currentDistance, maximumDistance, allDistancesArray );
    }
    double pointUniqueness = sumOfDistanceUniquenesses;
    uniquenesses->InsertNextTuple1( pointUniqueness );
  }
}

//------------------------------------------------------------------------------
double vtkPointMatcher::ComputeUniquenessForDistance( double distance, double maximumDistance, vtkDoubleArray* allDistancesArray )
{
  if ( allDistancesArray == NULL )
  {
    vtkGenericWarningMacro( "Distances array is null" );
    return 0.0;
  }

  if ( maximumDistance == 0 )
  {
    vtkGenericWarningMacro( "Maximum distance is zero. Setting to 1 to avoid division by zero." );
    maximumDistance = 1.0;
  }

  double distanceUniqueness = 0.0;
  int numberOfDistances = allDistancesArray->GetNumberOfTuples();
  for ( int distanceIndex = 0; distanceIndex < numberOfDistances; distanceIndex++ )
  {
    double otherDistance = allDistancesArray->GetComponent( distanceIndex, 0 );
    if ( distance < otherDistance )
    {
      // treat uniqueness as 0
      continue;
    }
    double heuristicMeasure = 1.0 - ( abs( distance - otherDistance ) / maximumDistance ); // will be bounded between 0..1
    distanceUniqueness += heuristicMeasure;
  }
  return distanceUniqueness;
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::UpdateNeeded()
{
  return ( this->GetMTime() > this->OutputChangedTime );
}

//------------------------------------------------------------------------------
bool vtkPointMatcher::GeneratePolyDataFromPoints( vtkPoints* points, vtkPolyData* polyData )
{
  if ( points == NULL )
  {
    vtkGenericWarningMacro( "Points are null." );
    return false;
  }

  if ( polyData == NULL )
  {
    vtkGenericWarningMacro( "Poly data is null." );
    return false;
  }

  // This function is typically called before using ICP
  // ICP tends to fail with the default float representation of vtkPoints
  // http://vtk.1045678.n5.nabble.com/ICP-Error-vtkMath-Jacobi-Error-extracting-eigenfunctions-td5441946.html
  // So copy to a vtkPoints that uses double storage instead - should hopefully reduce the frequency of failure
  // If stability/warning message reduction is a priority then the vtk implementation of ICP can be 
  // abandoned in favour of a custom one.

  vtkSmartPointer< vtkPoints > copiedPoints = vtkSmartPointer< vtkPoints >::New();
  copiedPoints->SetDataTypeToDouble();
  vtkSmartPointer< vtkCellArray > verts = vtkSmartPointer< vtkCellArray >::New();
  int numberOfPoints = points->GetNumberOfPoints();
  for ( int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++ )
  {
    double point[ 3 ];
    points->GetPoint( pointIndex, point );
    copiedPoints->InsertNextPoint( point );
    vtkIdType id[ 1 ];
    id[ 0 ] = ( vtkIdType ) pointIndex;
    verts->InsertNextCell( 1, id );
  }
  polyData->SetPoints( copiedPoints );
  polyData->SetVerts( verts );

  return true;
}
