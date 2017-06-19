#include <vtkCardinalSpline.h>
#include <vtkKochanekSpline.h>
#include <vtkSplineFilter.h>
#include <vtkTubeFilter.h>

namespace CreateCurveUtil
{

  static const int NUMBER_OF_LINE_POINTS_MIN = 2;

  // Generates the curve model from the markups connecting consecutive segments.
  // Each segment can be linear, cardinal or Kochanek Splines (described and implemented in UpdateOutputCurveModel, UpdateOutputLinearModel
  // and UpdateOutputHermiteSplineModel methods). Uses Tube radius and clean markups option from the module node.
  void UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* outputPolyData);
  // Generates the linear curve model connecting linear tubes from each markup.
  void UpdateOutputLinearModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkPolyData* outputPolyData);
  // Generates Cardinal Spline curve model.
  void UpdateOutputCardinalSplineModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkPolyData* outputPolyData);
  // Generates Kochanek Spline curve model.
  void UpdateOutputKochanekSplineModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkPolyData* outputPolyData);
  // Generates a polynomial curve model.
  void UpdateOutputPolynomialFitModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters, vtkPolyData* outputPolyData);
  // Assign parameter values to points based on their position in the markups list (good for ordered point sets)
  void ComputePointParametersRawIndices(vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters);
  // Assign parameter values to points based on their position in a minimum spanning tree between the two farthest points (good for unordered point sets)
  void ComputePointParametersMinimumSpanningTree(vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters);

  void MarkupsToPoints(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* outputPoints);

  //------------------------------------------------------------------------------
  void UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* pNode, vtkPolyData* outputPolyData)
  {
    vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
    if (markupsNode == NULL)
    {
      return;
    }

    int numberOfMarkups = markupsNode->GetNumberOfFiducials();
    if (numberOfMarkups < NUMBER_OF_LINE_POINTS_MIN) // check this here, but also perform redundant checks elsewhere
    {
      return;
    }

    vtkSmartPointer< vtkPoints > controlPoints = vtkSmartPointer< vtkPoints >::New();
    MarkupsToPoints(pNode, controlPoints);

    switch (pNode->GetInterpolationType())
    {
      case vtkMRMLMarkupsToModelNode::Linear:
      {
        UpdateOutputLinearModel(pNode, controlPoints, outputPolyData);
        break;
      }
      case vtkMRMLMarkupsToModelNode::CardinalSpline:
      {
        UpdateOutputCardinalSplineModel(pNode, controlPoints, outputPolyData);
        break;
      }
      case vtkMRMLMarkupsToModelNode::KochanekSpline:
      {
        UpdateOutputKochanekSplineModel(pNode, controlPoints, outputPolyData);
        break;
      }
      case vtkMRMLMarkupsToModelNode::Polynomial:
      {
        vtkSmartPointer<vtkDoubleArray> pointParameters = vtkSmartPointer<vtkDoubleArray>::New();
        switch (pNode->GetPointParameterType())
        {
          case vtkMRMLMarkupsToModelNode::RawIndices:
          {
            ComputePointParametersRawIndices(controlPoints, pointParameters);
            break;
          }
          case vtkMRMLMarkupsToModelNode::MinimumSpanningTree:
          {
            ComputePointParametersMinimumSpanningTree(controlPoints, pointParameters);
            break;
          }
          default:
          {
            vtkGenericWarningMacro("Invalid PointParameterType: " << pNode->GetPointParameterType() << ". Using raw indices.");
            ComputePointParametersRawIndices(controlPoints, pointParameters);
            break;
          }
        }
        UpdateOutputPolynomialFitModel(pNode, controlPoints, pointParameters, outputPolyData);
        break;
      }
    }
  }

  //------------------------------------------------------------------------------
  void AllocateCurvePoints(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPoints* outputPoints)
  {
    // Number of points is different depending on whether the curve is a loop
    int numberControlPoints = controlPoints->GetNumberOfPoints();
    int numberOutputPointsPerControlPoint = pNode->GetTubeSegmentsBetweenControlPoints();
    if (pNode->GetTubeLoop())
    {
      outputPoints->SetNumberOfPoints(numberControlPoints * numberOutputPointsPerControlPoint + 2);
      // two extra points are required to "close off" the loop, and ensure that the tube appears fully continuous
    }
    else
    {
      outputPoints->SetNumberOfPoints((numberControlPoints - 1) * numberOutputPointsPerControlPoint + 1);
    }
  }

  //------------------------------------------------------------------------------
  void CloseLoop(vtkPoints* outputPoints)
  {
    // If looped, move the first point and add an *extra* point. This is 
    // needed in order for the curve to be continuous, otherwise the tube ends won't 
    // align properly
    double point0[3];
    outputPoints->GetPoint(0, point0);
    double point1[3];
    outputPoints->GetPoint(1, point1);
    double finalPoint[3];
    finalPoint[0] = point0[0] * 0.5 + point1[0] * 0.5;
    finalPoint[1] = point0[1] * 0.5 + point1[1] * 0.5;
    finalPoint[2] = point0[2] * 0.5 + point1[2] * 0.5;
    outputPoints->SetPoint(0, finalPoint);
    int finalIndex = outputPoints->GetNumberOfPoints() - 1;
    outputPoints->SetPoint(finalIndex, finalPoint);
  }

  //------------------------------------------------------------------------------
  void SetCardinalSplineParameters(vtkMRMLMarkupsToModelNode* pNode, vtkCardinalSpline* splineX, vtkCardinalSpline* splineY, vtkCardinalSpline* splineZ)
  {
    if (pNode->GetTubeLoop())
    {
      splineX->ClosedOn();
      splineY->ClosedOn();
      splineZ->ClosedOn();
    }
    vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
    int numberMarkups = markupsNode->GetNumberOfFiducials();
    for (int i = 0; i < numberMarkups; i++)
    {
      double point[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNthFiducialPosition(i, point);
      splineX->AddPoint(i, point[0]);
      splineY->AddPoint(i, point[1]);
      splineZ->AddPoint(i, point[2]);
    }
  }

  //------------------------------------------------------------------------------
  void SetKochanekSplineParameters(vtkMRMLMarkupsToModelNode* pNode, vtkKochanekSpline* splineX, vtkKochanekSpline* splineY, vtkKochanekSpline* splineZ)
  {
    if (pNode->GetTubeLoop())
    {
      splineX->ClosedOn();
      splineY->ClosedOn();
      splineZ->ClosedOn();
    }
    splineX->SetDefaultBias(pNode->GetKochanekBias());
    splineY->SetDefaultBias(pNode->GetKochanekBias());
    splineZ->SetDefaultBias(pNode->GetKochanekBias());
    splineX->SetDefaultContinuity(pNode->GetKochanekContinuity());
    splineY->SetDefaultContinuity(pNode->GetKochanekContinuity());
    splineZ->SetDefaultContinuity(pNode->GetKochanekContinuity());
    splineX->SetDefaultTension(pNode->GetKochanekTension());
    splineY->SetDefaultTension(pNode->GetKochanekTension());
    splineZ->SetDefaultTension(pNode->GetKochanekTension());
    vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
    int numberMarkups = markupsNode->GetNumberOfFiducials();
    for (int i = 0; i < numberMarkups; i++)
    {
      double point[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNthFiducialPosition(i, point);
      splineX->AddPoint(i, point[0]);
      splineY->AddPoint(i, point[1]);
      splineZ->AddPoint(i, point[2]);
    }
    if (pNode->GetKochanekEndsCopyNearestDerivatives())
    {
      // manually set the derivative to the nearest value
      // (difference between the two nearest points). The
      // constraint mode is set to 1, this tells the spline
      // class to use our manual definition.
      // left derivative
      double point0[3];
      markupsNode->GetNthFiducialPosition(0, point0);
      double point1[3];
      markupsNode->GetNthFiducialPosition(1, point1);
      splineX->SetLeftConstraint(1);
      splineX->SetLeftValue(point1[0] - point0[0]);
      splineY->SetLeftConstraint(1);
      splineY->SetLeftValue(point1[1] - point0[1]);
      splineZ->SetLeftConstraint(1);
      splineZ->SetLeftValue(point1[2] - point0[2]);
      // right derivative
      double pointNMinus2[3];
      markupsNode->GetNthFiducialPosition(numberMarkups - 2, pointNMinus2);
      double pointNMinus1[3];
      markupsNode->GetNthFiducialPosition(numberMarkups - 1, pointNMinus1);
      splineX->SetRightConstraint(1);
      splineX->SetRightValue(pointNMinus1[0] - pointNMinus2[0]);
      splineY->SetRightConstraint(1);
      splineY->SetRightValue(pointNMinus1[1] - pointNMinus2[1]);
      splineZ->SetRightConstraint(1);
      splineZ->SetRightValue(pointNMinus1[2] - pointNMinus2[2]);
    }
    else
    {
      // This ("0") is the most simple mode for end derivative computation, 
      // described by documentation as using the "first/last two points".
      // Use this as the default because others would require setting the 
      // derivatives manually
      splineX->SetLeftConstraint(0);
      splineY->SetLeftConstraint(0);
      splineZ->SetLeftConstraint(0);
      splineX->SetRightConstraint(0);
      splineY->SetRightConstraint(0);
      splineZ->SetRightConstraint(0);
    }
  }

  //------------------------------------------------------------------------------
  void GetTubePolyDataFromPoints(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* pointsToConnect, vtkPolyData* outputTube)
  {
    int numPoints = pointsToConnect->GetNumberOfPoints();

    vtkSmartPointer< vtkCellArray > lineCellArray = vtkSmartPointer< vtkCellArray >::New();
    lineCellArray->InsertNextCell(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
      lineCellArray->InsertCellPoint(i);
    }

    vtkSmartPointer< vtkPolyData > linePolyData = vtkSmartPointer< vtkPolyData >::New();
    linePolyData->Initialize();
    linePolyData->SetPoints(pointsToConnect);
    linePolyData->SetLines(lineCellArray);

    vtkSmartPointer< vtkTubeFilter> tubeSegmentFilter = vtkSmartPointer< vtkTubeFilter>::New();
    tubeSegmentFilter->SetInputData(linePolyData);

    tubeSegmentFilter->SetRadius(pNode->GetTubeRadius());
    tubeSegmentFilter->SetNumberOfSides(pNode->GetTubeNumberOfSides());
    tubeSegmentFilter->CappingOn();
    tubeSegmentFilter->Update();

    outputTube->DeepCopy(tubeSegmentFilter->GetOutput());
  }

  //------------------------------------------------------------------------------
  void UpdateOutputLinearModel(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPolyData* outputTubePolyData)
  {
    if (controlPoints == NULL)
    {
      vtkGenericWarningMacro("Control points data structure is null. No model generated.");
      return;
    }

    int numberControlPoints = controlPoints->GetNumberOfPoints();
    // redundant error checking, to be safe
    if (numberControlPoints < NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Not enough points to create an output spline model. Need at least " << NUMBER_OF_LINE_POINTS_MIN << " points but " << numberControlPoints << " are provided. No output created.");
      return;
    }

    vtkSmartPointer< vtkPoints > curvePoints = vtkSmartPointer< vtkPoints >::New();
    AllocateCurvePoints(pNode, controlPoints, curvePoints);

    // Iterate over the segments to interpolate, add all the "in-between" points
    int numberSegmentsToInterpolate;
    if (pNode->GetTubeLoop())
    {
      numberSegmentsToInterpolate = numberControlPoints;
    }
    else
    {
      numberSegmentsToInterpolate = numberControlPoints - 1;
    }
    int numberCurvePointsPerControlPoint = pNode->GetTubeSegmentsBetweenControlPoints();
    int controlPointIndex = 0;
    int controlPointIndexNext = 1;
    while (controlPointIndex < numberSegmentsToInterpolate)
    {
      // find the two points to interpolate between
      double controlPointCurrent[3];
      controlPoints->GetPoint(controlPointIndex, controlPointCurrent);
      if (controlPointIndexNext >= numberControlPoints)
      {
        controlPointIndexNext = 0;
      }
      double controlPointNext[3];
      controlPoints->GetPoint(controlPointIndexNext, controlPointNext);
      // iterate to compute interpolating points
      for (int i = 0; i < numberCurvePointsPerControlPoint; i++)
      {
        double interpolationParam = i / (double)numberCurvePointsPerControlPoint;
        double curvePoint[3];
        curvePoint[0] = (1.0 - interpolationParam) * controlPointCurrent[0] + interpolationParam * controlPointNext[0];
        curvePoint[1] = (1.0 - interpolationParam) * controlPointCurrent[1] + interpolationParam * controlPointNext[1];
        curvePoint[2] = (1.0 - interpolationParam) * controlPointCurrent[2] + interpolationParam * controlPointNext[2];
        int curveIndex = controlPointIndex * numberCurvePointsPerControlPoint + i;
        curvePoints->SetPoint(curveIndex, curvePoint);
      }
      controlPointIndex++;
      controlPointIndexNext++;
    }
    // bring it the rest of the way to the final control point
    controlPointIndex = controlPointIndex % numberControlPoints; // if the index exceeds the max, bring back to 0
    double finalPoint[3] = { 0.0, 0.0, 0.0 };
    controlPoints->GetPoint(controlPointIndex, finalPoint);
    int finalIndex = numberCurvePointsPerControlPoint * numberSegmentsToInterpolate;
    curvePoints->SetPoint(finalIndex, finalPoint);

    // the last part of the curve depends on whether it is a loop or not
    if (pNode->GetTubeLoop())
    {
      CloseLoop(curvePoints);
    }

    GetTubePolyDataFromPoints(pNode, curvePoints, outputTubePolyData);
  }

  //------------------------------------------------------------------------------
  void UpdateOutputCardinalSplineModel(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPolyData* outputTubePolyData)
  {
    if (controlPoints == NULL)
    {
      vtkGenericWarningMacro("Control points data structure is null. No model generated.");
      return;
    }

    int numberControlPoints = controlPoints->GetNumberOfPoints();
    // redundant error checking, to be safe
    if (numberControlPoints < NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Not enough points to create an output spline model. Need at least " << NUMBER_OF_LINE_POINTS_MIN << " points but " << numberControlPoints << " are provided. No output created.");
      return;
    }

    // special case, fit a line. Spline fitting will not work with fewer than 3 points
    if (numberControlPoints == NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Only " << NUMBER_OF_LINE_POINTS_MIN << " provided. Fitting line.");
      UpdateOutputLinearModel(pNode, controlPoints, outputTubePolyData);
      return;
    }

    // Create the splines
    vtkSmartPointer< vtkCardinalSpline > splineX = vtkSmartPointer< vtkCardinalSpline >::New();
    vtkSmartPointer< vtkCardinalSpline > splineY = vtkSmartPointer< vtkCardinalSpline >::New();
    vtkSmartPointer< vtkCardinalSpline > splineZ = vtkSmartPointer< vtkCardinalSpline >::New();
    SetCardinalSplineParameters(pNode, splineX, splineY, splineZ);

    vtkSmartPointer< vtkPoints > curvePoints = vtkSmartPointer< vtkPoints >::New();
    AllocateCurvePoints(pNode, controlPoints, curvePoints);

    // Iterate over the segments to interpolate, add all the "in-between" points
    int numberSegmentsToInterpolate;
    if (pNode->GetTubeLoop())
    {
      numberSegmentsToInterpolate = numberControlPoints;
    }
    else
    {
      numberSegmentsToInterpolate = numberControlPoints - 1;
    }
    int numberCurvePointsPerControlPoint = pNode->GetTubeSegmentsBetweenControlPoints();
    int controlPointIndex = 0;
    int controlPointIndexNext = 1;
    while (controlPointIndex < numberSegmentsToInterpolate)
    {
      // iterate to compute interpolating points
      for (int i = 0; i < numberCurvePointsPerControlPoint; i++)
      {
        double interpolationParam = controlPointIndex + i / (double)numberCurvePointsPerControlPoint;
        double curvePoint[3];
        curvePoint[0] = splineX->Evaluate(interpolationParam);
        curvePoint[1] = splineY->Evaluate(interpolationParam);
        curvePoint[2] = splineZ->Evaluate(interpolationParam);
        int curveIndex = controlPointIndex * numberCurvePointsPerControlPoint + i;
        curvePoints->SetPoint(curveIndex, curvePoint);
      }
      controlPointIndex++;
      controlPointIndexNext++;
    }
    // bring it the rest of the way to the final control point
    controlPointIndex = controlPointIndex % numberControlPoints; // if the index exceeds the max, bring back to 0
    double finalPoint[3] = { 0.0, 0.0, 0.0 };
    controlPoints->GetPoint(controlPointIndex, finalPoint);
    int finalIndex = numberCurvePointsPerControlPoint * numberSegmentsToInterpolate;
    curvePoints->SetPoint(finalIndex, finalPoint);

    // the last part of the curve depends on whether it is a loop or not
    if (pNode->GetTubeLoop())
    {
      CloseLoop(curvePoints);
    }

    GetTubePolyDataFromPoints(pNode, curvePoints, outputTubePolyData);
  }

  //------------------------------------------------------------------------------
  void UpdateOutputKochanekSplineModel(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPolyData* outputTubePolyData)
  {
    if (controlPoints == NULL)
    {
      vtkGenericWarningMacro("Control points data structure is null. No model generated.");
      return;
    }

    int numberControlPoints = controlPoints->GetNumberOfPoints();
    // redundant error checking, to be safe
    if (numberControlPoints < NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Not enough points to create an output spline model. Need at least " << NUMBER_OF_LINE_POINTS_MIN << " points but " << numberControlPoints << " are provided. No output created.");
      return;
    }

    // special case, fit a line. Spline fitting will not work with fewer than 3 points
    if (numberControlPoints == NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Only " << NUMBER_OF_LINE_POINTS_MIN << " provided. Fitting line.");
      UpdateOutputLinearModel(pNode, controlPoints, outputTubePolyData);
      return;
    }

    // Create the splines
    vtkSmartPointer< vtkKochanekSpline > splineX = vtkSmartPointer< vtkKochanekSpline >::New();
    vtkSmartPointer< vtkKochanekSpline > splineY = vtkSmartPointer< vtkKochanekSpline >::New();
    vtkSmartPointer< vtkKochanekSpline > splineZ = vtkSmartPointer< vtkKochanekSpline >::New();
    SetKochanekSplineParameters(pNode, splineX, splineY, splineZ);

    vtkSmartPointer< vtkPoints > curvePoints = vtkSmartPointer< vtkPoints >::New();
    AllocateCurvePoints(pNode, controlPoints, curvePoints);

    // Iterate over the segments to interpolate, add all the "in-between" points
    int numberSegmentsToInterpolate;
    if (pNode->GetTubeLoop())
    {
      numberSegmentsToInterpolate = numberControlPoints;
    }
    else
    {
      numberSegmentsToInterpolate = numberControlPoints - 1;
    }
    int numberCurvePointsPerControlPoint = pNode->GetTubeSegmentsBetweenControlPoints();
    int controlPointIndex = 0;
    int controlPointIndexNext = 1;
    while (controlPointIndex < numberSegmentsToInterpolate)
    {
      // iterate to compute interpolating points
      for (int i = 0; i < numberCurvePointsPerControlPoint; i++)
      {
        double interpolationParam = controlPointIndex + i / (double)numberCurvePointsPerControlPoint;
        double curvePoint[3];
        curvePoint[0] = splineX->Evaluate(interpolationParam);
        curvePoint[1] = splineY->Evaluate(interpolationParam);
        curvePoint[2] = splineZ->Evaluate(interpolationParam);
        int curveIndex = controlPointIndex * numberCurvePointsPerControlPoint + i;
        curvePoints->SetPoint(curveIndex, curvePoint);
      }
      controlPointIndex++;
      controlPointIndexNext++;
    }
    // bring it the rest of the way to the final control point
    controlPointIndex = controlPointIndex % numberControlPoints; // if the index exceeds the max, bring back to 0
    double finalPoint[3] = { 0.0, 0.0, 0.0 };
    controlPoints->GetPoint(controlPointIndex, finalPoint);
    int finalIndex = numberCurvePointsPerControlPoint * numberSegmentsToInterpolate;
    curvePoints->SetPoint(finalIndex, finalPoint);

    // the last part of the curve depends on whether it is a loop or not
    if (pNode->GetTubeLoop())
    {
      CloseLoop(curvePoints);
    }

    GetTubePolyDataFromPoints(pNode, curvePoints, outputTubePolyData);
  }

  //------------------------------------------------------------------------------
  void ComputePointParametersRawIndices(vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters)
  {
    if (controlPoints == NULL)
    {
      vtkGenericWarningMacro("Markups contains a vtkPoints object that is null.");
      return;
    }

    int numPoints = controlPoints->GetNumberOfPoints();
    // redundant error checking, to be safe
    if (numPoints < NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Not enough points to compute polynomial parameters. Need at least " << NUMBER_OF_LINE_POINTS_MIN << " points but " << numPoints << " are provided.");
      return;
    }

    if (markupsPointsParameters->GetNumberOfTuples())
    {
      // this should never happen, but in case it does, output a warning
      vtkGenericWarningMacro("markupsPointsParameters already has contents. Clearing.");
      while (markupsPointsParameters->GetNumberOfTuples()) // clear contents just in case
        markupsPointsParameters->RemoveLastTuple();
    }

    for (int v = 0; v < numPoints; v++)
    {
      markupsPointsParameters->InsertNextTuple1(v / double(numPoints - 1));
      // division to clamp all values to range 0.0 - 1.0
    }
  }

  //------------------------------------------------------------------------------
  void ComputePointParametersMinimumSpanningTree(vtkPoints * controlPoints, vtkDoubleArray* markupsPointsParameters)
  {
    if (controlPoints == NULL)
    {
      vtkGenericWarningMacro("Markups contains a vtkPoints object that is null.");
      return;
    }

    int numPoints = controlPoints->GetNumberOfPoints();
    // redundant error checking, to be safe
    if (numPoints < NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Not enough points to compute polynomial parameters. Need at least " << NUMBER_OF_LINE_POINTS_MIN << " points but " << numPoints << " are provided.");
      return;
    }

    // vtk boost algorithms cannot be used because they are not built with 3D Slicer
    // so this is a custom implementation of:
    // 1. constructing an undirected graph as a 2D array
    // 2. Finding the two vertices that are the farthest apart
    // 3. running prim's algorithm on the graph
    // 4. extract the "trunk" path from the last vertex to the first
    // 5. based on the distance along that path, assign each vertex a polynomial parameter value

    // in the following code, two tasks are done:
    // 1. construct an undirected graph
    std::vector< double > distances(numPoints * numPoints);
    distances.assign(numPoints * numPoints, 0.0);
    // 2. find the two farthest-seperated vertices in the distances array
    int treeStartIndex = 0;
    int treeEndIndex = 0;
    double maximumDistance = 0;
    // iterate through all points
    for (int v = 0; v < numPoints; v++)
    {
      for (int u = 0; u < numPoints; u++)
      {
        double pointU[3], pointV[3];
        controlPoints->GetPoint(u, pointU);
        controlPoints->GetPoint(v, pointV);
        double distX = (pointU[0] - pointV[0]); double distXsq = distX * distX;
        double distY = (pointU[1] - pointV[1]); double distYsq = distY * distY;
        double distZ = (pointU[2] - pointV[2]); double distZsq = distZ * distZ;
        double dist3D = sqrt(distXsq + distYsq + distZsq);
        distances[v * numPoints + u] = dist3D;
        if (dist3D > maximumDistance)
        {
          maximumDistance = dist3D;
          treeStartIndex = v;
          treeEndIndex = u;
        }
      }
    }
    // use the 1D vector as a 2D vector
    std::vector< double* > graph(numPoints);
    for (int v = 0; v < numPoints; v++)
    {
      graph[v] = &(distances[v * numPoints]);
    }

    // implementation of Prim's algorithm heavily based on:
    // http://www.geeksforgeeks.org/greedy-algorithms-set-5-prims-minimum-spanning-tree-mst-2/
    std::vector< int > parent(numPoints); // Array to store constructed MST
    std::vector< double > key(numPoints);   // Key values used to pick minimum weight edge in cut
    std::vector< bool > mstSet(numPoints);  // To represent set of vertices not yet included in MST

    // Initialize all keys as INFINITE (or at least as close as we can get)
    for (int i = 0; i < numPoints; i++)
    {
      key[i] = VTK_DOUBLE_MAX;
      mstSet[i] = false;
    }

    // Always include first 1st vertex in MST.
    key[treeStartIndex] = 0.0;     // Make key 0 so that this vertex is picked as first vertex
    parent[treeStartIndex] = -1; // First node is always root of MST 

    // The MST will have numPoints vertices
    for (int count = 0; count < numPoints - 1; count++)
    {
      // Pick the minimum key vertex from the set of vertices
      // not yet included in MST
      int nextPointIndex = -1;
      double minDistance = VTK_DOUBLE_MAX, min_index;
      for (int v = 0; v < numPoints; v++)
      {
        if (mstSet[v] == false && key[v] < minDistance)
        {
          minDistance = key[v];
          nextPointIndex = v;
        }
      }

      // Add the picked vertex to the MST Set
      mstSet[nextPointIndex] = true;

      // Update key value and parent index of the adjacent vertices of
      // the picked vertex. Consider only those vertices which are not yet
      // included in MST
      for (int v = 0; v < numPoints; v++)
      {
        // graph[u][v] is non zero only for adjacent vertices of m
        // mstSet[v] is false for vertices not yet included in MST
        // Update the key only if graph[u][v] is smaller than key[v]
        if (graph[nextPointIndex][v] >= 0 &&
          mstSet[v] == false &&
          graph[nextPointIndex][v] < key[v])
        {
          parent[v] = nextPointIndex;
          key[v] = graph[nextPointIndex][v];
        }
      }
    }

    // determine the "trunk" path of the tree, from first index to last index
    std::vector< int > pathIndices;
    int currentPathIndex = treeEndIndex;
    while (currentPathIndex != -1)
    {
      pathIndices.push_back(currentPathIndex);
      currentPathIndex = parent[currentPathIndex]; // go up the tree one layer
    }

    // find the sum of distances along the trunk path of the tree
    double sumOfDistances = 0.0;
    for (int i = 0; i < pathIndices.size() - 1; i++)
    {
      sumOfDistances += graph[i][i + 1];
    }

    // check this to prevent a division by zero (in case all points are duplicates)
    if (sumOfDistances == 0)
    {
      vtkGenericWarningMacro("Minimum spanning tree path has distance zero. No parameters will be assigned. Check inputs.");
      return;
    }

    // find the parameters along the trunk path of the tree
    std::vector< double > pathParameters;
    double currentDistance = 0.0;
    for (int i = 0; i < pathIndices.size() - 1; i++)
    {
      pathParameters.push_back(currentDistance / sumOfDistances);
      currentDistance += graph[i][i + 1];
    }
    pathParameters.push_back(currentDistance / sumOfDistances); // this should be 1.0

    // finally assign polynomial parameters to each point, and store in the output array
    if (markupsPointsParameters->GetNumberOfTuples() > 0)
    {
      // this should never happen, but in case it does, output a warning
      vtkGenericWarningMacro("markupsPointsParameters already has contents. Clearing.");
      while (markupsPointsParameters->GetNumberOfTuples()) // clear contents just in case
        markupsPointsParameters->RemoveLastTuple();
    }

    for (int i = 0; i < numPoints; i++)
    {
      int currentIndex = i;
      bool alongPath = false;
      int indexAlongPath = -1;
      for (int j = 0; j < pathIndices.size(); j++)
      {
        if (pathIndices[j] == currentIndex)
        {
          alongPath = true;
          indexAlongPath = j;
          break;
        }
      }
      while (!alongPath)
      {
        currentIndex = parent[currentIndex];
        for (int j = 0; j < pathIndices.size(); j++)
        {
          if (pathIndices[j] == currentIndex)
          {
            alongPath = true;
            indexAlongPath = j;
            break;
          }
        }
      }
      markupsPointsParameters->InsertNextTuple1(pathParameters[indexAlongPath]);
    }
  }

  //------------------------------------------------------------------------------
  void UpdateOutputPolynomialFitModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters, vtkPolyData* outputPolyData)
  {
    if (controlPoints == NULL)
    {
      vtkGenericWarningMacro("Markups contains a vtkPoints object that is null. No model generated.");
      return;
    }

    int numPoints = controlPoints->GetNumberOfPoints();
    // redundant error checking, to be safe
    if (numPoints < NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Not enough points to compute a polynomial fit. Need at least " << NUMBER_OF_LINE_POINTS_MIN << " points but " << numPoints << " are provided. No output created.");
      return;
    }

    // special case, fit a line. The polynomial solver does not work with only 2 points.
    if (numPoints == NUMBER_OF_LINE_POINTS_MIN)
    {
      vtkGenericWarningMacro("Only " << NUMBER_OF_LINE_POINTS_MIN << " provided. Fitting line.");
      UpdateOutputLinearModel(markupsToModelModuleNode, controlPoints, outputPolyData);
      return;
    }

    // check size of point parameters array for consistency
    if (markupsPointsParameters->GetNumberOfTuples() != numPoints)
    {
      vtkGenericWarningMacro("Incorrect number of point parameters provided. Should have " << numPoints << " parameters (one for each point), but " << markupsPointsParameters->GetNumberOfTuples() << " are provided. No output created.");
      return;
    }

    // The system of equations using high-order polynomials is not well-conditioned.
    // The vtkMath implementation will usually abort with polynomial orders higher than 9.
    // Since there is also numerical instability, we decide to limit the polynomial order to 6.
    // If order higher than 6 is needed on a global fit, then another algorithm should be considered anyway.
    // If at some point we want to add support for higher order polynomials, then here are two options (from Andras):
    // 1. VNL. While the VNL code is more sophisticated, and I guess also more stable, you would probably need to
    //    limit the number of samples and normalize data that you pass to the LSQR solver to be able to compute 
    //    higher-order fits (see for example this page for related discussion:
    //    http://digital.ni.com/public.nsf/allkb/45C2016C23B3B0298525645F0073B828). 
    //    See an example how VNL is used in Plus:
    //    https://app.assembla.com/spaces/plus/subversion/source/HEAD/trunk/PlusLib/src/PlusCommon/PlusMath.cxx#ln111
    // 2. Mathematica uses different basis functions for polynomial fitting (shifted Chebyshev polynomials) instead 
    //    of basis functions that are simple powers of a variable to make the fitting more robust (the source code
    //    is available here: http://library.wolfram.com/infocenter/MathSource/6780/).
    int polynomialOrder = markupsToModelModuleNode->GetPolynomialOrder();
    const int maximumPolynomialOrder = 6;
    if (polynomialOrder > maximumPolynomialOrder)
    {
      vtkGenericWarningMacro("Desired polynomial order " << polynomialOrder << " is not supported. "
        << "Maximum polynomial order is " << maximumPolynomialOrder << ". "
        << "Will attempt to create polynomial order " << maximumPolynomialOrder << " instead.");
      polynomialOrder = maximumPolynomialOrder;
    }

    int numPolynomialCoefficients = polynomialOrder + 1;
    // special case, if polynomial is underdetermined, change the order of the polynomial
    std::set<double> uniquePointParameters;
    for (int i = 0; i < numPoints; i++)
      uniquePointParameters.insert(markupsPointsParameters->GetValue(i));
    int numUniquePointParameters = uniquePointParameters.size();
    if (numUniquePointParameters < numPolynomialCoefficients)
    {
      vtkGenericWarningMacro("Not enough points to compute a polynomial fit. " << "For an order " << polynomialOrder << " polynomial, at least " << numPolynomialCoefficients << " points with unique parameters are needed. "
        << numUniquePointParameters << " points with unique parameters were found. "
        << "An order " << (numUniquePointParameters - 1) << " polynomial will be created instead.");
      numPolynomialCoefficients = numUniquePointParameters;
    }

    // independent values (parameter along the curve)
    int numIndependentValues = numPoints * numPolynomialCoefficients;
    std::vector<double> independentValues(numIndependentValues); // independent values
    independentValues.assign(numIndependentValues, 0.0);
    for (int c = 0; c < numPolynomialCoefficients; c++) // o = degree index
    {
      for (int p = 0; p < numPoints; p++) // p = point index
      {
        double value = std::pow(markupsPointsParameters->GetValue(p), c);
        independentValues[p * numPolynomialCoefficients + c] = value;
      }
    }
    std::vector<double*> independentMatrix(numPoints);
    independentMatrix.assign(numPoints, NULL);
    for (int p = 0; p < numPoints; p++)
    {
      independentMatrix[p] = &(independentValues[p * numPolynomialCoefficients]);
    }
    double** independentMatrixPtr = &(independentMatrix[0]);

    // dependent values
    const int numDimensions = 3; // this should never be changed from 3
    int numDependentValues = numPoints * numDimensions;
    std::vector<double> dependentValues(numDependentValues); // dependent values
    dependentValues.assign(numDependentValues, 0.0);
    for (int p = 0; p < numPoints; p++) // p = point index
    {
      double* currentPoint = controlPoints->GetPoint(p);
      for (int d = 0; d < numDimensions; d++) // d = dimension index
      {
        double value = currentPoint[d];
        dependentValues[p * numDimensions + d] = value;
      }
    }
    std::vector<double*> dependentMatrix(numPoints);
    dependentMatrix.assign(numPoints, NULL);
    for (int p = 0; p < numPoints; p++)
    {
      dependentMatrix[p] = &(dependentValues[p * numDimensions]);
    }
    double** dependentMatrixPtr = &(dependentMatrix[0]);

    // solution to least squares
    int totalNumberCoefficients = numDimensions*numPolynomialCoefficients;
    std::vector<double> coefficientValues(totalNumberCoefficients);
    coefficientValues.assign(totalNumberCoefficients, 0.0);
    std::vector<double*> coefficientMatrix(numPolynomialCoefficients);
    for (int c = 0; c < numPolynomialCoefficients; c++)
    {
      coefficientMatrix[c] = &(coefficientValues[c * numDimensions]);
    }
    double** coefficientMatrixPtr = &(coefficientMatrix[0]); // the solution

    // Input the forumulation into SolveLeastSquares
    vtkMath::SolveLeastSquares(numPoints, independentMatrixPtr, numPolynomialCoefficients, dependentMatrixPtr, numDimensions, coefficientMatrixPtr);

    // Use the values to generate points along the polynomial curve
    vtkSmartPointer<vtkPoints> smoothedPoints = vtkSmartPointer<vtkPoints>::New(); // points
    vtkSmartPointer< vtkCellArray > smoothedLines = vtkSmartPointer<  vtkCellArray >::New(); // lines
    int numPointsOnCurve = (numPoints - 1) * markupsToModelModuleNode->GetTubeSegmentsBetweenControlPoints() + 1;
    smoothedLines->InsertNextCell(numPointsOnCurve); // one long continuous line
    for (int p = 0; p < numPointsOnCurve; p++) // p = point index
    {
      double pointMm[3];
      for (int d = 0; d < numDimensions; d++)
      {
        pointMm[d] = 0.0;
        for (int c = 0; c < numPolynomialCoefficients; c++)
        {
          double coefficient = coefficientValues[c * numDimensions + d];
          pointMm[d] += coefficient * std::pow((double(p) / (numPointsOnCurve - 1)), c);
        }
      }
      smoothedPoints->InsertPoint(p, pointMm);
      smoothedLines->InsertCellPoint(p);
    }

    // Convert the points to a tube model
    vtkSmartPointer< vtkPolyData >smoothedSegments = vtkSmartPointer< vtkPolyData >::New();
    smoothedSegments->Initialize();
    smoothedSegments->SetPoints(smoothedPoints);
    smoothedSegments->SetLines(smoothedLines);

    vtkSmartPointer< vtkTubeFilter> tubeFilter = vtkSmartPointer< vtkTubeFilter>::New();
    tubeFilter->SetInputData(smoothedSegments);
    tubeFilter->SetRadius(markupsToModelModuleNode->GetTubeRadius());
    tubeFilter->SetNumberOfSides(markupsToModelModuleNode->GetTubeNumberOfSides());
    tubeFilter->CappingOn();
    tubeFilter->Update();

    outputPolyData->DeepCopy(tubeFilter->GetOutput());
  }

  //------------------------------------------------------------------------------
  void MarkupsToPoints(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* outputPoints)
  {
    vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
    int  numberOfMarkups = markupsNode->GetNumberOfFiducials();
    outputPoints->SetNumberOfPoints(numberOfMarkups);
    double markupPoint[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < numberOfMarkups; i++)
    {
      markupsNode->GetNthFiducialPosition(i, markupPoint);
      outputPoints->SetPoint(i, markupPoint);
    }

    if (pNode->GetCleanMarkups())
    {
      vtkSmartPointer< vtkPolyData > polyData = vtkSmartPointer< vtkPolyData >::New();
      polyData->Initialize();
      polyData->SetPoints(outputPoints);

      vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New();
      cleanPointPolyData->SetInputData(polyData);
      cleanPointPolyData->SetTolerance(CLEAN_POLYDATA_TOLERANCE_MM);
      cleanPointPolyData->Update();
    }
  }

} // namespace CreateClosedSurfaceUtil
