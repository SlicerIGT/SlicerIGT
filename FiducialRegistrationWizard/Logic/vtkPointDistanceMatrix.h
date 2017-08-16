#ifndef __vtkPointDistanceMatrix_h
#define __vtkPointDistanceMatrix_h

#include <vtkDoubleArray.h>
#include <vtkObject.h>
#include <vtkPoints.h>

// At its most basic level, this is a Wrapper class for storing a matrix of
// point to point distances in a vtkDoubleArray.
// The purpose is to improve the abstraction of storing distances, instead of
// memorizing how distances are accessed etc... One can use this class to
// encapsulate that functionality.
// The contents of the matrix are automatically re-generated when either input
// point list is changed.
class vtkPointDistanceMatrix : public vtkObject
{
  public:
    vtkTypeMacro( vtkPointDistanceMatrix, vtkObject );
    void PrintSelf( ostream &os, vtkIndent indent ) VTK_OVERRIDE;
    
    static vtkPointDistanceMatrix* New();
    
    void   SetPointList1( vtkPoints* points );
    void   SetPointList2( vtkPoints* points );
    int    GetPointList1Length();
    int    GetPointList2Length();
    double GetElement( int list1Index, int list2Index );
    double GetMinimumElement();
    void   Update();

    // compute element-wise difference between two point distance matrices.
    // Store the result in a structure other than a point distance matrix
    // because its contents are not regenerated.
    static void ComputeElementWiseDifference( vtkPointDistanceMatrix* firstMatrix, vtkPointDistanceMatrix* secondMatrix, vtkDoubleArray* output );

  protected:
    vtkPointDistanceMatrix();
    ~vtkPointDistanceMatrix();
  private:
    vtkSmartPointer< vtkPoints > pointList1;
    vtkSmartPointer< vtkPoints > pointList2;
    vtkSmartPointer< vtkDoubleArray > distanceMatrix;
    int UpdateNeededFlag; // 1 means the output needs to be updated. 0 means the output is up to date.

		vtkPointDistanceMatrix(const vtkPointDistanceMatrix&); // Not implemented.
		void operator=(const vtkPointDistanceMatrix&); // Not implemented.
};

#endif
