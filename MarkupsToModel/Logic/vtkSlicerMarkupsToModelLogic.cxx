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
    vtkMath::MultiplyScalar(meanPoint, 1.0 / numberOfMarkups);

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

  vtkPoints * points;

  points = markupsPointsPolyData->GetPoints();

  double point0 [3] = {0, 0, 0};
  double point1 [3]= {0, 0, 0};

  for( int i =0; i<points->GetNumberOfPoints()-1; i++)
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
  int totalNumberOfPoints = markupsToModelModuleNode->GetTubeSamplingFrequency()*markupsPointsPolyData->GetNumberOfPoints();
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
void vtkSlicerMarkupsToModelLogic::UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* outputPolyData)
{

  vtkMRMLMarkupsFiducialNode* markupsNode = markupsToModelModuleNode->GetMarkupsNode( );
  if( markupsNode == NULL )
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
  case vtkMRMLMarkupsToModelNode::Linear: UpdateOutputLinearModel( markupsToModelModuleNode, finalMarkupsPointsPolyData, outputPolyData ); break;
  case vtkMRMLMarkupsToModelNode::CardinalSpline: UpdateOutputHermiteSplineModel( markupsToModelModuleNode, finalMarkupsPointsPolyData, outputPolyData ); break;
  case vtkMRMLMarkupsToModelNode::KochanekSpline: UpdateOutputHermiteSplineModel( markupsToModelModuleNode, finalMarkupsPointsPolyData, outputPolyData ); break;
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
  //vtkWarningMacro("PUTAS");
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
    return std::string( mrmlNode->GetID() ).append( "Markups" );

  return std::string( markupsNode->GetName() );
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetModelNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLModelNode* modelNode = mrmlNode->GetModelNode();
  if ( modelNode == NULL )
    return std::string( mrmlNode->GetID() ).append( "Model" );

  return std::string( modelNode->GetName() );
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetMarkupsDisplayNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLMarkupsNode* markupsNode = mrmlNode->GetMarkupsNode();
  if ( markupsNode == NULL )
    return std::string( mrmlNode->GetID() ).append( "MarkupsDisplay" );

  vtkMRMLModelDisplayNode* markupsDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( markupsNode->GetDisplayNode() );
  if ( markupsDisplayNode == NULL )
    return std::string( mrmlNode->GetID() ).append( "MarkupsDisplay" );

  return std::string( markupsDisplayNode->GetName() );
}

//------------------------------------------------------------------------------
std::string vtkSlicerMarkupsToModelLogic::GetModelDisplayNodeName(vtkMRMLMarkupsToModelNode* mrmlNode)
{
  vtkMRMLModelNode* modelNode = mrmlNode->GetModelNode();
  if ( modelNode == NULL )
    return std::string( mrmlNode->GetID() ).append( "ModelDisplay" );

  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast( modelNode->GetDisplayNode() );
  if ( modelDisplayNode == NULL )
    return std::string( mrmlNode->GetID() ).append( "ModelDisplay" );

  return std::string( modelDisplayNode->GetName() );
}
