#include <vtkButterflySubdivisionFilter.h>
#include <vtkCubeSource.h>
#include <vtkDelaunay3D.h>
#include <vtkGlyph3D.h>
#include <vtkLineSource.h>
#include <vtkOBBTree.h>
#include <vtkRegularPolygonSource.h>
#include <vtkLinearSubdivisionFilter.h>

namespace CreateClosedSurfaceUtil
{

  static const double COMPARE_TO_ZERO_TOLERANCE = 0.0001;

  enum PointArrangement
  {
    POINT_ARRANGEMENT_SINGULAR = 0,
    POINT_ARRANGEMENT_LINEAR,
    POINT_ARRANGEMENT_PLANAR,
    POINT_ARRANGEMENT_NONPLANAR,
    POINT_ARRANGEMENT_LAST // do not set to this type, insert valid types above this line
  };

  // Generates the closed surface from the markups using vtkDelaunay3D. Uses Delanauy alpha value, subdivision filter and clean markups
  // options from the module node.
  void UpdateOutputCloseSurfaceModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* outputPolyData);

  // Compute the best fit plane through the points, as well as the major and minor axes which describe variation in points.
  void ComputeTransformMatrixFromBoundingAxes(vtkPoints* points, vtkMatrix4x4* transformFromBoundingAxes);

  // Compute the range of points along the specified axes (total lengths along which points appear)
  void ComputeTransformedExtentRanges(vtkPoints* points, vtkMatrix4x4* transformMatrix, double outputExtentRanges[3]);

  // Compute the amount to extrude surfaces when closed surface is linear or planar.
  double ComputeSurfaceExtrusionAmount(const double extents[3]);

  // Find out what kind of arrangment the points are in (see PointArrangementEnum above).
  // If the arrangement is planar, stores the normal of the best fit plane in planeNormal.
  // If the arrangement is linear, stores the axis of the best fit line in lineAxis.
  PointArrangement ComputePointArrangement(const double smallestBoundingExtentRanges[3]);

  // helper utility functions
  void SetNthColumnInMatrix(vtkMatrix4x4* matrix, int n, const double axis[3]);
  void GetNthColumnInMatrix(vtkMatrix4x4* matrix, int n, double outputAxis[3]);

  //------------------------------------------------------------------------------
  // Implementation

  //------------------------------------------------------------------------------
  void UpdateOutputCloseSurfaceModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* output)
  {
    if (markupsToModelModuleNode == NULL)
    {
      vtkGenericWarningMacro("No markupsToModelModuleNode provided to UpdateOutputCloseSurfaceModel. No operation performed.");
      return;
    }
    if (output == NULL)
    {
      vtkGenericWarningMacro("No output poly data provided to UpdateOutputCloseSurfaceModel. No operation performed.");
      return;
    }
    vtkMRMLMarkupsFiducialNode* markups = markupsToModelModuleNode->GetMarkupsNode();
    if (markups == NULL)
    {
      return; // The output will remain empty
    }


    int numberOfMarkups = markups->GetNumberOfFiducials();

    if (numberOfMarkups == 0)
    {
      return; // The output will remain empty
    }

    bool outputContainsData = output->GetNumberOfPoints();
    if (outputContainsData)
    {
      vtkGenericWarningMacro("Output poly data provided to UpdateOutputCloseSurfaceModel already contains data. Existing data will be overwritten.");
    }

    output->Reset(); // empty the poly data of all existing data

    vtkSmartPointer< vtkPoints > inputPoints = vtkSmartPointer< vtkPoints >::New();
    inputPoints->SetNumberOfPoints(numberOfMarkups);

    vtkSmartPointer< vtkCellArray > inputCellArray = vtkSmartPointer< vtkCellArray >::New();
    inputCellArray->InsertNextCell(numberOfMarkups);

    double markupPoint[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < numberOfMarkups; i++)
    {
      markups->GetNthFiducialPosition(i, markupPoint);
      inputCellArray->InsertCellPoint(i);
      inputPoints->SetPoint(i, markupPoint);
    }

    vtkSmartPointer< vtkPolyData > inputPolyData = vtkSmartPointer< vtkPolyData >::New();
    inputPolyData->SetLines(inputCellArray);
    inputPolyData->SetPoints(inputPoints);

    vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New();
    cleanPointPolyData->SetInputData(inputPolyData);
    cleanPointPolyData->SetTolerance(CLEAN_POLYDATA_TOLERANCE_MM);
    if (markupsToModelModuleNode->GetCleanMarkups())
    {
      cleanPointPolyData->SetPointMerging(1);
    }
    else
    {
      cleanPointPolyData->SetPointMerging(0);
    }
    cleanPointPolyData->Update();

    vtkPolyData* cleanedPolyData = cleanPointPolyData->GetOutput();
    // a lot of operations (vtkOBBTree, computing bounds) seem to fail on vtkPolyData, so we need vtkPoints
    vtkPoints* cleanedPoints = cleanedPolyData->GetPoints();

    vtkSmartPointer< vtkDelaunay3D > delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
    delaunay->SetAlpha(markupsToModelModuleNode->GetDelaunayAlpha());
    delaunay->AlphaTrisOff();
    delaunay->AlphaLinesOff();
    delaunay->AlphaVertsOff();

    vtkSmartPointer< vtkMatrix4x4 > boundingAxesToRasTransformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    ComputeTransformMatrixFromBoundingAxes(cleanedPoints, boundingAxesToRasTransformMatrix);

    vtkSmartPointer< vtkMatrix4x4 > rasToBoundingAxesTransformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
    vtkMatrix4x4::Invert(boundingAxesToRasTransformMatrix, rasToBoundingAxesTransformMatrix);

    double smallestBoundingExtentRanges[3] = { 0.0, 0.0, 0.0 }; // temporary values
    ComputeTransformedExtentRanges(cleanedPoints, rasToBoundingAxesTransformMatrix, smallestBoundingExtentRanges);

    PointArrangement pointArrangement = ComputePointArrangement(smallestBoundingExtentRanges);

    switch (pointArrangement)
    {
    case POINT_ARRANGEMENT_SINGULAR:
    {
      vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
      double extrusionMagnitude = ComputeSurfaceExtrusionAmount(smallestBoundingExtentRanges); // need to give some depth
      cubeSource->SetBounds(-extrusionMagnitude, extrusionMagnitude,
        -extrusionMagnitude, extrusionMagnitude,
        -extrusionMagnitude, extrusionMagnitude);

      vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
      glyph->SetSourceConnection(cubeSource->GetOutputPort());
      glyph->SetInputConnection(cleanPointPolyData->GetOutputPort());
      glyph->Update();

      delaunay->SetInputConnection(glyph->GetOutputPort());

      break;
    }
    case POINT_ARRANGEMENT_LINEAR:
    {
      // draw a "square" around the line (make it a rectangular prism)
      vtkSmartPointer<vtkRegularPolygonSource> squareSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
      squareSource->SetCenter(0.0, 0.0, 0.0);
      double extrusionMagnitude = ComputeSurfaceExtrusionAmount(smallestBoundingExtentRanges); // need to give some depth
      squareSource->SetRadius(extrusionMagnitude);
      squareSource->SetNumberOfSides(4);
      double lineAxis[3] = { 0.0, 0.0, 0.0 }; // temporary values
      const int LINE_AXIS_INDEX = 0; // The largest (and only meaningful) axis is in the 0th column
      // the bounding axes are stored in the columns of transformFromBoundingAxes
      GetNthColumnInMatrix(boundingAxesToRasTransformMatrix, LINE_AXIS_INDEX, lineAxis);
      squareSource->SetNormal(lineAxis);

      vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
      glyph->SetSourceConnection(squareSource->GetOutputPort());
      glyph->SetInputConnection(cleanPointPolyData->GetOutputPort());
      glyph->Update();

      delaunay->SetInputConnection(glyph->GetOutputPort());

      break;
    }
    case POINT_ARRANGEMENT_PLANAR:
    {
      // extrude additional points on either side of the plane
      vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
      double planeNormal[3] = { 0.0, 0.0, 0.0 }; // temporary values
      const int PLANE_NORMAL_INDEX = 2; // The plane normal has the smallest variation, and is stored in the last column
      // the bounding axes are stored in the columns of transformFromBoundingAxes
      GetNthColumnInMatrix(boundingAxesToRasTransformMatrix, PLANE_NORMAL_INDEX, planeNormal);
      double extrusionMagnitude = ComputeSurfaceExtrusionAmount(smallestBoundingExtentRanges); // need to give some depth
      double point1[3] = { planeNormal[0], planeNormal[1], planeNormal[2] };
      vtkMath::MultiplyScalar(point1, extrusionMagnitude);
      lineSource->SetPoint1(point1);
      double point2[3] = { planeNormal[0], planeNormal[1], planeNormal[2] };
      vtkMath::MultiplyScalar(point2, -extrusionMagnitude);
      lineSource->SetPoint2(point2);

      vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
      glyph->SetSourceConnection(lineSource->GetOutputPort());
      glyph->SetInputConnection(cleanPointPolyData->GetOutputPort());
      glyph->Update();

      delaunay->SetInputConnection(glyph->GetOutputPort());

      break;
    }
    case POINT_ARRANGEMENT_NONPLANAR:
    {
      delaunay->SetInputConnection(cleanPointPolyData->GetOutputPort());
      break;
    }
    default: // unsupported or invalid
    {
      vtkGenericWarningMacro("Unsupported pointArrangementType detected: " << pointArrangement << ". Aborting closed surface generation.");
      return;
    }
    }

    vtkSmartPointer< vtkDataSetSurfaceFilter > surfaceFilter = vtkSmartPointer< vtkDataSetSurfaceFilter >::New();
    surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
    surfaceFilter->Update();

    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetFeatureAngle(100); // TODO: This needs some justification

    if (markupsToModelModuleNode->GetButterflySubdivision() &&
      pointArrangement == POINT_ARRANGEMENT_NONPLANAR)
    {
      vtkSmartPointer< vtkButterflySubdivisionFilter > subdivisionFilter = vtkSmartPointer< vtkButterflySubdivisionFilter >::New();
      subdivisionFilter->SetInputConnection(surfaceFilter->GetOutputPort());
      subdivisionFilter->SetNumberOfSubdivisions(3);
      subdivisionFilter->Update();
      if (markupsToModelModuleNode->GetConvexHull())
      {
        vtkSmartPointer< vtkDelaunay3D > convexHull = vtkSmartPointer< vtkDelaunay3D >::New();
        convexHull->SetInputConnection(subdivisionFilter->GetOutputPort());
        convexHull->Update();
        vtkSmartPointer< vtkDataSetSurfaceFilter > surfaceFilter = vtkSmartPointer< vtkDataSetSurfaceFilter >::New();
        surfaceFilter->SetInputData(convexHull->GetOutput());
        surfaceFilter->Update();
        normals->SetInputConnection(surfaceFilter->GetOutputPort());
      }
      else
      {
        normals->SetInputConnection(subdivisionFilter->GetOutputPort());
      }
    }
    else
    {
      vtkNew<vtkLinearSubdivisionFilter> linearSubdivision;
      linearSubdivision->SetInputConnection(surfaceFilter->GetOutputPort());
      normals->SetInputConnection(linearSubdivision->GetOutputPort());
    }
    normals->Update();
    output->DeepCopy(normals->GetOutput());
  }

  //------------------------------------------------------------------------------
  // Compute the principal axes of the point cloud. The x axis represents the axis
  // with maximum variation, and the z axis has minimum variation.
  // This function is currently implemented using the vtkOBBTree object.
  // There are two limitations with this approach:
  // 1. vtkOBBTree may have a performance impact
  // 2. The axes returned are based on variation of coordinates, not the range
  //    (so the return result is not necessarily intuitive, variation != length).
  // Neither of these limitations will prevent the overall logic from functioning
  // correctly, but it is worth keeping in mind, and worth changing should a need 
  // arise
  void ComputeTransformMatrixFromBoundingAxes(vtkPoints* points, vtkMatrix4x4* boundingAxesToRasTransformMatrix)
  {
    if (points == NULL)
    {
      vtkGenericWarningMacro("Points object is null. Cannot compute best fit planes.");
      return;
    }

    if (boundingAxesToRasTransformMatrix == NULL)
    {
      vtkGenericWarningMacro("Output matrix object is null. Cannot compute best fit planes.");
      return;
    }

    // the output matrix should start as identity, so no translation etc.
    boundingAxesToRasTransformMatrix->Identity();

    // Compute the plane using the smallest bounding box that can have arbitrary axes
    vtkSmartPointer<vtkOBBTree> obbTree = vtkSmartPointer<vtkOBBTree>::New();
    double cornerOBBOrigin[3] = { 0.0, 0.0, 0.0 }; // unused
    double variationMaximumOBBAxis[3] = { 0.0, 0.0, 0.0 };
    double variationMediumOBBAxis[3] = { 0.0, 0.0, 0.0 };
    double variationMinimumOBBAxis[3] = { 0.0, 0.0, 0.0 };
    double relativeAxisSizes[3] = { 0.0, 0.0, 0.0 }; // unused, the values represented herein are unclear
    obbTree->ComputeOBB(points, cornerOBBOrigin, variationMaximumOBBAxis, variationMediumOBBAxis, variationMinimumOBBAxis, relativeAxisSizes);

    // now to store the desired results in the appropriate axis of the output matrix.
    // must check each axis to make sure it was actually computed (non-zero)
    // do the maxmimum variation axis
    if (vtkMath::Norm(variationMaximumOBBAxis) < COMPARE_TO_ZERO_TOLERANCE)
    {
      // there is no variation in the points whatsoever.
      // i.e. all points are in a single position.
      // return arbitrary orthonormal axes (the standard axes will do).
      boundingAxesToRasTransformMatrix->Identity();
      return;
    }
    vtkMath::Normalize(variationMaximumOBBAxis);
    SetNthColumnInMatrix(boundingAxesToRasTransformMatrix, 0, variationMaximumOBBAxis);

    // do the medium variation axis
    if (vtkMath::Norm(variationMediumOBBAxis) < COMPARE_TO_ZERO_TOLERANCE)
    {
      // the points are colinear along only the maximum axis
      // any two perpendicular orthonormal vectors will do for the remaining axes.
      double thetaAngle = 0.0; // this can be arbitrary
      vtkMath::Perpendiculars(variationMaximumOBBAxis, variationMediumOBBAxis, variationMinimumOBBAxis, thetaAngle);
    }
    vtkMath::Normalize(variationMediumOBBAxis);
    SetNthColumnInMatrix(boundingAxesToRasTransformMatrix, 1, variationMediumOBBAxis);

    // do the minimum variation axis
    if (vtkMath::Norm(variationMinimumOBBAxis) < COMPARE_TO_ZERO_TOLERANCE)
    {
      // all points lie exactly on a plane.
      // the remaining perpendicular vector found using cross product.
      vtkMath::Cross(variationMaximumOBBAxis, variationMediumOBBAxis, variationMinimumOBBAxis);
    }
    vtkMath::Normalize(variationMinimumOBBAxis);
    SetNthColumnInMatrix(boundingAxesToRasTransformMatrix, 2, variationMinimumOBBAxis);
  }

  //------------------------------------------------------------------------------
  // It is assumed that sortedExtentRanges is pre-sorted in descending order (largest to smallest)
  PointArrangement ComputePointArrangement(const double sortedExtentRanges[3])
  {
    if (sortedExtentRanges == NULL)
    {
      vtkGenericWarningMacro("Input sortedExtentRanges is null. Returning singularity result.");
      return POINT_ARRANGEMENT_SINGULAR;
    }

    double longestExtentRange = sortedExtentRanges[0];
    double mediumExtentRange = sortedExtentRanges[1];
    double shortestExtentRange = sortedExtentRanges[2];

    // sanity checking
    bool longestExtentSmallerThanMedium = longestExtentRange >= COMPARE_TO_ZERO_TOLERANCE && longestExtentRange < mediumExtentRange;
    bool longestExtentSmallerThanShortest = longestExtentRange >= COMPARE_TO_ZERO_TOLERANCE && longestExtentRange < shortestExtentRange;
    bool mediumExtentSmallerThanShortest = mediumExtentRange >= COMPARE_TO_ZERO_TOLERANCE && mediumExtentRange < shortestExtentRange;
    if (longestExtentSmallerThanMedium || longestExtentSmallerThanShortest || mediumExtentSmallerThanShortest)
    {
      // Don't correct the problem here. Code external to this function should pass
      // extent ranges already sorted, so it indicates a problem elsewhere.
      vtkGenericWarningMacro("Extent ranges not provided in order largest to smallest. Unexpected results may occur.");
    }

    if (longestExtentRange < COMPARE_TO_ZERO_TOLERANCE)
    {
      return POINT_ARRANGEMENT_SINGULAR;
    }

    // We need to compare relative lengths of the short and medium axes against
    // the longest axis.
    double mediumToLongestRatio = mediumExtentRange / longestExtentRange;

    // The Delaunay3D class tends to fail with thin planes/lines, so it is important
    // to capture these cases, even liberally. It was experimentally determined that
    // extents less than 1/10th of the maximum extent tend to produce errors.
    const double RATIO_THRESHOLD = 0.1;

    if (mediumToLongestRatio < RATIO_THRESHOLD)
    {
      return POINT_ARRANGEMENT_LINEAR;
    }

    double shortestToLongestRatio = shortestExtentRange / longestExtentRange;
    if (shortestToLongestRatio < RATIO_THRESHOLD)
    {
      return POINT_ARRANGEMENT_PLANAR;
    }

    return POINT_ARRANGEMENT_NONPLANAR;
  }

  //------------------------------------------------------------------------------
  void ComputeTransformedExtentRanges(vtkPoints* points, vtkMatrix4x4* transformMatrix, double outputExtentRanges[3])
  {
    if (points == NULL)
    {
      vtkGenericWarningMacro("points is null. Aborting output extent computation.");
      return;
    }

    if (transformMatrix == NULL)
    {
      vtkGenericWarningMacro("transformMatrix is null. Aborting output extent computation.");
      return;
    }

    if (outputExtentRanges == NULL)
    {
      vtkGenericWarningMacro("outputExtentRanges is null. Aborting output extent computation.");
      return;
    }

    vtkSmartPointer< vtkTransform > transform = vtkSmartPointer< vtkTransform >::New();
    transform->SetMatrix(transformMatrix);
    transform->Update();

    // can't transform points directly, so need to store in a container
    vtkSmartPointer< vtkPolyData > polyDataWithPoints = vtkSmartPointer< vtkPolyData >::New();
    polyDataWithPoints->SetPoints(points);

    vtkSmartPointer< vtkTransformFilter > transformFilter = vtkSmartPointer< vtkTransformFilter >::New();
    transformFilter->SetTransform(transform);
    transformFilter->SetInputData(polyDataWithPoints);
    transformFilter->Update();

    // the extent can be extracted from the output points object (poly data bounds does not work)
    vtkPoints* transformedPoints = transformFilter->GetPolyDataOutput()->GetPoints();
    transformedPoints->ComputeBounds();
    double* extents = transformedPoints->GetBounds(); // { xmin, xmax, ymin, ymax, zmin, zmax }

    for (int i = 0; i < 3; i++)
    {
      double axisIMin = extents[2 * i];
      double axisIMax = extents[2 * i + 1];
      double axisIRange = axisIMax - axisIMin;
      outputExtentRanges[i] = axisIRange;
    }
  }

  //------------------------------------------------------------------------------
  double ComputeSurfaceExtrusionAmount(const double extents[3])
  {
    // MINIMUM_SURFACE_EXTRUSION_AMOUNT is the value returned by default, and the final result cannot be less than this.
    const double MINIMUM_SURFACE_EXTRUSION_AMOUNT = 0.01;
    if (extents == NULL)
    {
      vtkGenericWarningMacro("extents is null. Returning MINIMUM_SURFACE_EXTRUSION_AMOUNT: " << MINIMUM_SURFACE_EXTRUSION_AMOUNT << ".");
      return MINIMUM_SURFACE_EXTRUSION_AMOUNT;
    }

    double normOfExtents = vtkMath::Norm(extents);
    const double SURFACE_EXTRUSION_NORM_MULTIPLIER = 0.01; // this value is observed to produce generally acceptable results
    double surfaceExtrusionAmount = normOfExtents * SURFACE_EXTRUSION_NORM_MULTIPLIER;

    if (surfaceExtrusionAmount < MINIMUM_SURFACE_EXTRUSION_AMOUNT)
    {
      vtkGenericWarningMacro("Surface extrusion amount smaller than " << MINIMUM_SURFACE_EXTRUSION_AMOUNT << " : " << surfaceExtrusionAmount << ". "
        << "Consider checking the points for singularity. Setting surface extrusion amount to default "
        << MINIMUM_SURFACE_EXTRUSION_AMOUNT << ".");
      surfaceExtrusionAmount = MINIMUM_SURFACE_EXTRUSION_AMOUNT;
    }
    return surfaceExtrusionAmount;
  }

  //------------------------------------------------------------------------------
  void SetNthColumnInMatrix(vtkMatrix4x4* matrix, int n, const double axis[3])
  {
    if (matrix == NULL)
    {
      vtkGenericWarningMacro("No matrix provided as input. No operation performed.");
      return;
    }

    if (n < 0 || n >= 3)
    {
      vtkGenericWarningMacro("Axis n " << n << " is out of bounds. Valid values are 0, 1, and 2. No operation performed.");
      return;
    }

    if (axis == NULL)
    {
      vtkGenericWarningMacro("Axis is null. No operation performed.");
      return;
    }

    matrix->SetElement(0, n, axis[0]);
    matrix->SetElement(1, n, axis[1]);
    matrix->SetElement(2, n, axis[2]);
  }

  //------------------------------------------------------------------------------
  void GetNthColumnInMatrix(vtkMatrix4x4* matrix, int n, double outputAxis[3])
  {
    if (matrix == NULL)
    {
      vtkGenericWarningMacro("No matrix provided as input. No operation performed.");
      return;
    }

    if (n < 0 || n >= 3)
    {
      vtkGenericWarningMacro("Axis n " << n << " is out of bounds. Valid values are 0, 1, and 2. No operation performed.");
      return;
    }

    if (outputAxis == NULL)
    {
      vtkGenericWarningMacro("Axis is null. No operation performed.");
      return;
    }

    outputAxis[0] = matrix->GetElement(0, n);
    outputAxis[1] = matrix->GetElement(1, n);
    outputAxis[2] = matrix->GetElement(2, n);
  }

} // namespace CreateClosedSurfaceUtil
