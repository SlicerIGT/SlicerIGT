/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImplicitPolyDataDistanceWithClosestPointCoordinate.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImplicitPolyDataDistanceWithClosestPointCoordinate
// .SECTION Description
//
// Implicit function that computes the distance from a point x to the
// nearest point p on an input vtkPolyData. The sign of the function
// is set to the sign of the dot product between the angle-weighted
// pseudonormal at the nearest surface point and the vector x - p.
// Points interior to the geometry have a negative distance, points on
// the exterior have a positive distance, and points on the input
// vtkPolyData have a distance of zero. The gradient of the function
// is the angle-weighted pseudonormal at the nearest point.
//
// Baerentzen, J. A. and Aanaes, H. (2005). Signed distance
// computation using the angle weighted pseudonormal. IEEE
// Transactions on Visualization and Computer Graphics, 11:243-253.
//
// This code was contributed in the VTK Journal paper:
// "Boolean Operations on Surfaces in VTK Without External Libraries"
// by Cory Quammen, Chris Weigle C., Russ Taylor
// http://hdl.handle.net/10380/3262
// http://www.midasjournal.org/browse/publication/797

#ifndef vtkImplicitPolyDataDistanceWithClosestPointCoordinate_h
#define vtkImplicitPolyDataDistanceWithClosestPointCoordinate_h

#include "vtkFiltersCoreModule.h" // For export macro
#include "vtkImplicitFunction.h"

class vtkCellLocator;
class vtkPolyData;

class VTKFILTERSCORE_EXPORT vtkImplicitPolyDataDistanceWithClosestPointCoordinate : public vtkImplicitFunction
{
public:
  static vtkImplicitPolyDataDistanceWithClosestPointCoordinate *New();
  vtkTypeMacro(vtkImplicitPolyDataDistanceWithClosestPointCoordinate,vtkImplicitFunction);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Return the MTime also considering the Input dependency.
  unsigned long GetMTime();

  // Description:
  // Evaluate plane equation of nearest triangle to point x[3].
  double EvaluateFunction(double x[3]);

  // Description:
  // Evaluate plane equation of nearest triangle to point x[3] and provides the position of the closest point.
  double EvaluateFunction(double x[3], double cp[3]);

  // Description:
  // Evaluate function gradient of nearest triangle to point x[3].
  void EvaluateGradient(double x[3], double g[3]);

  // Description:
  // Set the input vtkPolyData used for the implicit function
  // evaluation.  Passes input through an internal instance of
  // vtkTriangleFilter to remove vertices and lines, leaving only
  // triangular polygons for evaluation as implicit planes.
  void SetInput(vtkPolyData *input);

  // Description:
  // Set/get the function value to use if no input vtkPolyData
  // specified.
  vtkSetMacro(NoValue, double);
  vtkGetMacro(NoValue, double);

  // Description:
  // Set/get the function gradient to use if no input vtkPolyData
  // specified.
  vtkSetVector3Macro(NoGradient, double);
  vtkGetVector3Macro(NoGradient, double);

  // Description:
  // Set/get the tolerance usued for the locator.
  vtkGetMacro(Tolerance, double);
  vtkSetMacro(Tolerance, double);

  // Description:
  // Set/Get a spatial locator for speeding up the search process.
  // An instance of vtkCellLocator is used by default.
  void SetLocator(vtkCellLocator *locator);
  vtkGetObjectMacro(Locator, vtkCellLocator);
  
protected:
  vtkImplicitPolyDataDistanceWithClosestPointCoordinate();
  ~vtkImplicitPolyDataDistanceWithClosestPointCoordinate();

  // Description:
  // Release locator
  void ReleaseLocator(void);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator(void);

  double SharedEvaluate(double x[3], double n[3], double cp[3]);

  double NoValue;
  double NoGradient[3];
  double Tolerance;

  vtkPolyData       *Input;
  vtkCellLocator    *Locator;

private:
  vtkImplicitPolyDataDistanceWithClosestPointCoordinate(const vtkImplicitPolyDataDistanceWithClosestPointCoordinate&);  // Not implemented.
  void operator=(const vtkImplicitPolyDataDistanceWithClosestPointCoordinate&);  // Not implemented.
};

#endif
