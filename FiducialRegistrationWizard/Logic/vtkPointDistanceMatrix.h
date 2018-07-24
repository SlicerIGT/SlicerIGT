#ifndef __vtkPointDistanceMatrix_h
#define __vtkPointDistanceMatrix_h

#include <vtkDoubleArray.h>
#include <vtkObject.h>
#include <vtkPoints.h>
#include <vtkTimeStamp.h>

// export
#include "vtkSlicerFiducialRegistrationWizardModuleLogicExport.h"

// At its most basic level, this is a Wrapper class for storing a matrix of
// point to point distances in a vtkDoubleArray.
// The purpose is to improve the abstraction of storing distances, instead of
// memorizing how distances are accessed etc... One can use this class to
// encapsulate that functionality.
// The contents of the matrix are automatically re-generated when either input
// point list is changed.
class VTK_SLICER_FIDUCIALREGISTRATIONWIZARD_MODULE_LOGIC_EXPORT vtkPointDistanceMatrix : public vtkObject //vtkAlgorithm?
{
  public:
    vtkTypeMacro( vtkPointDistanceMatrix, vtkObject );
    void PrintSelf( ostream &os, vtkIndent indent ) VTK_OVERRIDE;
    
    static vtkPointDistanceMatrix* New();
    
    vtkPoints* GetPointList1();
    vtkPoints* GetPointList2();
    double GetDistance( int list1Index, int list2Index );
    void GetDistances( vtkDoubleArray* outputArray );
    vtkGetMacro( MaximumDistance, int );
    vtkGetMacro( MinimumDistance, int );

    void SetPointList1( vtkPoints* points );
    void SetPointList2( vtkPoints* points );
    
    void Update();

    // compute pair-wise difference between two point distance matrices.
    // Store the result in a structure other than a point distance matrix
    // because its contents are not regenerated.
    static void ComputePairWiseDifferences( vtkPointDistanceMatrix* firstMatrix, vtkPointDistanceMatrix* secondMatrix, vtkDoubleArray* output );

  protected:
    vtkPointDistanceMatrix();
    ~vtkPointDistanceMatrix();
  private:
    // inputs
    vtkPoints* PointList1;
    vtkPoints* PointList2;

    // outputs
    vtkSmartPointer< vtkDoubleArray > DistanceMatrix;
    vtkTimeStamp MatrixUpdateTime;
    double MaximumDistance;
    double MinimumDistance;

    bool UpdateNeeded();
    bool InputsContainErrors( bool verbose=true );

    void ResetDistances();

		vtkPointDistanceMatrix(const vtkPointDistanceMatrix&); // Not implemented.
		void operator=(const vtkPointDistanceMatrix&); // Not implemented.
};

#endif
