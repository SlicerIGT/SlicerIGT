/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MarkupsToModel Logic includes
#include "vtkSlicerMarkupsToModelLogic.h"

// MRML includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSelectionNode.h"
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkButterflySubdivisionFilter.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkCubeSource.h>
#include <vtkDoubleArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDelaunay3D.h>
#include <vtkGlyph3D.h>
#include <vtkIntArray.h>
#include <vtkKochanekSpline.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkOBBTree.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyDataNormals.h>
#include <vtkSphereSource.h>
#include <vtkSplineFilter.h>
#include <vtkTubeFilter.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <cassert>
#include <cmath>
#include <vector>
#include <set>

#define LINE_MIN_NUMBER_POINTS 2

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsToModelLogic);

//----------------------------------------------------------------------------
vtkSlicerMarkupsToModelLogic::vtkSlicerMarkupsToModelLogic()
{
  this->ImportingScene=0;
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsToModelLogic::~vtkSlicerMarkupsToModelLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLMarkupsToModelNode >::New() );
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneEndImport()
{
  vtkCollection* markupsToModelNodes = this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMarkupsToModelNode" );
  vtkCollectionIterator* markupsToModelNodeIt = vtkCollectionIterator::New();
  markupsToModelNodeIt->SetCollection( markupsToModelNodes );
  for ( markupsToModelNodeIt->InitTraversal(); ! markupsToModelNodeIt->IsDoneWithTraversal(); markupsToModelNodeIt->GoToNextItem() )
  {
    vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( markupsToModelNodeIt->GetCurrentObject() );
    if ( markupsToModelNode != NULL)
    {
      vtkWarningMacro( "OnMRMLSceneEndImport: Module node added. Set the model pointer " );

      if( GetModelNodeName(markupsToModelNode).compare("")!=0 &&
          markupsToModelNode->GetModelNode()==NULL)
      {
        vtkMRMLNode* modelNodeFromScene = this->GetMRMLScene()->GetNodeByID(markupsToModelNode->GetModelNode()->GetID());
        if ( modelNodeFromScene != NULL )
        {
          markupsToModelNode->SetAndObserveModelNodeID( modelNodeFromScene->GetID() );
        }
        else
        {
          vtkWarningMacro("NOT founded the saved Model");
        }
      }
    }
  }
  markupsToModelNodeIt->Delete();
  markupsToModelNodes->Delete();
  this->Modified();
  this->ImportingScene=0;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneStartImport()
{
  this->ImportingScene=1;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast(node);
  if ( markupsToModelNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( markupsToModelNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLMarkupsToModelNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( markupsToModelNode, events.GetPointer() );
  }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkSlicerMarkupsToModelNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::SetMarkupsNode( vtkMRMLMarkupsFiducialNode* newMarkups, vtkMRMLMarkupsToModelNode* moduleNode )
{
  if ( moduleNode == NULL )
  {
    vtkWarningMacro( "SetWatchedModelNode: Module node is invalid" );
    return;
  }

  vtkMRMLMarkupsFiducialNode* previousMarkups = moduleNode->GetMarkupsNode();
  if (previousMarkups==newMarkups)
  {
    // no change
    return;
  }
  // Switch to the new model node
  moduleNode->SetAndObserveMarkupsNodeID( (newMarkups!=NULL) ? newMarkups->GetID() : NULL );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateSelectionNode( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode )
{
  vtkMRMLMarkupsFiducialNode* markupsNode = markupsToModelModuleNode->GetMarkupsNode();
  if ( markupsNode == NULL )
  {
    vtkWarningMacro("No markups yet");
    return;
  }
  std::string selectionNodeID = std::string("");

  if ( !this->GetMRMLScene() )
  {
    vtkErrorMacro("UpdateSelectionNode: no scene defined!");
    return;
  }

  // try the application logic first
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  vtkMRMLSelectionNode *selectionNode ;
  if (mrmlAppLogic)
  {
    selectionNode = mrmlAppLogic->GetSelectionNode();
  }
  else
  {
    // try a default string
    selectionNodeID = std::string("vtkMRMLSelectionNodeSingleton");
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str()));
  }

  if (selectionNode)
  {
    // check if changed
    const char *selectionNodeActivePlaceNodeID = selectionNode->GetActivePlaceNodeID();

    const char *activeID = NULL;
    if (markupsNode)
    {
      activeID = markupsNode->GetID();
    }

    // get the current node from the combo box
    //QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
    //qDebug() << "setActiveMarkupsNode: combo box says: " << qPrintable(activeMarkupsNodeID) << ", input node says " << (activeID ? activeID : "null");

    // don't update the selection node if the active ID is null (can happen
    // when entering the module)
    if (activeID != NULL)
    {
      if (!selectionNodeActivePlaceNodeID || !activeID ||
        strcmp(selectionNodeActivePlaceNodeID, activeID) != 0)
      {
        selectionNode->SetReferenceActivePlaceNodeID(activeID);
      }
    }
    //else
    //{
    //  if (selectionNodeActivePlaceNodeID != NULL)
    //  {
    //    //std::cout << "Setting combo box from selection node " << selectionNodeActivePlaceNodeID << std::endl;
    //    d->activeMarkupMRMLNodeComboBox->setCurrentNodeID(selectionNodeActivePlaceNodeID);
    //  }
    //}
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputCloseSurfaceModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* output)
{
  vtkMRMLMarkupsFiducialNode* markups = markupsToModelModuleNode->GetMarkupsNode();
  if(markups == NULL)
  {
    return;
  }
  int numberOfMarkups = markups->GetNumberOfFiducials();

  if (numberOfMarkups == 0)
  {
    return;
  }

  vtkSmartPointer< vtkPoints > modelPoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkCellArray > modelCellArray = vtkSmartPointer< vtkCellArray >::New();

  modelPoints->SetNumberOfPoints(numberOfMarkups);
  modelCellArray->InsertNextCell(numberOfMarkups);

  double markupPoint[3] = {0.0, 0.0, 0.0};
  double* coords = new double[numberOfMarkups*3];
  double meanPoint[3] = {0.0, 0.0, 0.0};

  for (int i = 0; i < numberOfMarkups; i++)
  {
    markups->GetNthFiducialPosition(i, markupPoint);
    modelPoints->SetPoint(i, markupPoint);
    vtkMath::Add(meanPoint, markupPoint, meanPoint);
    coords[i * 3 + 0] = markupPoint[0];
    coords[i * 3 + 1] = markupPoint[1];
    coords[i * 3 + 2] = markupPoint[2];
    modelCellArray->InsertCellPoint(i);
  }
  if (numberOfMarkups > 0)
  {
    vtkMath::MultiplyScalar(meanPoint, 1.0 / numberOfMarkups);
  }

  double corner[3] = {0.0, 0.0, 0.0};
  double normal[3] = {0.0, 0.0, 0.0};
  double temp[3] = {0.0, 0.0, 0.0};
  vtkSmartPointer<vtkOBBTree> obbTree = vtkSmartPointer<vtkOBBTree>::New();
  obbTree->ComputeOBB(modelPoints, corner, temp, temp, normal, temp);

  double relative[3] = {0.0, 0.0, 0.0};
  vtkMath::Subtract(meanPoint, corner, relative);
  vtkMath::Normalize(normal);
  double proj[3];
  std::copy(normal, normal+3, proj);
  double dotProd = vtkMath::Dot(relative, normal);
  vtkMath::MultiplyScalar(proj, dotProd);

  double origin[3];
  vtkMath::Add(corner, proj, origin);

  double A = normal[0];
  double B = normal[1];
  double C = normal[2];
  double x0 = origin[0];
  double y0 = origin[1];
  double z0 = origin[2];
  double D = (-1)*A*x0 - B*y0 - C*z0;
  
  // assume by default that the surface is planar
  bool planarSurface = true;
  for (int i = 0; i < numberOfMarkups; i++)
  {
    double x1 = coords[3 * i + 0];
    double y1 = coords[3 * i + 1];
    double z1 = coords[3 * i + 2];
    double distance = std::abs(A*x1 + B*y1 + C*z1 + D) / std::sqrt(A*A + B*B + C*C);
    if (distance >= MINIMUM_THICKNESS)
    {
      planarSurface = false;
    }
  }

  delete[] coords;

  vtkSmartPointer< vtkPolyData > pointPolyData = vtkSmartPointer< vtkPolyData >::New();
  pointPolyData->SetLines(modelCellArray);
  pointPolyData->SetPoints(modelPoints);

  vtkSmartPointer< vtkDelaunay3D > delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetAlpha(markupsToModelModuleNode->GetDelaunayAlpha());

  if (planarSurface)
  {
    vtkSmartPointer<vtkCubeSource> cube = vtkSmartPointer<vtkCubeSource>::New();
    vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
    glyph->SetSourceConnection(cube->GetOutputPort());
    glyph->SetInputData(pointPolyData);
    glyph->Update();
    delaunay->SetInputConnection(glyph->GetOutputPort());
  }
  else
  {
    if(markupsToModelModuleNode->GetCleanMarkups())
    {
      vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New();
#if (VTK_MAJOR_VERSION <= 5)
      cleanPointPolyData->SetInput(pointPolyData);
#else
      cleanPointPolyData->SetInputData(pointPolyData);
#endif
      cleanPointPolyData->SetTolerance(CLEAN_POLYDATA_TOLERANCE);
      delaunay->SetInputConnection(cleanPointPolyData->GetOutputPort());
    }
    else
    {
#if (VTK_MAJOR_VERSION <= 5)
      delaunay->SetInput(pointPolyData);
#else
      delaunay->SetInputData(pointPolyData);
#endif
    }
  }

  vtkSmartPointer< vtkDataSetSurfaceFilter > surfaceFilter = vtkSmartPointer< vtkDataSetSurfaceFilter >::New();
  surfaceFilter->SetInputConnection(delaunay->GetOutputPort());
  surfaceFilter->Update();
  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetFeatureAngle(100);

  if ( markupsToModelModuleNode->GetButterflySubdivision() && !planarSurface )
  {
    vtkSmartPointer< vtkButterflySubdivisionFilter > subdivisionFilter = vtkSmartPointer< vtkButterflySubdivisionFilter >::New();
    subdivisionFilter->SetInputConnection(surfaceFilter->GetOutputPort());
    subdivisionFilter->SetNumberOfSubdivisions(3);
    subdivisionFilter->Update();
    if(markupsToModelModuleNode->GetConvexHull())
    {
      vtkSmartPointer<vtkDelaunay3D> convexHull = vtkSmartPointer<vtkDelaunay3D>::New();
      convexHull->SetAlpha(markupsToModelModuleNode->GetDelaunayAlpha());
      convexHull->SetInputConnection(subdivisionFilter->GetOutputPort());
      convexHull->Update();
      vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
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
    normals->SetInputConnection(surfaceFilter->GetOutputPort());
  }
  normals->Update();
  output->DeepCopy(normals->GetOutput());
}

//------------------------------------------------------------------------------
void markupsToPath(vtkMRMLMarkupsFiducialNode* markupsNode, vtkPolyData* markupsPointsPolyData)
{
  vtkSmartPointer< vtkPoints > modelPoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkCellArray > modelCellArray = vtkSmartPointer< vtkCellArray >::New();
  int numberOfMarkups = markupsNode->GetNumberOfFiducials();
  modelPoints->SetNumberOfPoints(numberOfMarkups);
  double markupPoint [3] = {0.0, 0.0, 0.0};

  for (int i=0; i<numberOfMarkups;i++)
  {
    markupsNode->GetNthFiducialPosition(i,markupPoint);
    modelPoints->SetPoint(i, markupPoint);
  }

  modelCellArray->InsertNextCell(numberOfMarkups);
  for (int i=0; i < numberOfMarkups;i++)
  {
    modelCellArray->InsertCellPoint(i);
  }

  markupsPointsPolyData->SetPoints(modelPoints);
  markupsPointsPolyData->SetLines(modelCellArray);
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputLinearModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData, vtkPolyData* outputPolyData)
{
  vtkSmartPointer< vtkAppendPolyData> append = vtkSmartPointer< vtkAppendPolyData>::New();

  vtkPoints* points = markupsPointsPolyData->GetPoints();
  if (points == NULL)
  {
    vtkErrorMacro("Markups contains a vtkPoints object that is null. No model generated.");
    return;
  }

  int numPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numPoints < LINE_MIN_NUMBER_POINTS)
  {
    vtkErrorMacro("Not enough points to create an output linear model. Need at least 2, " << numPoints << " are provided. No output created.");
    return;
  }

  double point0 [3] = {0, 0, 0};
  double point1 [3]= {0, 0, 0};

  for( int i =0; i<numPoints-1; i++)
  {
    points->GetPoint(i, point0);
    points->GetPoint(i+1, point1);
    vtkSmartPointer< vtkPoints > pointsSegment = vtkSmartPointer< vtkPoints >::New();
    pointsSegment->SetNumberOfPoints(2);
    pointsSegment->SetPoint(0, point0);
    pointsSegment->SetPoint(1, point1);

    vtkSmartPointer< vtkCellArray > cellArraySegment = vtkSmartPointer<  vtkCellArray >::New();
    cellArraySegment->InsertNextCell(2);
    cellArraySegment->InsertCellPoint(0);
    cellArraySegment->InsertCellPoint(1);

    vtkSmartPointer< vtkPolyData >polydataSegment = vtkSmartPointer< vtkPolyData >::New();
    polydataSegment->Initialize();
    polydataSegment->SetPoints(pointsSegment);
    polydataSegment->SetLines(cellArraySegment);

    vtkSmartPointer< vtkTubeFilter> tubeSegmentFilter = vtkSmartPointer< vtkTubeFilter>::New();
#if (VTK_MAJOR_VERSION <= 5)
    tubeSegmentFilter->SetInput(polydataSegment);
#else
    tubeSegmentFilter->SetInputData(polydataSegment);
#endif

    tubeSegmentFilter->SetRadius(markupsToModelModuleNode->GetTubeRadius());
    tubeSegmentFilter->SetNumberOfSides(20);
    tubeSegmentFilter->CappingOn();
    tubeSegmentFilter->Update();

    //if vtk.VTK_MAJOR_VERSION <= 5
    //append.AddInput(tubeFilter.GetOutput());
    //else:
    append->AddInputData(tubeSegmentFilter->GetOutput());
  }

  append->Update();

  outputPolyData->DeepCopy(append->GetOutput());
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputHermiteSplineModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData, vtkPolyData* outputPolyData)
{
  vtkPoints* points = markupsPointsPolyData->GetPoints();
  if (points == NULL)
  {
    vtkErrorMacro("Markups contains a vtkPoints object that is null. No model generated.");
    return;
  }

  int numPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numPoints < LINE_MIN_NUMBER_POINTS)
  {
    vtkErrorMacro("Not enough points to create an output spline model. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numPoints << " are provided. No output created.");
    return;
  }
  
  // special case, fit a line. Spline fitting will not work with fewer than 3 points
  if (numPoints == LINE_MIN_NUMBER_POINTS)
  {
    vtkWarningMacro("Only " << LINE_MIN_NUMBER_POINTS << " provided. Fitting line.");
    UpdateOutputLinearModel(markupsToModelModuleNode, markupsPointsPolyData, outputPolyData);
    return;
  }

  int totalNumberOfPoints = markupsToModelModuleNode->GetTubeSamplePointsBetweenControlPoints()*markupsPointsPolyData->GetNumberOfPoints();
  vtkSmartPointer< vtkSplineFilter > splineFilter = vtkSmartPointer< vtkSplineFilter >::New();
#if (VTK_MAJOR_VERSION <= 5)
  splineFilter->SetInput(markupsPointsPolyData);
#else
  splineFilter->SetInputData(markupsPointsPolyData);
#endif

  splineFilter->SetNumberOfSubdivisions(totalNumberOfPoints);

  vtkSmartPointer<vtkKochanekSpline> spline;
  if (markupsToModelModuleNode->GetInterpolationType()== vtkMRMLMarkupsToModelNode::KochanekSpline)
  {
    spline = vtkSmartPointer<vtkKochanekSpline>::New();
    spline->SetDefaultBias(markupsToModelModuleNode->GetKochanekBias());
    spline->SetDefaultContinuity(markupsToModelModuleNode->GetKochanekContinuity());
    spline->SetDefaultTension(markupsToModelModuleNode->GetKochanekTension());
    splineFilter->SetSpline(spline);
  }

  splineFilter->Update();

  vtkSmartPointer< vtkTubeFilter> cardinalSplineTubeFilter = vtkSmartPointer< vtkTubeFilter>::New();
  cardinalSplineTubeFilter->SetInputConnection(splineFilter->GetOutputPort());
  cardinalSplineTubeFilter->SetRadius(markupsToModelModuleNode->GetTubeRadius());
  cardinalSplineTubeFilter->SetNumberOfSides(markupsToModelModuleNode->GetTubeNumberOfSides());
  cardinalSplineTubeFilter->CappingOn();
  cardinalSplineTubeFilter->Update();

  outputPolyData->DeepCopy(cardinalSplineTubeFilter->GetOutput());
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ComputePointParametersRawIndices(vtkPolyData * markupsPointsPolyData, vtkDoubleArray* markupsPointsParameters)
{
  vtkPoints* points = markupsPointsPolyData->GetPoints();
  if (points == NULL)
  {
    vtkErrorMacro("Markups contains a vtkPoints object that is null.");
    return;
  }

  int numPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numPoints < LINE_MIN_NUMBER_POINTS)
  {
    vtkErrorMacro("Not enough points to compute polynomial parameters. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numPoints << " are provided.");
    return;
  }

  if (markupsPointsParameters->GetNumberOfTuples())
  {
    // this should never happen, but in case it does, output a warning
    vtkWarningMacro("markupsPointsParameters already has contents. Clearing.");
    while (markupsPointsParameters->GetNumberOfTuples()) // clear contents just in case
      markupsPointsParameters->RemoveLastTuple();
  }

  for (int v = 0; v < numPoints; v++)
  {
    markupsPointsParameters->InsertNextTuple1( v / double(numPoints-1) );
    // division to clamp all values to range 0.0 - 1.0
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ComputePointParametersMinimumSpanningTree(vtkPolyData * markupsPointsPolyData, vtkDoubleArray* markupsPointsParameters)
{
  vtkPoints* points = markupsPointsPolyData->GetPoints();
  if (points == NULL)
  {
    vtkErrorMacro("Markups contains a vtkPoints object that is null.");
    return;
  }

  int numPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numPoints < LINE_MIN_NUMBER_POINTS)
  {
    vtkErrorMacro("Not enough points to compute polynomial parameters. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numPoints << " are provided.");
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
  std::vector<double> distances(numPoints*numPoints);
  distances.assign(numPoints*numPoints, 0.0);
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
      points->GetPoint(u, pointU);
      points->GetPoint(v, pointV);
      double distX = (pointU[0] - pointV[0]); double distXsq = distX * distX;
      double distY = (pointU[1] - pointV[1]); double distYsq = distY * distY;
      double distZ = (pointU[2] - pointV[2]); double distZsq = distZ * distZ;
      double dist3D = sqrt(distXsq+distYsq+distZsq);
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
  std::vector<double*> graph(numPoints);
  for (int v = 0; v < numPoints; v++)
  {
    graph[v] = &(distances[v * numPoints]);
  }

  // implementation of Prim's algorithm heavily based on:
  // http://www.geeksforgeeks.org/greedy-algorithms-set-5-prims-minimum-spanning-tree-mst-2/
  std::vector<int> parent(numPoints); // Array to store constructed MST
  std::vector<double> key(numPoints);   // Key values used to pick minimum weight edge in cut
  std::vector<bool> mstSet(numPoints);  // To represent set of vertices not yet included in MST
 
  // Initialize all keys as INFINITE
  for (int i = 0; i < numPoints; i++)
  {
    key[i] = VTK_DOUBLE_MAX;
    mstSet[i] = false;
  }
 
  // Always include first 1st vertex in MST.
  key[treeStartIndex] = 0.0;     // Make key 0 so that this vertex is picked as first vertex
  parent[treeStartIndex] = -1; // First node is always root of MST 
 
  // The MST will have numPoints vertices
  for (int count = 0; count < numPoints-1; count++)
  {
    // Pick the minimum key vertex from the set of vertices
    // not yet included in MST
    int nextPointIndex = -1;
    double minDistance = VTK_DOUBLE_MAX, min_index;
    for (int v = 0; v < numPoints; v++)
    {
      if (mstSet[v] == false &&
          key[v] < minDistance)
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
          graph[nextPointIndex][v] <  key[v])
      {
        parent[v] = nextPointIndex;
        key[v] = graph[nextPointIndex][v];
      }
    }
  }
 
  // determine the "trunk" path of the tree, from first index to last index
  std::vector<int> pathIndices;
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
    sumOfDistances += graph[i][i+1];
  }

  // check this to prevent a division by zero (in case all points are duplicates)
  if (sumOfDistances == 0)
  {
    vtkErrorMacro("Minimum spanning tree path has distance zero. No parameters will be assigned. Check inputs!");
    return;
  }

  // find the parameters along the trunk path of the tree
  std::vector<double> pathParameters;
  double currentDistance = 0.0;
  for (int i = 0; i < pathIndices.size() - 1; i++)
  {
    pathParameters.push_back(currentDistance/sumOfDistances);
    currentDistance += graph[i][i+1];
  }
  pathParameters.push_back(currentDistance/sumOfDistances); // this should be 1.0

  // finally assign polynomial parameters to each point, and store in the output array
  if (markupsPointsParameters->GetNumberOfTuples() > 0)
  {
    // this should never happen, but in case it does, output a warning
    vtkWarningMacro("markupsPointsParameters already has contents. Clearing.");
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
void vtkSlicerMarkupsToModelLogic::UpdateOutputPolynomialFitModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData, vtkDoubleArray* markupsPointsParameters, vtkPolyData* outputPolyData)
{
  vtkPoints* points = markupsPointsPolyData->GetPoints();
  if (points == NULL)
  {
    vtkErrorMacro("Markups contains a vtkPoints object that is null. No model generated.");
    return;
  }

  int numPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numPoints < LINE_MIN_NUMBER_POINTS)
  {
    vtkErrorMacro("Not enough points to compute a polynomial fit. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numPoints << " are provided. No output created.");
    return;
  }

  // special case, fit a line. The polynomial solver does not work with only 2 points.
  if (numPoints == LINE_MIN_NUMBER_POINTS)
  {
    vtkWarningMacro("Only " << LINE_MIN_NUMBER_POINTS << " provided. Fitting line.");
    UpdateOutputLinearModel(markupsToModelModuleNode, markupsPointsPolyData, outputPolyData);
    return;
  }

  // check size of point parameters array for consistency
  if ( markupsPointsParameters->GetNumberOfTuples() != numPoints )
  {
    vtkErrorMacro("Incorrect number of point parameters provided. Should have " << numPoints << " parameters (one for each point), but " << markupsPointsParameters->GetNumberOfTuples() << " are provided. No output created.");
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
    vtkWarningMacro("Desired polynomial order " << polynomialOrder << " is not supported. "
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
    vtkWarningMacro("Not enough points to compute a polynomial fit. " << "For an order " << polynomialOrder << " polynomial, at least " << numPolynomialCoefficients << " points with unique parameters are needed. "
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
      double value = std::pow( markupsPointsParameters->GetValue(p), c );
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
    double* currentPoint = points->GetPoint(p);
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
  int numControlPointsOnTube = numPoints * markupsToModelModuleNode->GetTubeSamplePointsBetweenControlPoints();
  smoothedLines->InsertNextCell(numControlPointsOnTube); // one long continuous line
  for (int p = 0; p < numControlPointsOnTube; p++) // p = point index
  {
    double pointMm[3];
    for (int d = 0; d < numDimensions; d++)
    {
      pointMm[d] = 0.0;
      for (int c = 0; c < numPolynomialCoefficients; c++)
      {
        double coefficient = coefficientValues[c * numDimensions + d];
        pointMm[d] += coefficient * std::pow( (double(p)/(numControlPointsOnTube-1)), c );
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
#if (VTK_MAJOR_VERSION <= 5)
  tubeFilter->SetInput(smoothedSegments);
#else
  tubeFilter->SetInputData(smoothedSegments);
#endif
  tubeFilter->SetRadius(markupsToModelModuleNode->GetTubeRadius());
  tubeFilter->SetNumberOfSides(markupsToModelModuleNode->GetTubeNumberOfSides());
  tubeFilter->CappingOn();
  tubeFilter->Update();

  outputPolyData->DeepCopy(tubeFilter->GetOutput());
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* outputPolyData)
{

  vtkMRMLMarkupsFiducialNode* markupsNode = markupsToModelModuleNode->GetMarkupsNode( );
  if( markupsNode == NULL )
  {
    return;
  }

  int numberOfMarkups = markupsNode->GetNumberOfFiducials();
  if (numberOfMarkups < LINE_MIN_NUMBER_POINTS) // check this here, but also perform redundant checks elsewhere
  {
    return;
  }

  vtkSmartPointer< vtkPolyData > markupsPointsPolyData = vtkSmartPointer< vtkPolyData >::New( );
  markupsToPath( markupsNode, markupsPointsPolyData );
  vtkSmartPointer< vtkPolyData > finalMarkupsPointsPolyData;

  vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New( );
  if ( markupsToModelModuleNode->GetCleanMarkups( ) )
  {
#if (VTK_MAJOR_VERSION <= 5)
    cleanPointPolyData->SetInput( markupsPointsPolyData );
#else
    cleanPointPolyData->SetInputData( markupsPointsPolyData );
#endif
    cleanPointPolyData->SetTolerance( CLEAN_POLYDATA_TOLERANCE );
    cleanPointPolyData->Update( );
    finalMarkupsPointsPolyData= cleanPointPolyData->GetOutput( );
  }
  else
  {
    finalMarkupsPointsPolyData = markupsPointsPolyData;
  }

  switch( markupsToModelModuleNode->GetInterpolationType() )
  {
    case vtkMRMLMarkupsToModelNode::Linear:
    {
      UpdateOutputLinearModel( markupsToModelModuleNode, finalMarkupsPointsPolyData, outputPolyData );
      break;
    }
    case vtkMRMLMarkupsToModelNode::CardinalSpline:
    {
      UpdateOutputHermiteSplineModel( markupsToModelModuleNode, finalMarkupsPointsPolyData, outputPolyData );
      break;
    }
    case vtkMRMLMarkupsToModelNode::KochanekSpline:
    {
      UpdateOutputHermiteSplineModel( markupsToModelModuleNode, finalMarkupsPointsPolyData, outputPolyData );
      break;
    }
    case vtkMRMLMarkupsToModelNode::Polynomial:
    {
      vtkSmartPointer<vtkDoubleArray> pointParameters = vtkSmartPointer<vtkDoubleArray>::New();
      switch ( markupsToModelModuleNode->GetPointParameterType() )
      {
        case vtkMRMLMarkupsToModelNode::RawIndices:
        {
          ComputePointParametersRawIndices(finalMarkupsPointsPolyData,pointParameters);
          break;
        }
        case vtkMRMLMarkupsToModelNode::MinimumSpanningTree:
        {
          ComputePointParametersMinimumSpanningTree(finalMarkupsPointsPolyData,pointParameters);
          break;
        }
        default:
        {
          vtkWarningMacro("Invalid PointParameterType: " << markupsToModelModuleNode->GetPointParameterType() << ". Using raw indices.");
          ComputePointParametersRawIndices(finalMarkupsPointsPolyData,pointParameters);
          break;
        }
      }
      UpdateOutputPolynomialFitModel(markupsToModelModuleNode,finalMarkupsPointsPolyData,pointParameters,outputPolyData);
      break;
    }
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{
  if ( this->ImportingScene == 1 )
  {
    return;
  }

  if ( markupsToModelModuleNode->GetModelNode() == NULL || markupsToModelModuleNode->GetMarkupsNode() == NULL )
  {
    return;
  }

  // store the output poly data in this pointer
  vtkSmartPointer<vtkPolyData> outputPolyData = vtkSmartPointer<vtkPolyData>::New();

  switch ( markupsToModelModuleNode->GetModelType() )
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: UpdateOutputCloseSurfaceModel( markupsToModelModuleNode, outputPolyData ); break;
  case vtkMRMLMarkupsToModelNode::Curve: UpdateOutputCurveModel( markupsToModelModuleNode, outputPolyData ); break;
  }

  // assign the poly data to the model node
  vtkSmartPointer< vtkMRMLModelNode > modelNode = markupsToModelModuleNode->GetModelNode();
  if ( markupsToModelModuleNode->GetModelNode() == NULL )
  {
    modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
    this->GetMRMLScene()->AddNode( modelNode );
    modelNode->SetName( GetModelNodeName(markupsToModelModuleNode).c_str() );
  }
  modelNode->SetAndObservePolyData( outputPolyData );

  vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if ( displayNode == NULL )
  {
    modelNode->CreateDefaultDisplayNodes();
    displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
    this->GetMRMLScene()->AddNode( displayNode );
    displayNode->SetName( GetModelDisplayNodeName(markupsToModelModuleNode).c_str());
    modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  }

  markupsToModelModuleNode->SetAndObserveModelNodeID(modelNode->GetID());
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if (callerNode == NULL)
  {
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelModuleNode = vtkMRMLMarkupsToModelNode::SafeDownCast( callerNode );
  if (markupsToModelModuleNode == NULL)
  {
    return;
  }

  if (event==vtkMRMLMarkupsToModelNode::InputDataModifiedEvent)
  {
    if(markupsToModelModuleNode->GetAutoUpdateOutput())
    {
      this->UpdateOutputModel(markupsToModelModuleNode);
    }
  }
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetMarkupsNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLMarkupsNode* markupsNode = mrmlNode->GetMarkupsNode();
  if ( markupsNode == NULL )
  {
    return std::string( mrmlNode->GetID() ).append( "Markups" );
  }

  return std::string( markupsNode->GetName() );
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetModelNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLModelNode* modelNode = mrmlNode->GetModelNode();
  if ( modelNode == NULL )
  {
    return std::string( mrmlNode->GetID() ).append( "Model" );
  }

  return std::string( modelNode->GetName() );
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetMarkupsDisplayNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLMarkupsNode* markupsNode = mrmlNode->GetMarkupsNode();
  if ( markupsNode == NULL )
  {
    return std::string( mrmlNode->GetID() ).append( "MarkupsDisplay" );
  }

  vtkMRMLModelDisplayNode* markupsDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  if ( markupsDisplayNode == NULL )
  {
    return std::string( mrmlNode->GetID() ).append( "MarkupsDisplay" );
  }

  return std::string( markupsDisplayNode->GetName() );
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetModelDisplayNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLModelNode* modelNode = mrmlNode->GetModelNode();
  if ( modelNode == NULL )
  {
    return std::string( mrmlNode->GetID() ).append( "ModelDisplay" );
  }

  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( modelDisplayNode == NULL )
  {
    return std::string( mrmlNode->GetID() ).append( "ModelDisplay" );
  }

  return std::string( modelDisplayNode->GetName() );
}
