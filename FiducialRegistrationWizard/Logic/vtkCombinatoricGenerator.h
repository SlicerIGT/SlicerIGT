#ifndef __vtkCombinatoricGenerator_h
#define __vtkCombinatoricGenerator_h

// vtk includes
#include <vtkSetGet.h>
#include <vtkObject.h>
#include <vtkTimeStamp.h>

// std includes
#include <vector>

// A class to generate combinatorics from input arrays of integer values
class vtkCombinatoricGenerator : public vtkObject //vtkAlgorithm
{
  public:
    vtkTypeMacro( vtkCombinatoricGenerator, vtkObject );
    static vtkCombinatoricGenerator* New();

    void PrintSelf( ostream &os, vtkIndent indent ) VTK_OVERRIDE;
    void PrintVectorOfVectors( ostream &os, vtkIndent indent, std::vector< std::vector< int > > vectorOfVectors ); // helper
    
    // type of combinatoric to compute
    enum CombinatoricType
    {
      COMBINATORIC_CARTESIAN_PRODUCT = 0,
      COMBINATORIC_COMBINATION,
      COMBINATORIC_PERMUTATION,
      COMBINATORIC_LAST // Valid types go above this line
    };
    void SetCombinatoricToCartesianProduct();
    void SetCombinatoricToCombination();
    void SetCombinatoricToPermutation();
    std::string GetCombinatoricAsString();
    void SetSubsetSize( unsigned int size ); // output set size (for permutation and combination)

    // operations involving number of input sets (mainly for cartesian product)
    void SetNumberOfInputSets( unsigned int numSets );
    unsigned int GetNumberOfInputSets();

    // operations involving entire input sets
    void AddInputSet( std::vector< int > vector );
    void RemoveInputSet( unsigned int setIndex ); // remove the set entirely
    void ClearInputSet( unsigned int setIndex ); // remove contents of set only
    unsigned int GetInputSetSize( unsigned int setIndex );

    // element-wise operations on input sets (useful for python scripting, which does not natively support std::vector)
    void AddInputElement( unsigned int setIndex, int newElement );
    int GetInputElement( unsigned int setIndex, unsigned int elementIndex );

    // output accessors
    unsigned int ComputeNumberOfOutputSets(); // returns the number of sets that *would* be computed on update
    std::vector< std::vector< int > > GetOutputSets(); // returns a deep copy
    unsigned int GetOutputSetSize();
    int GetOutputElement( unsigned int setIndex, unsigned int elementIndex );

    // logic
    void Update();

  protected:
    vtkCombinatoricGenerator();
    ~vtkCombinatoricGenerator();

  private:
    // operation to perform
    CombinatoricType Combinatoric;

    // size of the permutations/combinations
    unsigned int SubsetSize;

    // Internal storage of input sets + output sets
    // vtkIntArray would have been used, but each set (whether it is a tuple or component)
    // could be a different size, and vtkIntArray is incapable of representing this.
    std::vector< std::vector< int > > InputSets;
    std::vector< std::vector< int > > OutputSets;

    // these determine when the output needs to be updated
    vtkTimeStamp InputChangedTime;
    vtkTimeStamp OutputChangedTime;
    bool UpdateNeeded();

    // logic methods for cartesian product computation
    void UpdateCartesianProducts();
    void UpdateCartesianProductsHelper( std::vector< int >& currentProduct, unsigned int& cartesianProductCount ); // recursive helper
    unsigned int NumberOfPossibleCartesianProducts();

    // logic methods for combination computation
    void UpdateCombinations();
    void UpdateCombinationsHelper( unsigned int inputElementIndex, std::vector< int >& currentSubset, unsigned int& combinationCount ); // recursive helper
    unsigned int NumberOfPossibleCombinations();
    
    // logic methods for permutation computation
    void UpdatePermutations();
    void UpdatePermutationsHelper( unsigned int currentSubsetSize, std::vector< int >& baseSet, unsigned int& permutationCount ); // recursive helper
    unsigned int NumberOfPossiblePermutations();
    
    unsigned int Factorial( unsigned int x );

    vtkCombinatoricGenerator(const vtkCombinatoricGenerator&); // Not implemented.
    void operator=(const vtkCombinatoricGenerator&); // Not implemented.
};

#endif
