#ifndef __vtkPointMatcher_h
#define __vtkPointMatcher_h

#include <vtkObject.h>
#include <vtkTimeStamp.h>
#include <vtkSmartPointer.h>

class vtkAbstractTransform;
class vtkDoubleArray;
class vtkPoints;
class vtkPolyData;

// export
#include "vtkSlicerFiducialRegistrationWizardModuleLogicExport.h"

// This class takes two corresponding input point sets as input (source and 
// target), and tries to determine their pairing. The outputs are two lists
// that contain ordered, corresponding pairs from the two input lists.
// Extra or missing points are removed from the output.
class VTK_SLICER_FIDUCIALREGISTRATIONWIZARD_MODULE_LOGIC_EXPORT vtkPointMatcher : public vtkObject //vtkAlgorithm?
{
  public:
    vtkTypeMacro( vtkPointMatcher, vtkObject );
    static vtkPointMatcher* New();

    void PrintSelf( ostream &os, vtkIndent indent ) VTK_OVERRIDE;
    
    // Input Mutators/Accessors
    // these points may not be in order and may be different lengths
    void SetInputSourcePoints( vtkPoints* points );
    void SetInputTargetPoints( vtkPoints* points );

    // A parameter to control how many points different the two input lists can be:
    // Higher = slower but more robust
    // Lower = faster but less likely to succeed
    vtkGetMacro( MaximumDifferenceInNumberOfPoints, unsigned int );
    vtkSetMacro( MaximumDifferenceInNumberOfPoints, unsigned int );

    // The distance error that should be tolerated.
    // The value used is computed as a multiple of the maximum distance within the target point set
    // Valid range is 0 .. infinity, though value of 0.1 is recommended.
    // Higher = faster but less accurate and less likely to find a solution
    // Lower = slower, but more accurate
    vtkGetMacro( TolerableDistanceErrorMultiple, double );
    vtkSetMacro( TolerableDistanceErrorMultiple, double );

    // Distance used to determine if there is more than one feasible point-to-point mapping
    // The value used is computed as a multiple of the maximum distance within the target point set
    // Valid range is 0 .. inifinity, though value of 0.05 is recommended
    // Higher = More false positives, more true positives
    // Lower = More false negatives, more true negatives
    vtkGetMacro( AmbiguityDistanceErrorMultiple, double );
    vtkSetMacro( AmbiguityDistanceErrorMultiple, double );

    // Output Accessors
    // these points will be ordered pairs and the lists will be the same length as one another
    vtkPoints* GetOutputSourcePoints();
    vtkPoints* GetOutputTargetPoints();

    // The root mean square distance error between each pair of points, after matching
    double GetComputedDistanceError();

    bool IsMatchingWithinTolerance();
    double GetTolerableDistanceError();

    bool IsMatchingAmbiguous();
    double GetAmbiguityDistanceError();

    // Logic
    void Update();

  protected:
    vtkPointMatcher();
    ~vtkPointMatcher();

  private:
    vtkPoints* InputSourcePoints;
    vtkPoints* InputTargetPoints;

    unsigned int MaximumDifferenceInNumberOfPoints;

    double TolerableDistanceErrorMultiple; // input by user
    double TolerableDistanceError; // computed

    double AmbiguityDistanceErrorMultiple; // input by user
    double AmbiguityDistanceError; // computed
    bool MatchingAmbiguous;

    double ComputedDistanceError;

    vtkSmartPointer< vtkPoints > OutputSourcePoints;
    vtkSmartPointer< vtkPoints > OutputTargetPoints;

    // Determine whether an update is needed
    vtkTimeStamp OutputChangedTime;
    bool UpdateNeeded();

    // error checking
    bool InputsValid( bool verbose=true );

    // Logic helpers
    // all the bool methods below return 'true' on successful registration
    // otherwise they return false
    bool MatchPointsExhaustively();
    bool MatchPointsGenerally();
    bool MatchPointsGenerallyUsingUniqueDistances();
    bool MatchPointsGenerallyUsingMaximumDistancesAndCentroid();
    bool MatchPointsGenerallyUsingSubsample( vtkPoints* unmatchedReducedSourcePoints, vtkPoints* unmatchedReducedTargetPoints ); // helper to the functions above
    bool MatchPointsGenerallyUsingICP();

    void HandleMatchFailure(); // copies input point list to output point list. Used when matching is otherwise impossible.

    double Distance2ForOutlierRemovalAfterInitialRegistration();

    static void UpdateBestMatchingForSubsetsOfPoints( int minimumSubsetSize, int maximumSubsetSize,
                                                      vtkPoints* unmatchedPointList1, vtkPoints* unmatchedPointList2,
                                                      double ambiguityDistance, bool& matchingAmbiguous, 
                                                      double& computedDistanceError, double tolerableDistanceError,
                                                      vtkPoints* outputMatchedPointList1, vtkPoints* outputMatchedPointList2 );
    static void UpdateBestMatchingForNSizedSubsetsOfPoints( int subsetSize,
                                                            vtkPoints* unmatchedPointList1, vtkPoints* unmatchedPointList2,
                                                            double ambiguityDistance, bool& matchingAmbiguous, 
                                                            double& computedDistanceError,
                                                            vtkPoints* outputMatchedPointList1, vtkPoints* outputMatchedPointList2 );
    static void UpdateBestMatchingForSubsetOfPoints( vtkPoints* unmatchedPointList1, vtkPoints* unmatchedPointList2,
                                                     double ambiguityDistance, bool& matchingAmbiguous, 
                                                     double& computedDistanceError,
                                                     vtkPoints* outputMatchedPointList1, vtkPoints* outputMatchedPointList2 );
    static void UpdateAmbiguityFlag( double currentDistance, double& bestDistance, double ambiguityDistance, bool& ambiguityFlag );
    static double ComputeRegistrationRootMeanSquareError( vtkPoints* sourcePoints, vtkPoints* targetPoints );
    static bool ComputePointMatchingBasedOnRegistration( vtkAbstractTransform* registration,
                                                         vtkPoints* unmatchedSourcePoints, vtkPoints* unmatchedTargetPoints,
                                                         double thresholdDistance2ForOutlier, unsigned int maximumOutlierCount,
                                                         vtkPoints* matchedSourcePoints, vtkPoints* matchedTargetPoints );
    static double ComputeMaximumDistanceInPointSet( vtkPoints* points );
    static void CopyFirstNPoints( vtkPoints* inputList, vtkPoints* outputList, int n );
    static void ReorderPointsAccordingToUniqueGeometry( vtkPoints* inputUnsortedPointList, vtkPoints* outputSortedPointList );
    static void ComputeUniquenessesForPoints( vtkPoints* points, vtkDoubleArray* uniquenesses );
    static double ComputeUniquenessForDistance( double distance, double maximumDistance, vtkDoubleArray* allDistancesArray );
    static bool GeneratePolyDataFromPoints( vtkPoints*, vtkPolyData* );
    static bool ComputeCentroidOfPoints( vtkPoints*, double* centroid );
    static bool ExtractMaximumDistanceAndCentroidFeatures( vtkPoints* points, vtkPoints* features );

    // Not implemented:
		vtkPointMatcher(const vtkPointMatcher&);
		void operator=(const vtkPointMatcher&);
};

#endif
