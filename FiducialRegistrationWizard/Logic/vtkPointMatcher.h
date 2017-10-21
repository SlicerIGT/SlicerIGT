#ifndef __vtkPointMatcher_h
#define __vtkPointMatcher_h

#include <vtkObject.h>
#include <vtkPoints.h>
#include <vtkTimeStamp.h>
#include "vtkPointDistanceMatrix.h"

// This class takes two corresponding input point sets as input (InputPointList1 and 
// InputPointList2), and tries to determine their pairing. The outputs are two lists
// (OutputPointList1 and OutputPointList2) that contain ordered, corresponding pairs
// from the two input lists. Extra or missing points are removed from the output.
class vtkPointMatcher : public vtkObject //vtkAlgorithm?
{
  public:
    vtkTypeMacro( vtkPointMatcher, vtkObject );
    static vtkPointMatcher* New();

    void PrintSelf( ostream &os, vtkIndent indent ) VTK_OVERRIDE;
    
    // Input Mutators/Accessors
    void SetInputPointList1( vtkPoints* points );
    void SetInputPointList2( vtkPoints* points );
    void SetMaximumDifferenceInNumberOfPoints( unsigned int );
    void SetTolerableRootMeanSquareDistanceErrorMm( double );
    vtkGetMacro( MaximumDifferenceInNumberOfPoints, unsigned int );
    vtkGetMacro( TolerableRootMeanSquareDistanceErrorMm, double );

    // Output Accessors
    vtkPoints* GetOutputPointList1();
    vtkPoints* GetOutputPointList2();
    double GetComputedRootMeanSquareDistanceErrorMm();
    bool IsMatchingWithinTolerance();

    // Logic
    void Update();

  protected:
    vtkPointMatcher();
    ~vtkPointMatcher();

  private:
    // these points may not be in order,
    // and may be different lengths
    vtkPoints* InputPointList1;
    vtkPoints* InputPointList2;

    // A parameter to control how many points different
    // the two input lists can be:
    // Higher = slower but more robust
    // Lower = faster but less likely to succeed
    unsigned int MaximumDifferenceInNumberOfPoints;

    // Return a result when this threshold error
    // has been met:
    // Higher = faster but less accurate and less likely to find a solution
    // Lower = slower,  but more accurate
    double TolerableRootMeanSquareDistanceErrorMm;

    // The mean distance error between each pair of points, after matching
    double ComputedRootMeanSquareDistanceErrorMm;

    // these points will be ordered pairs
    // and the same length as one another
    vtkSmartPointer< vtkPoints > OutputPointList1;
    vtkSmartPointer< vtkPoints > OutputPointList2;

    // Determine whether an update is needed
    vtkTimeStamp OutputChangedTime;
    bool UpdateNeeded();

    // Logic helpers
    void UpdateBestMatchingForAllSubsetsOfPoints( int sizeOfSubset );
    void UpdateBestMatchingForSubsetOfPoints( vtkPoints* list1, vtkPoints* list2 );
    double ComputeRootMeanSquareistanceErrors( vtkPointDistanceMatrix*, vtkPointDistanceMatrix* );

    // Not implemented:
		vtkPointMatcher(const vtkPointMatcher&);
		void operator=(const vtkPointMatcher&);
};

#endif
