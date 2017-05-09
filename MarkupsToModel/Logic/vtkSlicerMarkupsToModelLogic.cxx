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
#include <vtkCardinalSpline.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkDoubleArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDelaunay3D.h>
#include <vtkGlyph3D.h>
#include <vtkIntArray.h>
#include <vtkKochanekSpline.h>
#include <vtkLineSource.h>
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

// local constants
static const int LINE_MIN_NUMBER_POINTS = 2;
static const int MINIMUM_MARKUPS_NUMBER = 3;
static const double MINIMUM_THICKNESS = 3.0;
static const double CLEAN_POLYDATA_TOLERANCE = 0.01;
static const double COMPARE_TO_ZERO_TOLERANCE = 0.0001;

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
void vtkSlicerMarkupsToModelLogic::UpdateOutputCloseSurfaceModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* output )
{
  vtkMRMLMarkupsFiducialNode* markups = markupsToModelModuleNode->GetMarkupsNode();
  if ( markups == NULL )
  {
    return;
  }
  int numberOfMarkups = markups->GetNumberOfFiducials();

  if ( numberOfMarkups == 0 )
  {
    return;
  }

  vtkSmartPointer< vtkPoints > modelPoints = vtkSmartPointer< vtkPoints >::New();
  modelPoints->SetNumberOfPoints( numberOfMarkups );

  vtkSmartPointer< vtkCellArray > modelCellArray = vtkSmartPointer< vtkCellArray >::New();
  modelCellArray->InsertNextCell( numberOfMarkups );
  
  double markupPoint[ 3 ] = { 0.0, 0.0, 0.0 };
  for ( int i = 0; i < numberOfMarkups; i++ )
  {
    markups->GetNthFiducialPosition( i, markupPoint );
    modelCellArray->InsertCellPoint( i );
    modelPoints->SetPoint( i, markupPoint );
  }

  vtkSmartPointer< vtkPolyData > pointPolyData = vtkSmartPointer< vtkPolyData >::New();
  pointPolyData->SetLines( modelCellArray );
  pointPolyData->SetPoints( modelPoints );

  vtkSmartPointer< vtkDelaunay3D > delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetAlpha( markupsToModelModuleNode->GetDelaunayAlpha() );
  delaunay->AlphaTrisOff();
  delaunay->AlphaLinesOff();
  delaunay->AlphaVertsOff();

  double bestFitPlanePoint[ 3 ] = { 0.0, 0.0, 0.0 };
  ComputeMeanPoint( modelPoints, bestFitPlanePoint );
  double bestFitPlaneNormal[ 3 ] = { 0.0, 0.0, 0.0 };
  ComputeBestFitPlaneNormal( modelPoints, bestFitPlaneNormal );

  bool planarSurface = PointsCoplanar( modelPoints, bestFitPlaneNormal, bestFitPlanePoint );
  if (planarSurface)
  {
    // extrude additional points on either side of the plane
    vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
    double extrusionMagnitude = 1.0;
    double point1[ 3 ] = { bestFitPlaneNormal[0], bestFitPlaneNormal[1], bestFitPlaneNormal[2] };
    vtkMath::MultiplyScalar( point1, extrusionMagnitude );
    lineSource->SetPoint1( point1 );
    double point2[ 3 ] = { bestFitPlaneNormal[0], bestFitPlaneNormal[1], bestFitPlaneNormal[2] };
    vtkMath::MultiplyScalar( point2, -extrusionMagnitude );
    lineSource->SetPoint2( point2 );

    vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
    glyph->SetSourceConnection(lineSource->GetOutputPort());
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
void vtkSlicerMarkupsToModelLogic::ComputeMeanPoint( vtkPoints* points, double meanPoint[ 3 ] )
{
  // compute the mean point, which is on the plane of best fit
  meanPoint[0] = 0.0;
  meanPoint[1] = 0.0;
  meanPoint[2] = 0.0;
  int numberOfPoints = points->GetNumberOfPoints();
  for ( int i = 0; i < numberOfPoints; i++ )
  {
    double currentPoint[ 3 ] = { 0.0, 0.0, 0.0 };
    points->GetPoint( i, currentPoint );
    vtkMath::Add( meanPoint, currentPoint, meanPoint );
  }

  if ( numberOfPoints > 0 )
  {
    vtkMath::MultiplyScalar( meanPoint, 1.0 / numberOfPoints );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ComputeBestFitPlaneNormal( vtkPoints* points, double outputPlaneNormal[ 3 ] )
{
  // Compute the plane using the smallest bounding box that can have arbitrary axes
  // The shortest axis will correspond to the plane normal
  double cornerOBBOrigin[3] = { 0.0, 0.0, 0.0 }; // unused
  double longestOBBAxis[3]  = { 0.0, 0.0, 0.0 }; // unused
  double mediumOBBAxis[3]   = { 0.0, 0.0, 0.0 }; // unused
  double shortestOBBAxis[3] = { 0.0, 0.0, 0.0 };
  double relativeAxisSizes[3] = { 0.0, 0.0, 0.0 }; // unused
  vtkSmartPointer<vtkOBBTree> obbTree = vtkSmartPointer<vtkOBBTree>::New();
  obbTree->ComputeOBB( points, cornerOBBOrigin, longestOBBAxis, mediumOBBAxis, shortestOBBAxis, relativeAxisSizes );
  
  // compute the plane normal
  outputPlaneNormal[ 0 ] = shortestOBBAxis[ 0 ];
  outputPlaneNormal[ 1 ] = shortestOBBAxis[ 1 ];
  outputPlaneNormal[ 2 ] = shortestOBBAxis[ 2 ];
  vtkMath::Normalize( outputPlaneNormal );
}

//------------------------------------------------------------------------------
bool vtkSlicerMarkupsToModelLogic::PointsCoplanar( vtkPoints* points, const double planeNormal[ 3 ], const double planePoint[ 3 ] )
{
  double normalLength = vtkMath::Norm( planeNormal );
  bool normalLengthEqualToOne = abs( normalLength - 1.0 ) < COMPARE_TO_ZERO_TOLERANCE;
  if ( !normalLengthEqualToOne )
  {
    vtkWarningMacro( "Normal " << planeNormal[ 0 ] << ", "
                               << planeNormal[ 1 ] << ", "
                               << planeNormal[ 2 ] << " provided to pointsCoplanar does not have unit length."
                               << "Unable to determine whether points are coplanar." );
    return false;
  }

  // assume by default that the surface is planar
  bool planarSurface = true;
  int numberOfPoints = points->GetNumberOfPoints();
  for ( int i = 0; i < numberOfPoints; i++ )
  {
    double currentPoint[ 3 ];
    points->GetPoint( i, currentPoint );
    // translate the current point to the origin
    double currentPointOrigin[ 3 ];
    vtkMath::Subtract( currentPoint, planePoint, currentPointOrigin );
    // compute the distance to the plane going through the origin
    double distance = abs ( vtkMath::Dot( planeNormal, currentPointOrigin ) );
    if ( distance >= MINIMUM_THICKNESS )
    {
      planarSurface = false;
    }
  }

  return planarSurface;
}

//------------------------------------------------------------------------------
void MarkupsToPoints( vtkMRMLMarkupsToModelNode* pNode, vtkPoints* outputPoints )
{
  vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
  int  numberOfMarkups = markupsNode->GetNumberOfFiducials();
  outputPoints->SetNumberOfPoints( numberOfMarkups );
  double markupPoint [ 3 ] = { 0.0, 0.0, 0.0 };
  for ( int i = 0 ; i < numberOfMarkups ; i++ )
  {
    markupsNode->GetNthFiducialPosition( i, markupPoint );
    outputPoints->SetPoint( i, markupPoint );
  }
  
  if ( pNode->GetCleanMarkups() )
  {
    vtkSmartPointer< vtkPolyData > polyData = vtkSmartPointer< vtkPolyData >::New();
    polyData->Initialize();
    polyData->SetPoints( outputPoints );

    vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New();
    #if ( VTK_MAJOR_VERSION <= 5 )
      cleanPointPolyData->SetInput( polyData );
    #else
      cleanPointPolyData->SetInputData( polyData );
    #endif
    cleanPointPolyData->SetTolerance( CLEAN_POLYDATA_TOLERANCE );
    cleanPointPolyData->Update();
  }
}

//------------------------------------------------------------------------------
void AllocateCurvePoints(vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPoints* outputPoints)
{
  // Number of points is different depending on whether the curve is a loop
  int numberControlPoints = controlPoints->GetNumberOfPoints();
  int numberOutputPointsPerControlPoint = pNode->GetTubeSegmentsBetweenControlPoints();
  if ( pNode->GetTubeLoop() )
  {
    outputPoints->SetNumberOfPoints( numberControlPoints * numberOutputPointsPerControlPoint + 2 );
    // two extra points are required to "close off" the loop, and ensure that the tube appears fully continuous
  }
  else
  {
    outputPoints->SetNumberOfPoints( ( numberControlPoints - 1 ) * numberOutputPointsPerControlPoint + 1 );
  }
}

//------------------------------------------------------------------------------
void CloseLoop( vtkPoints* outputPoints )
{
  // If looped, move the first point and add an *extra* point. This is 
  // needed in order for the curve to be continuous, otherwise the tube ends won't 
  // align properly
  double point0[ 3 ];
  outputPoints->GetPoint( 0, point0 );
  double point1[ 3 ];
  outputPoints->GetPoint( 1, point1 );
  double finalPoint[ 3 ];
  finalPoint[ 0 ] = point0[ 0 ] * 0.5 + point1[ 0 ] * 0.5;
  finalPoint[ 1 ] = point0[ 1 ] * 0.5 + point1[ 1 ] * 0.5;
  finalPoint[ 2 ] = point0[ 2 ] * 0.5 + point1[ 2 ] * 0.5;
  outputPoints->SetPoint( 0, finalPoint );
  int finalIndex = outputPoints->GetNumberOfPoints() - 1;
  outputPoints->SetPoint( finalIndex, finalPoint );
}

//------------------------------------------------------------------------------
void SetCardinalSplineParameters( vtkMRMLMarkupsToModelNode* pNode, vtkCardinalSpline* splineX, vtkCardinalSpline* splineY, vtkCardinalSpline* splineZ )
{
  if ( pNode->GetTubeLoop() )
  {
    splineX->ClosedOn();
    splineY->ClosedOn();
    splineZ->ClosedOn();
  }
  vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
  int numberMarkups = markupsNode->GetNumberOfFiducials();
  for ( int i = 0; i < numberMarkups; i++ )
  {
    double point[ 3 ] = { 0.0, 0.0, 0.0 };
    markupsNode->GetNthFiducialPosition( i, point );
    splineX->AddPoint( i, point[ 0 ] );
    splineY->AddPoint( i, point[ 1 ] );
    splineZ->AddPoint( i, point[ 2 ] );
  }
}

//------------------------------------------------------------------------------
void SetKochanekSplineParameters( vtkMRMLMarkupsToModelNode* pNode, vtkKochanekSpline* splineX, vtkKochanekSpline* splineY, vtkKochanekSpline* splineZ )
{
  if ( pNode->GetTubeLoop() )
  {
    splineX->ClosedOn();
    splineY->ClosedOn();
    splineZ->ClosedOn();
  }
  splineX->SetDefaultBias( pNode->GetKochanekBias() );
  splineY->SetDefaultBias( pNode->GetKochanekBias() );
  splineZ->SetDefaultBias( pNode->GetKochanekBias() );
  splineX->SetDefaultContinuity( pNode->GetKochanekContinuity() );
  splineY->SetDefaultContinuity( pNode->GetKochanekContinuity() );
  splineZ->SetDefaultContinuity( pNode->GetKochanekContinuity() );
  splineX->SetDefaultTension( pNode->GetKochanekTension() );
  splineY->SetDefaultTension( pNode->GetKochanekTension() );
  splineZ->SetDefaultTension( pNode->GetKochanekTension() );
  vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode();
  int numberMarkups = markupsNode->GetNumberOfFiducials();
  for ( int i = 0; i < numberMarkups; i++ )
  {
    double point[ 3 ] = { 0.0, 0.0, 0.0 };
    markupsNode->GetNthFiducialPosition( i, point );
    splineX->AddPoint( i, point[ 0 ] );
    splineY->AddPoint( i, point[ 1 ] );
    splineZ->AddPoint( i, point[ 2 ] );
  }
  if ( pNode->GetKochanekEndsCopyNearestDerivatives() )
  {
    // manually set the derivative to the nearest value
    // (difference between the two nearest points). The
    // constraint mode is set to 1, this tells the spline
    // class to use our manual definition.
    // left derivative
    double point0[ 3 ];
    markupsNode->GetNthFiducialPosition( 0, point0 );
    double point1[ 3 ];
    markupsNode->GetNthFiducialPosition( 1, point1 );
    splineX->SetLeftConstraint( 1 );
    splineX->SetLeftValue( point1[ 0 ] - point0[ 0 ] );
    splineY->SetLeftConstraint( 1 );
    splineY->SetLeftValue( point1[ 1 ] - point0[ 1 ] );
    splineZ->SetLeftConstraint( 1 );
    splineZ->SetLeftValue( point1[ 2 ] - point0[ 2 ] );
    // right derivative
    double pointNMinus2[ 3 ];
    markupsNode->GetNthFiducialPosition( numberMarkups - 2, pointNMinus2 );
    double pointNMinus1[ 3 ];
    markupsNode->GetNthFiducialPosition( numberMarkups - 1, pointNMinus1 );
    splineX->SetRightConstraint( 1 );
    splineX->SetRightValue( pointNMinus1[ 0 ] - pointNMinus2[ 0 ] );
    splineY->SetRightConstraint( 1 );
    splineY->SetRightValue( pointNMinus1[ 1 ] - pointNMinus2[ 1 ] );
    splineZ->SetRightConstraint( 1 );
    splineZ->SetRightValue( pointNMinus1[ 2 ] - pointNMinus2[ 2 ] );
  }
  else
  {
    // This ("0") is the most simple mode for end derivative computation, 
    // described by documentation as using the "first/last two points".
    // Use this as the default because others would require setting the 
    // derivatives manually
    splineX->SetLeftConstraint( 0 );
    splineY->SetLeftConstraint( 0 );
    splineZ->SetLeftConstraint( 0 );
    splineX->SetRightConstraint( 0 );
    splineY->SetRightConstraint( 0 );
    splineZ->SetRightConstraint( 0 );
  }
}

//------------------------------------------------------------------------------
void GetTubePolyDataFromPoints( vtkMRMLMarkupsToModelNode* pNode, vtkPoints* pointsToConnect, vtkPolyData* outputTube )
{
  int numPoints = pointsToConnect->GetNumberOfPoints();

  vtkSmartPointer< vtkCellArray > lineCellArray = vtkSmartPointer< vtkCellArray >::New();
  lineCellArray->InsertNextCell(numPoints);
  for ( int i = 0; i < numPoints; i++ )
  {
    lineCellArray->InsertCellPoint( i );
  }

  vtkSmartPointer< vtkPolyData > linePolyData = vtkSmartPointer< vtkPolyData >::New();
  linePolyData->Initialize();
  linePolyData->SetPoints( pointsToConnect );
  linePolyData->SetLines( lineCellArray );

  vtkSmartPointer< vtkTubeFilter> tubeSegmentFilter = vtkSmartPointer< vtkTubeFilter>::New();
#if (VTK_MAJOR_VERSION <= 5)
  tubeSegmentFilter->SetInput( linePolyData );
#else
  tubeSegmentFilter->SetInputData( linePolyData );
#endif

  tubeSegmentFilter->SetRadius( pNode->GetTubeRadius() );
  tubeSegmentFilter->SetNumberOfSides( pNode->GetTubeNumberOfSides() );
  tubeSegmentFilter->CappingOn();
  tubeSegmentFilter->Update();

  outputTube->DeepCopy( tubeSegmentFilter->GetOutput() );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputLinearModel( vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPolyData* outputTubePolyData )
{
  if (controlPoints == NULL)
  {
    vtkErrorMacro("Control points data structure is null. No model generated.");
    return;
  }
  
  int numberControlPoints = controlPoints->GetNumberOfPoints();
  // redundant error checking, to be safe
  if ( numberControlPoints < LINE_MIN_NUMBER_POINTS )
  {
    vtkErrorMacro( "Not enough points to create an output spline model. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numberControlPoints << " are provided. No output created." );
    return;
  }
  
  vtkSmartPointer< vtkPoints > curvePoints = vtkSmartPointer< vtkPoints >::New();
  AllocateCurvePoints( pNode, controlPoints, curvePoints );

  // Iterate over the segments to interpolate, add all the "in-between" points
  int numberSegmentsToInterpolate;
  if ( pNode->GetTubeLoop() )
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
  while ( controlPointIndex < numberSegmentsToInterpolate )
  {
    // find the two points to interpolate between
    double controlPointCurrent[ 3 ];
    controlPoints->GetPoint( controlPointIndex, controlPointCurrent );
    if ( controlPointIndexNext >= numberControlPoints )
    {
      controlPointIndexNext = 0;
    }
    double controlPointNext[ 3 ];
    controlPoints->GetPoint( controlPointIndexNext, controlPointNext );
    // iterate to compute interpolating points
    for ( int i = 0; i < numberCurvePointsPerControlPoint; i++)
    {
      double interpolationParam = i / (double) numberCurvePointsPerControlPoint;
      double curvePoint[ 3 ];
      curvePoint[ 0 ] = ( 1.0 - interpolationParam ) * controlPointCurrent[ 0 ] + interpolationParam * controlPointNext[ 0 ];
      curvePoint[ 1 ] = ( 1.0 - interpolationParam ) * controlPointCurrent[ 1 ] + interpolationParam * controlPointNext[ 1 ];
      curvePoint[ 2 ] = ( 1.0 - interpolationParam ) * controlPointCurrent[ 2 ] + interpolationParam * controlPointNext[ 2 ];
      int curveIndex = controlPointIndex * numberCurvePointsPerControlPoint + i;
      curvePoints->SetPoint( curveIndex, curvePoint );
    }
    controlPointIndex++;
    controlPointIndexNext++;
  }
  // bring it the rest of the way to the final control point
  controlPointIndex = controlPointIndex % numberControlPoints; // if the index exceeds the max, bring back to 0
  double finalPoint[ 3 ] = { 0.0, 0.0, 0.0 };
  controlPoints->GetPoint( controlPointIndex, finalPoint );
  int finalIndex = numberCurvePointsPerControlPoint * numberSegmentsToInterpolate;
  curvePoints->SetPoint( finalIndex, finalPoint );

  // the last part of the curve depends on whether it is a loop or not
  if ( pNode->GetTubeLoop() )
  {
    CloseLoop( curvePoints );
  }

  GetTubePolyDataFromPoints( pNode, curvePoints, outputTubePolyData );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputCardinalSplineModel( vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPolyData* outputTubePolyData )
{
  if (controlPoints == NULL)
  {
    vtkErrorMacro("Control points data structure is null. No model generated.");
    return;
  }
  
  int numberControlPoints = controlPoints->GetNumberOfPoints();
  // redundant error checking, to be safe
  if ( numberControlPoints < LINE_MIN_NUMBER_POINTS )
  {
    vtkErrorMacro( "Not enough points to create an output spline model. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numberControlPoints << " are provided. No output created." );
    return;
  }
  
  // special case, fit a line. Spline fitting will not work with fewer than 3 points
  if ( numberControlPoints == LINE_MIN_NUMBER_POINTS )
  {
    vtkWarningMacro( "Only " << LINE_MIN_NUMBER_POINTS << " provided. Fitting line." );
    UpdateOutputLinearModel(pNode, controlPoints, outputTubePolyData);
    return;
  }

  // Create the splines
  vtkSmartPointer< vtkCardinalSpline > splineX = vtkSmartPointer< vtkCardinalSpline >::New();
  vtkSmartPointer< vtkCardinalSpline > splineY = vtkSmartPointer< vtkCardinalSpline >::New();
  vtkSmartPointer< vtkCardinalSpline > splineZ = vtkSmartPointer< vtkCardinalSpline >::New();
  SetCardinalSplineParameters( pNode, splineX, splineY, splineZ );

  vtkSmartPointer< vtkPoints > curvePoints = vtkSmartPointer< vtkPoints >::New();
  AllocateCurvePoints( pNode, controlPoints, curvePoints );

  // Iterate over the segments to interpolate, add all the "in-between" points
  int numberSegmentsToInterpolate;
  if ( pNode->GetTubeLoop() )
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
  while ( controlPointIndex < numberSegmentsToInterpolate )
  {
    // iterate to compute interpolating points
    for ( int i = 0; i < numberCurvePointsPerControlPoint; i++)
    {
      double interpolationParam = controlPointIndex + i / (double) numberCurvePointsPerControlPoint;
      double curvePoint[ 3 ];
      curvePoint[ 0 ] = splineX->Evaluate( interpolationParam );
      curvePoint[ 1 ] = splineY->Evaluate( interpolationParam );
      curvePoint[ 2 ] = splineZ->Evaluate( interpolationParam );
      int curveIndex = controlPointIndex * numberCurvePointsPerControlPoint + i;
      curvePoints->SetPoint( curveIndex, curvePoint );
    }
    controlPointIndex++;
    controlPointIndexNext++;
  }
  // bring it the rest of the way to the final control point
  controlPointIndex = controlPointIndex % numberControlPoints; // if the index exceeds the max, bring back to 0
  double finalPoint[ 3 ] = { 0.0, 0.0, 0.0 };
  controlPoints->GetPoint( controlPointIndex, finalPoint );
  int finalIndex = numberCurvePointsPerControlPoint * numberSegmentsToInterpolate;
  curvePoints->SetPoint( finalIndex, finalPoint );
  
  // the last part of the curve depends on whether it is a loop or not
  if ( pNode->GetTubeLoop() )
  {
    CloseLoop( curvePoints );
  }
  
  GetTubePolyDataFromPoints( pNode, curvePoints, outputTubePolyData );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputKochanekSplineModel( vtkMRMLMarkupsToModelNode* pNode, vtkPoints* controlPoints, vtkPolyData* outputTubePolyData )
{
  if (controlPoints == NULL)
  {
    vtkErrorMacro("Control points data structure is null. No model generated.");
    return;
  }
  
  int numberControlPoints = controlPoints->GetNumberOfPoints();
  // redundant error checking, to be safe
  if ( numberControlPoints < LINE_MIN_NUMBER_POINTS )
  {
    vtkErrorMacro( "Not enough points to create an output spline model. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numberControlPoints << " are provided. No output created." );
    return;
  }
  
  // special case, fit a line. Spline fitting will not work with fewer than 3 points
  if ( numberControlPoints == LINE_MIN_NUMBER_POINTS )
  {
    vtkWarningMacro( "Only " << LINE_MIN_NUMBER_POINTS << " provided. Fitting line." );
    UpdateOutputLinearModel(pNode, controlPoints, outputTubePolyData);
    return;
  }

  // Create the splines
  vtkSmartPointer< vtkKochanekSpline > splineX = vtkSmartPointer< vtkKochanekSpline >::New();
  vtkSmartPointer< vtkKochanekSpline > splineY = vtkSmartPointer< vtkKochanekSpline >::New();
  vtkSmartPointer< vtkKochanekSpline > splineZ = vtkSmartPointer< vtkKochanekSpline >::New();
  SetKochanekSplineParameters( pNode, splineX, splineY, splineZ );

  vtkSmartPointer< vtkPoints > curvePoints = vtkSmartPointer< vtkPoints >::New();
  AllocateCurvePoints( pNode, controlPoints, curvePoints );

  // Iterate over the segments to interpolate, add all the "in-between" points
  int numberSegmentsToInterpolate;
  if ( pNode->GetTubeLoop() )
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
  while ( controlPointIndex < numberSegmentsToInterpolate )
  {
    // iterate to compute interpolating points
    for ( int i = 0; i < numberCurvePointsPerControlPoint; i++)
    {
      double interpolationParam = controlPointIndex + i / (double) numberCurvePointsPerControlPoint;
      double curvePoint[ 3 ];
      curvePoint[ 0 ] = splineX->Evaluate( interpolationParam );
      curvePoint[ 1 ] = splineY->Evaluate( interpolationParam );
      curvePoint[ 2 ] = splineZ->Evaluate( interpolationParam );
      int curveIndex = controlPointIndex * numberCurvePointsPerControlPoint + i;
      curvePoints->SetPoint( curveIndex, curvePoint );
    }
    controlPointIndex++;
    controlPointIndexNext++;
  }
  // bring it the rest of the way to the final control point
  controlPointIndex = controlPointIndex % numberControlPoints; // if the index exceeds the max, bring back to 0
  double finalPoint[ 3 ] = { 0.0, 0.0, 0.0 };
  controlPoints->GetPoint( controlPointIndex, finalPoint );
  int finalIndex = numberCurvePointsPerControlPoint * numberSegmentsToInterpolate;
  curvePoints->SetPoint( finalIndex, finalPoint );
  
  // the last part of the curve depends on whether it is a loop or not
  if ( pNode->GetTubeLoop() )
  {
    CloseLoop( curvePoints );
  }
  
  GetTubePolyDataFromPoints( pNode, curvePoints, outputTubePolyData );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ComputePointParametersRawIndices( vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters )
{
  if ( controlPoints == NULL )
  {
    vtkErrorMacro( "Markups contains a vtkPoints object that is null." );
    return;
  }

  int numPoints = controlPoints->GetNumberOfPoints();
  // redundant error checking, to be safe
  if ( numPoints < LINE_MIN_NUMBER_POINTS )
  {
    vtkErrorMacro( "Not enough points to compute polynomial parameters. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numPoints << " are provided." );
    return;
  }

  if ( markupsPointsParameters->GetNumberOfTuples() )
  {
    // this should never happen, but in case it does, output a warning
    vtkWarningMacro( "markupsPointsParameters already has contents. Clearing." );
    while ( markupsPointsParameters->GetNumberOfTuples() ) // clear contents just in case
      markupsPointsParameters->RemoveLastTuple();
  }

  for (int v = 0; v < numPoints; v++)
  {
    markupsPointsParameters->InsertNextTuple1( v / double(numPoints-1) );
    // division to clamp all values to range 0.0 - 1.0
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::ComputePointParametersMinimumSpanningTree(vtkPoints * controlPoints, vtkDoubleArray* markupsPointsParameters)
{
  if ( controlPoints == NULL )
  {
    vtkErrorMacro( "Markups contains a vtkPoints object that is null." );
    return;
  }

  int numPoints = controlPoints->GetNumberOfPoints();
  // redundant error checking, to be safe
  if ( numPoints < LINE_MIN_NUMBER_POINTS )
  {
    vtkErrorMacro( "Not enough points to compute polynomial parameters. Need at least " << LINE_MIN_NUMBER_POINTS << " points but " << numPoints << " are provided." );
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
  std::vector< double > distances( numPoints * numPoints );
  distances.assign( numPoints * numPoints, 0.0 );
  // 2. find the two farthest-seperated vertices in the distances array
  int treeStartIndex = 0;
  int treeEndIndex = 0;
  double maximumDistance = 0;
  // iterate through all points
  for ( int v = 0; v < numPoints; v++ )
  {
    for ( int u = 0; u < numPoints; u++ )
    {
      double pointU[ 3 ], pointV[ 3 ];
      controlPoints->GetPoint( u, pointU );
      controlPoints->GetPoint( v, pointV );
      double distX = ( pointU[ 0 ] - pointV[ 0 ] ); double distXsq = distX * distX;
      double distY = ( pointU[ 1 ] - pointV[ 1 ] ); double distYsq = distY * distY;
      double distZ = ( pointU[ 2 ] - pointV[ 2 ] ); double distZsq = distZ * distZ;
      double dist3D = sqrt( distXsq + distYsq + distZsq );
      distances[ v * numPoints + u ] = dist3D;
      if ( dist3D > maximumDistance )
      {
        maximumDistance = dist3D;
        treeStartIndex = v;
        treeEndIndex = u;
      }
    }
  }
  // use the 1D vector as a 2D vector
  std::vector< double* > graph( numPoints );
  for ( int v = 0; v < numPoints; v++ )
  {
    graph[ v ] = &( distances[ v * numPoints ] );
  }

  // implementation of Prim's algorithm heavily based on:
  // http://www.geeksforgeeks.org/greedy-algorithms-set-5-prims-minimum-spanning-tree-mst-2/
  std::vector< int > parent( numPoints ); // Array to store constructed MST
  std::vector< double > key( numPoints );   // Key values used to pick minimum weight edge in cut
  std::vector< bool > mstSet( numPoints );  // To represent set of vertices not yet included in MST
 
  // Initialize all keys as INFINITE (or at least as close as we can get)
  for ( int i = 0; i < numPoints; i++ )
  {
    key[ i ] = VTK_DOUBLE_MAX;
    mstSet[ i ] = false;
  }
 
  // Always include first 1st vertex in MST.
  key[ treeStartIndex ] = 0.0;     // Make key 0 so that this vertex is picked as first vertex
  parent[ treeStartIndex ] = -1; // First node is always root of MST 
 
  // The MST will have numPoints vertices
  for ( int count = 0; count < numPoints - 1; count++ )
  {
    // Pick the minimum key vertex from the set of vertices
    // not yet included in MST
    int nextPointIndex = -1;
    double minDistance = VTK_DOUBLE_MAX, min_index;
    for ( int v = 0; v < numPoints; v++ )
    {
      if ( mstSet[ v ] == false && key[ v ] < minDistance )
      {
        minDistance = key[ v ];
        nextPointIndex = v;
      }
    }
 
    // Add the picked vertex to the MST Set
    mstSet[ nextPointIndex ] = true;
 
    // Update key value and parent index of the adjacent vertices of
    // the picked vertex. Consider only those vertices which are not yet
    // included in MST
    for ( int v = 0; v < numPoints; v++ )
    {
      // graph[u][v] is non zero only for adjacent vertices of m
      // mstSet[v] is false for vertices not yet included in MST
      // Update the key only if graph[u][v] is smaller than key[v]
      if ( graph[ nextPointIndex ][ v ] >= 0 && 
           mstSet[ v ] == false &&
           graph[ nextPointIndex ][ v ] <  key[ v ])
      {
        parent[ v ] = nextPointIndex;
        key[ v ] = graph[ nextPointIndex ][ v ];
      }
    }
  }
 
  // determine the "trunk" path of the tree, from first index to last index
  std::vector< int > pathIndices;
  int currentPathIndex = treeEndIndex;
  while ( currentPathIndex != -1 )
  {
    pathIndices.push_back( currentPathIndex );
    currentPathIndex = parent[ currentPathIndex ]; // go up the tree one layer
  }
    
  // find the sum of distances along the trunk path of the tree
  double sumOfDistances = 0.0;
  for ( int i = 0; i < pathIndices.size() - 1; i++ )
  {
    sumOfDistances += graph[ i ][ i + 1 ];
  }

  // check this to prevent a division by zero (in case all points are duplicates)
  if ( sumOfDistances == 0 )
  {
    vtkErrorMacro( "Minimum spanning tree path has distance zero. No parameters will be assigned. Check inputs!" );
    return;
  }

  // find the parameters along the trunk path of the tree
  std::vector< double > pathParameters;
  double currentDistance = 0.0;
  for ( int i = 0; i < pathIndices.size() - 1; i++ )
  {
    pathParameters.push_back( currentDistance / sumOfDistances );
    currentDistance += graph[ i ][ i + 1 ];
  }
  pathParameters.push_back( currentDistance / sumOfDistances ); // this should be 1.0

  // finally assign polynomial parameters to each point, and store in the output array
  if ( markupsPointsParameters->GetNumberOfTuples() > 0 )
  {
    // this should never happen, but in case it does, output a warning
    vtkWarningMacro( "markupsPointsParameters already has contents. Clearing." );
    while ( markupsPointsParameters->GetNumberOfTuples() ) // clear contents just in case
      markupsPointsParameters->RemoveLastTuple();
  }

  for ( int i = 0; i < numPoints; i++ )
  {
    int currentIndex = i;
    bool alongPath = false;
    int indexAlongPath = -1;
    for ( int j = 0; j < pathIndices.size(); j++ )
    {
      if ( pathIndices[ j ] == currentIndex )
      {
        alongPath = true;
        indexAlongPath = j;
        break;
      }
    }
    while ( !alongPath )
    {
      currentIndex = parent[ currentIndex ];
      for ( int j = 0; j < pathIndices.size(); j++ )
      {
        if ( pathIndices[ j ] == currentIndex )
        {
          alongPath = true;
          indexAlongPath = j;
          break;
        }
      }
    }
    markupsPointsParameters->InsertNextTuple1( pathParameters[ indexAlongPath ] );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputPolynomialFitModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters, vtkPolyData* outputPolyData)
{
  if ( controlPoints == NULL )
  {
    vtkErrorMacro("Markups contains a vtkPoints object that is null. No model generated.");
    return;
  }

  int numPoints = controlPoints->GetNumberOfPoints();
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
    UpdateOutputLinearModel(markupsToModelModuleNode, controlPoints, outputPolyData);
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
        pointMm[d] += coefficient * std::pow( (double(p)/(numPointsOnCurve-1)), c );
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
void vtkSlicerMarkupsToModelLogic::UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* pNode, vtkPolyData* outputPolyData)
{

  vtkMRMLMarkupsFiducialNode* markupsNode = pNode->GetMarkupsNode( );
  if( markupsNode == NULL )
  {
    return;
  }

  int numberOfMarkups = markupsNode->GetNumberOfFiducials();
  if (numberOfMarkups < LINE_MIN_NUMBER_POINTS) // check this here, but also perform redundant checks elsewhere
  {
    return;
  }
  
  vtkSmartPointer< vtkPoints > controlPoints = vtkSmartPointer< vtkPoints >::New();
  MarkupsToPoints( pNode, controlPoints );

  switch( pNode->GetInterpolationType() )
  {
    case vtkMRMLMarkupsToModelNode::Linear:
    {
      UpdateOutputLinearModel( pNode, controlPoints, outputPolyData );
      break;
    }
    case vtkMRMLMarkupsToModelNode::CardinalSpline:
    {
      UpdateOutputCardinalSplineModel( pNode, controlPoints, outputPolyData );
      break;
    }
    case vtkMRMLMarkupsToModelNode::KochanekSpline:
    {
      UpdateOutputKochanekSplineModel( pNode, controlPoints, outputPolyData );
      break;
    }
    case vtkMRMLMarkupsToModelNode::Polynomial:
    {
      vtkSmartPointer<vtkDoubleArray> pointParameters = vtkSmartPointer<vtkDoubleArray>::New();
      switch ( pNode->GetPointParameterType() )
      {
        case vtkMRMLMarkupsToModelNode::RawIndices:
        {
          ComputePointParametersRawIndices( controlPoints, pointParameters );
          break;
        }
        case vtkMRMLMarkupsToModelNode::MinimumSpanningTree:
        {
          ComputePointParametersMinimumSpanningTree( controlPoints, pointParameters );
          break;
        }
        default:
        {
          vtkWarningMacro( "Invalid PointParameterType: " << pNode->GetPointParameterType() << ". Using raw indices." );
          ComputePointParametersRawIndices( controlPoints, pointParameters );
          break;
        }
      }
      UpdateOutputPolynomialFitModel( pNode, controlPoints, pointParameters,outputPolyData );
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
