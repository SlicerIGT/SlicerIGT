#include "vtkCombinatoricGenerator.h"
#include <vtkObjectFactory.h> //for vtkStandardNewMacro() macro

const int MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION = 0; // use only the zeroth set in permutation and combination operations

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkCombinatoricGenerator );

//------------------------------------------------------------------------------
vtkCombinatoricGenerator::vtkCombinatoricGenerator()
{
  this->Combinatoric = COMBINATORIC_COMBINATION;
  this->SubsetSize = 1;
  this->InputChangedTime.Modified();
  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
vtkCombinatoricGenerator::~vtkCombinatoricGenerator()
{
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::PrintSelf( std::ostream &os, vtkIndent indent )
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Combinatoric: " << this->Combinatoric << std::endl;
  os << indent << "Subset size: " << this->SubsetSize << std::endl;
  os << indent << "Number of input sets: " << this->InputSets.size() << std::endl;
  os << indent << "Input sets:" << std::endl;
  this->PrintVectorOfVectors( os, indent, this->InputSets );
  os << indent << "Number of output sets: " << this->OutputSets.size() << std::endl;
  os << indent << "Output sets:" << std::endl;
  this->PrintVectorOfVectors( os, indent, this->OutputSets );
  os << indent << "Update needed: " << ( this->UpdateNeeded() ? "true" : "false" ) << std::endl;
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::PrintVectorOfVectors( std::ostream &os, vtkIndent indent, std::vector< std::vector< int > > vectorOfVectors )
{
  for ( unsigned int setIndex = 0; setIndex < vectorOfVectors.size(); setIndex++ )
  {
    os << indent << "  Set " << setIndex << ":";
    for ( unsigned int elementIndex = 0; elementIndex < vectorOfVectors[ setIndex ].size(); elementIndex++ )
    {
      os << " " << vectorOfVectors[ setIndex ][ elementIndex ];
    }
    os << std::endl;
  }
}

//------------------------------------------------------------------------------
// METHODS FOR COMBINATORIC SETTING
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::SetCombinatoricToCartesianProduct()
{
  Combinatoric = COMBINATORIC_CARTESIAN_PRODUCT;
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::SetCombinatoricToCombination()
{
  Combinatoric = COMBINATORIC_COMBINATION;
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::SetCombinatoricToPermutation()
{
  Combinatoric = COMBINATORIC_PERMUTATION;
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
std::string vtkCombinatoricGenerator::GetCombinatoricAsString()
{
  switch ( this->Combinatoric )
  {
    case COMBINATORIC_CARTESIAN_PRODUCT:
      return "Cartesian Product";
    case COMBINATORIC_PERMUTATION:
      return "Permutation";
    case COMBINATORIC_COMBINATION:
      return "Combination";
    default:
      return "Unknown";
  }
}

//------------------------------------------------------------------------------
// for permutation and combination modes, assign desired output set size
void vtkCombinatoricGenerator::SetSubsetSize( unsigned int size )
{
  if ( size <= 0 )
  {
    vtkWarningMacro( "Cannot set SubsetSize to " << size << ". Must be greater than 0. Will retain original value of " << this->SubsetSize );
  }
  this->SubsetSize = size;
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
// OPERATIONS INVOLVING NUMBER OF INPUT SETS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
unsigned int vtkCombinatoricGenerator::GetNumberOfInputSets()
{
  return this->InputSets.size();
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::SetNumberOfInputSets( unsigned int numSets )
{
  if ( numSets == 0 )
  {
    vtkGenericWarningMacro( "NumSets " << numSets << " is too small, must be greater than 0. Returning." );
    return;
  }
  else if ( numSets < this->InputSets.size() )
  {
    vtkGenericWarningMacro( "NumSets " << numSets << " is smaller than the current value. Existing contents will be discarded." );
  }

  this->InputSets.resize( numSets );
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
// OPERATIONS INVOLVING ENTIRE INPUT SETS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::AddInputSet( std::vector< int > vector )
{
  // add a deep copy
  std::vector< int > vectorCopy;
  vectorCopy.resize( vector.size() );
  for ( unsigned int elementIndex = 0; elementIndex < vector.size(); elementIndex++ )
  {
    vectorCopy[ elementIndex ] = vector[ elementIndex ];
  }
  this->InputSets.push_back( vectorCopy );
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::RemoveInputSet( unsigned int removeSetIndex )
{
  if ( removeSetIndex >= this->InputSets.size() )
  {
    vtkGenericWarningMacro( "There is no removeSetIndex'th input set, removeSetIndex = " << removeSetIndex << ", number of input sets = " << this->InputSets.size() << ". Returning." );
    return;
  }

  // shift everything after removeSetIndex down one position
  for ( unsigned int setIndex = removeSetIndex; setIndex < this->InputSets.size() - 1; setIndex++ )
  {
    this->InputSets[ setIndex ] = this->InputSets[ setIndex + 1 ];
  }
  // remove the now-redundant last position
  this->InputSets.resize( this->InputSets.size() - 1 );
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::ClearInputSet( unsigned int setIndex )
{
  if ( setIndex >= this->InputSets.size() )
  {
    vtkGenericWarningMacro( "There is no setIndex'th set, setIndex = " << setIndex << ", number of input sets = " << this->InputSets.size() << ". Returning." );
    return;
  }

  this->InputSets[ setIndex ].clear();
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
unsigned int vtkCombinatoricGenerator::GetInputSetSize( unsigned int setIndex )
{
  if ( setIndex >= this->InputSets.size() )
  {
    vtkGenericWarningMacro( "There is no setIndex'th input set, setIndex = " << setIndex << ", number of input sets = " << this->InputSets.size() << ". Returning 0." );
    return 0;
  }

  return this->InputSets[ setIndex ].size();
}

//------------------------------------------------------------------------------
// ELEMENT-WISE OPERATIONS ON INPUT SETS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::AddInputElement( unsigned int setIndex, int newElement )
{
  if ( setIndex >= this->InputSets.size() )
  {
    vtkGenericWarningMacro( "There is no setIndex'th set, setIndex = " << setIndex << ", number of input sets = " << this->InputSets.size() << ". Returning." );
    return;
  }

  this->InputSets[ setIndex ].push_back( newElement );
  this->InputChangedTime.Modified();
}

//------------------------------------------------------------------------------
int vtkCombinatoricGenerator::GetInputElement( unsigned int setIndex, unsigned int elementIndex )
{
  if ( setIndex >= this->InputSets.size() )
  {
    vtkGenericWarningMacro( "There is no setIndex'th set, setIndex = " << setIndex << ", number of input sets = " << this->InputSets.size() << ". Returning 0." );
    return 0;
  }

  if ( elementIndex >= this->InputSets[ setIndex ].size() )
  {
    vtkGenericWarningMacro( "There is no elementIndex'th element in the input set, setIndex = " << setIndex << ", elementIndex = " << elementIndex << ", size of set = " << this->InputSets[ setIndex ].size() << ". Returning 0." );
    return 0;
  }

  return this->InputSets[ setIndex ][ elementIndex ];
}

//------------------------------------------------------------------------------
// OUTPUT ACCESSORS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
std::vector< std::vector< int > > vtkCombinatoricGenerator::GetOutputSets()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  // return a deep copy
  std::vector< std::vector< int > > returnValue;
  returnValue.reserve( this->OutputSets.size() );
  for ( unsigned int setIndex = 0; setIndex < this->OutputSets.size(); setIndex++ )
  {
    std::vector< int > innerList;
    innerList.reserve( this->OutputSets[ setIndex ].size() );
    for ( unsigned int elementIndex = 0; elementIndex < this->OutputSets[ setIndex ].size(); elementIndex++ )
    {
      innerList.push_back( this->OutputSets[ setIndex ][ elementIndex ] );
    }
    returnValue.push_back( this->OutputSets[ setIndex ] );
  }
  return returnValue;
}

//------------------------------------------------------------------------------
unsigned int vtkCombinatoricGenerator::ComputeNumberOfOutputSets()
{
  switch ( this->Combinatoric )
  {
    case COMBINATORIC_CARTESIAN_PRODUCT:
    {
      return NumberOfPossibleCartesianProducts();
    }
    case COMBINATORIC_COMBINATION:
    {
      return NumberOfPossibleCombinations();
    }
    case COMBINATORIC_PERMUTATION:
    {
      return NumberOfPossiblePermutations();
    }
    default:
    {
      vtkGenericWarningMacro( "Unknown combinatoric. Returning 0." );
      return 0;
    }
  }
}

//------------------------------------------------------------------------------
unsigned int vtkCombinatoricGenerator::GetOutputSetSize()
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  unsigned int setIndex = 0; // all output sets will be the same size, might as well just report the 0'th set size
  if ( setIndex >= this->OutputSets.size() )
  {
    vtkGenericWarningMacro( "There are no output sets. Returning 0." );
    return 0;
  }

  return this->OutputSets[ setIndex ].size();
}

//------------------------------------------------------------------------------
int vtkCombinatoricGenerator::GetOutputElement( unsigned int setIndex, unsigned int elementIndex )
{
  if ( this->UpdateNeeded() )
  {
    this->Update();
  }

  if ( setIndex >= this->OutputSets.size() )
  {
    vtkGenericWarningMacro( "There is no setIndex'th set, setIndex = " << setIndex << ", number of output sets = " << this->OutputSets.size() << ". Returning 0." );
    return 0;
  }

  if ( elementIndex >= this->OutputSets[ setIndex ].size() )
  {
    vtkGenericWarningMacro( "There is no elementIndex'th element in the output set, setIndex = " << setIndex << ", elementIndex = " << elementIndex << ", size of set = " << this->OutputSets[ setIndex ].size() << ". Returning 0." );
    return 0;
  }

  return this->OutputSets[ setIndex ][ elementIndex ];
}

//------------------------------------------------------------------------------
// LOGIC
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::Update()
{
  if ( !this->UpdateNeeded() )
  {
    return;
  }

  switch ( this->Combinatoric )
  {
    case COMBINATORIC_CARTESIAN_PRODUCT:
    {
      this->UpdateCartesianProducts();
      break;
    }
    case COMBINATORIC_PERMUTATION:
    {
      this->UpdatePermutations();
      break;
    }
    case COMBINATORIC_COMBINATION:
    {
      this->UpdateCombinations();
      break;
    }
    default:
    {
      vtkErrorMacro( "Unknown combinatoric. Cannot update." );
      break;
    }
  }

  this->OutputChangedTime.Modified();
}

//------------------------------------------------------------------------------
bool vtkCombinatoricGenerator::UpdateNeeded()
{
  return ( this->InputChangedTime > this->OutputChangedTime );
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::UpdateCartesianProducts()
{
  // error/warning checking
  if ( !this->OutputSets.empty() )
  {
    vtkGenericWarningMacro( "Output is not empty. Clearing contents." );
    this->OutputSets.clear();
  }

  if ( this->InputSets.size() == 0 )
  {
    vtkGenericWarningMacro( "There is no input. Output will be empty." );
    return;
  }
  
  // size the output appropriately
  unsigned int numberOfPossibleCartesianProducts = this->NumberOfPossibleCartesianProducts();
  this->OutputSets.reserve( numberOfPossibleCartesianProducts );

  // prepare the recursive call
  unsigned int numberOfComputedCartesianProducts = 0;
  std::vector< int > currentProduct; // starts empty
  this->UpdateCartesianProductsHelper( currentProduct, numberOfComputedCartesianProducts );

  // sanity check
  if ( numberOfComputedCartesianProducts != numberOfPossibleCartesianProducts )
  {
    vtkGenericWarningMacro( "Number of computed cartesian products " << numberOfComputedCartesianProducts << " does not match the " <<
                            "number of possible cartesian products " << numberOfPossibleCartesianProducts << ". " <<
                            "This is a bug and results are likely to contain errors. Please report this issue." );
  }
}

//------------------------------------------------------------------------------
unsigned int vtkCombinatoricGenerator::NumberOfPossibleCartesianProducts()
{
  if ( this->InputSets.size() == 0 )
  {
    return 0;
  }

  int numberOfCartesianProducts = 1;
  for ( unsigned int setIndex = 0; setIndex < this->InputSets.size(); setIndex++ )
  {
    numberOfCartesianProducts *= this->InputSets[ setIndex ].size();
  }
  return numberOfCartesianProducts;
}

//------------------------------------------------------------------------------
// Recursive function to iterate through all input sets, and generate the cartesian products.
// Elements are added to the currentProduct variable in each function call.
void vtkCombinatoricGenerator::UpdateCartesianProductsHelper( std::vector< int >& currentProduct, unsigned int& cartesianProductCount )
{
  unsigned int numberOfInputSetsProcessed = currentProduct.size();

  // base case, we've iterated through all input sets. So copy the current product to the output
  if ( numberOfInputSetsProcessed >= this->InputSets.size() )
  {
    std::vector< int > currentProductCopy( numberOfInputSetsProcessed ); // initialize with size numberOfInputSetsProcessed
    for ( unsigned int elementIndex = 0; elementIndex < numberOfInputSetsProcessed; elementIndex++ )
    {
      currentProductCopy[ elementIndex ] = currentProduct[ elementIndex ];
    }
    this->OutputSets.push_back( currentProductCopy );
    cartesianProductCount++;
    return;
  }

  // recursive case, we need to iterate through all products containing each element
  int inputSetIndex = numberOfInputSetsProcessed;
  for ( unsigned int elementIndex = 0; elementIndex < this->InputSets[ inputSetIndex ].size(); elementIndex++ )
  {
    currentProduct.push_back( this->InputSets[ inputSetIndex ][ elementIndex ] );
    this->UpdateCartesianProductsHelper( currentProduct, cartesianProductCount );
    currentProduct.pop_back();
  }
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::UpdateCombinations()
{
  // error/warning checking
  if ( !this->OutputSets.empty() )
  {
    vtkGenericWarningMacro( "Output is not empty. Clearing contents." );
    this->OutputSets.clear();
  }
  
  if ( this->InputSets.size() == 0 )
  {
    vtkGenericWarningMacro( "There is no input. Output will be empty." );
    return;
  }
  
  if ( this->InputSets.size() > 1 )
  {
    vtkGenericWarningMacro( "There are multiple inputs. Only the first input will be used for this operation." );
  }

  // size the output appropriately
  unsigned int numberOfPossibleCombinations = this->NumberOfPossibleCombinations();
  this->OutputSets.reserve( numberOfPossibleCombinations );

  // prepare the recursive call
  unsigned int inputElementIndex = 0; // Traverse the input set from first element to last
  std::vector< int > initialSubset; // empty at the start
  initialSubset.reserve( this->SubsetSize ); // reserve enough space for output sets
  unsigned int numberOfComputedCombinations = 0;
  this->UpdateCombinationsHelper( inputElementIndex, initialSubset, numberOfComputedCombinations );

  // sanity check
  if ( numberOfComputedCombinations != numberOfPossibleCombinations )
  {
    vtkGenericWarningMacro( "Number of computed combinations " << numberOfComputedCombinations << " does not match the " <<
                            "number of possible combinations " << numberOfPossibleCombinations << ". " <<
                            "This is a bug and results are likely to contain errors. Please report this issue." );
  }
}

//------------------------------------------------------------------------------
// conventionally N choose K combinations,
// ( N = input set size, K = subset size )
// The number of combinations is N! / (K! * (N-K)!)
// See: https://en.wikipedia.org/wiki/Combination
unsigned int vtkCombinatoricGenerator::NumberOfPossibleCombinations()
{
  if ( this->InputSets.size() == 0 )
  {
    return 0;
  }

  int setSize = this->GetInputSetSize( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION );
  int setSizeFactorial = Factorial( setSize );

  int subsetSize = this->SubsetSize;
  if ( setSize < subsetSize )
  {
    vtkWarningMacro( "Set size " << setSize << " must be greater than subset size " << subsetSize << ". Will set subset size to " << setSize << " for this computation." );
    subsetSize = setSize;
  }
  int subsetSizeFactorial = Factorial( subsetSize );    
  
  int setSizeMinusSubsetSize = setSize - subsetSize;
  int setSizeMinusSubsetSizeFactorial = Factorial( setSizeMinusSubsetSize );

  int numberOfCombinations = setSizeFactorial / ( subsetSizeFactorial * setSizeMinusSubsetSizeFactorial );
  return numberOfCombinations;
}

//------------------------------------------------------------------------------
// this recursive function traverses the input set from beginning to end,
// and creates all combinations of the input list containing exactly N (SubsetSize) elements.
// The combination will either contain element at index elementIndex, or it won't.
void vtkCombinatoricGenerator::UpdateCombinationsHelper( unsigned int elementIndex, std::vector< int >& currentSubset, unsigned int& combinationCount )
{
  unsigned int currentSubsetSize = currentSubset.size();
  unsigned int maximumSubsetSize = this->SubsetSize;

  // Base cases
  // The subset is complete, and should be added to the output
  if ( currentSubsetSize == maximumSubsetSize )
  {
    std::vector< int > currentSubsetCopy( maximumSubsetSize ); // initialize with size maximumSubsetSize
    for ( unsigned int elementIndex = 0; elementIndex < maximumSubsetSize; elementIndex++ )
    {
      int element = currentSubset[ elementIndex ];
      currentSubsetCopy[ elementIndex ] = element;
    }
    this->OutputSets.push_back( currentSubsetCopy );
    combinationCount++;
    return;
  }

  // We're out of elements in the input set, so return without adding the subset to the output
  if ( elementIndex == this->GetInputSetSize( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION ) )
  {
    return;
  }

  // Recursive cases
  // *Does* contain the input set element at elementIndex:
  currentSubset.push_back( this->GetInputElement( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION, elementIndex ) );
  this->UpdateCombinationsHelper( elementIndex + 1, currentSubset, combinationCount );
  currentSubset.pop_back(); // housekeeping

  // *Does not* contain the input set element at elementIndex:
  this->UpdateCombinationsHelper( elementIndex + 1, currentSubset, combinationCount );
}

//------------------------------------------------------------------------------
void vtkCombinatoricGenerator::UpdatePermutations()
{
  // error/warning checking
  if ( !this->OutputSets.empty() )
  {
    vtkGenericWarningMacro( "Output is not empty. Clearing contents." );
    this->OutputSets.clear();
  }
  
  if ( this->InputSets.size() == 0 )
  {
    vtkGenericWarningMacro( "There is no input. Output will be empty." );
    return;
  }

  if ( this->InputSets.size() > 1 )
  {
    vtkGenericWarningMacro( "There are multiple inputs. Only the set with index = " << MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION << " will be used for this operation." );
  }

  // size the output appropriately
  unsigned int numberOfPossiblePermutations = this->NumberOfPossiblePermutations();
  this->OutputSets.reserve( numberOfPossiblePermutations );

  // prepare the recursive call
  unsigned int numberOfComputedPermutations = 0; // variable is modified in place by the function below.
  std::vector< int > workingCopyOfMainInputSet;
  workingCopyOfMainInputSet.resize( this->GetInputSetSize( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION ) );
  for ( unsigned int elementIndex = 0; elementIndex < this->GetInputSetSize( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION ); elementIndex++ )
  {
    workingCopyOfMainInputSet[ elementIndex ] = this->GetInputElement( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION, elementIndex );
  }
  unsigned int initialSubsetSize = 0; // nothing has been processed yet, start at 0
  this->UpdatePermutationsHelper( initialSubsetSize, workingCopyOfMainInputSet, numberOfComputedPermutations );

  // sanity check
  if ( numberOfComputedPermutations != numberOfPossiblePermutations )
  {
    vtkGenericWarningMacro( "Number of computed permutations " << numberOfComputedPermutations << " does not match the " <<
                            "number of possible permutations " << numberOfPossiblePermutations << ". " <<
                            "This is a bug and results are likely to contain errors. Please report this issue." );
  }
}

//------------------------------------------------------------------------------
// we're actually interested in the number of K-permutations on a set of N elements
// ( N = input set size, K = subset size )
// The number of K-permutations is N! / ( N - K )!
// See: https://en.wikipedia.org/wiki/Permutation#k-permutations_of_n
unsigned int vtkCombinatoricGenerator::NumberOfPossiblePermutations()
{

  if ( this->InputSets.size() == 0 )
  {
    return 0;
  }

  int setSize = this->GetInputSetSize( MAIN_SET_INDEX_FOR_PERMUTATION_AND_COMBINATION );
  int setSizeFactorial = Factorial( setSize );

  int subsetSize = this->SubsetSize;
  if ( setSize < subsetSize )
  {
    vtkWarningMacro( "Input set size " << setSize << " must be greater than subset size " << subsetSize << ". Will set subset size to " << setSize << " for this computation." );
    subsetSize = setSize;
  }

  int setSizeMinusSubsetSize = setSize - subsetSize;
  int setSizeMinusSubsetSizeFactorial = Factorial( setSizeMinusSubsetSize );

  int numberOfPermutations = setSizeFactorial / setSizeMinusSubsetSizeFactorial;
  return numberOfPermutations;
}

//------------------------------------------------------------------------------
// This recursive function generates all possible N-permutations (N == SubsetSize) of the base set.
// (Note: Base set is a working copy of the input set. This was done to abstract some of the input storage details out from this method.)
// Output sets are constructed by one element at a time each time this function is called.
// The permutation is actually done in-place on the InputSet
void vtkCombinatoricGenerator::UpdatePermutationsHelper( unsigned int currentSubsetSize, std::vector< int >& baseSet, unsigned int& permutationCount )
{
  // Base case, subset is complete... just copy to the output
  unsigned int maximumSubsetSize = this->SubsetSize;
  if ( currentSubsetSize == maximumSubsetSize )
  {
    std::vector< int > baseSetCopyUpToSubsetSize( maximumSubsetSize ); // initialize with size maximumSubsetSize
    for ( unsigned int elementIndex = 0; elementIndex < maximumSubsetSize; elementIndex++ )
    {
      int element = baseSet[ elementIndex ];
      baseSetCopyUpToSubsetSize[ elementIndex ] = element;
    }
    this->OutputSets.push_back( baseSetCopyUpToSubsetSize );
    permutationCount++;
    return;
  }

  // Recursive case, try keeping element where it is, and...
  this->UpdatePermutationsHelper( currentSubsetSize + 1, baseSet, permutationCount );
  
  // also generate permutations by swapping the front value with each of the
  // following values. This will be done in-place on the input set.
  for ( unsigned int swapIndex = currentSubsetSize + 1; swapIndex < baseSet.size(); swapIndex++ )
  {
    // get the relevant indices and values _before_ swapping
    unsigned int frontIndex = currentSubsetSize;
    int frontElement = baseSet[ currentSubsetSize ];
    int swapElement = baseSet[ swapIndex ];

    // swap the values
    baseSet[ swapIndex ] = frontElement;
    baseSet[ frontIndex ] = swapElement;

    // recursive step
    this->UpdatePermutationsHelper( currentSubsetSize + 1, baseSet, permutationCount );

    baseSet[ frontIndex ] = frontElement;
    baseSet[ swapIndex ] = swapElement;
  }
}

//------------------------------------------------------------------------------
unsigned int vtkCombinatoricGenerator::Factorial( unsigned int x )
{
  if ( x == 0 )
  {
    return 1;
  }

  unsigned int factorial = 1;
  for ( unsigned int multiplier = 2; multiplier <= x; multiplier++ )
  {
    factorial *= multiplier;
  }
  return factorial;
}
