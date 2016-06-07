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
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDelaunay3D.h>
#include <vtkIntArray.h>
#include <vtkKochanekSpline.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkSphereSource.h>
#include <vtkSplineFilter.h>
#include <vtkTubeFilter.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsToModelLogic);

//----------------------------------------------------------------------------
vtkSlicerMarkupsToModelLogic::vtkSlicerMarkupsToModelLogic()
{
  ImportingScene=0;
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
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
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

      if( markupsToModelNode->GetModelNodeName().compare("")!=0 && markupsToModelNode->GetModelNode()==NULL)
      {
        vtkMRMLNode* modelNodeFromScene = this->GetMRMLScene()->GetNodeByID(markupsToModelNode->GetModelNodeID());
        if(modelNodeFromScene!=NULL)
        {
          markupsToModelNode->SetModelNode(vtkMRMLModelNode::SafeDownCast(modelNodeFromScene));
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
  ImportingScene=0;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::OnMRMLSceneStartImport()
{
  ImportingScene=1;
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
  moduleNode->SetMarkupsNodeID(newMarkups->GetID());

  // Switch to the new model node
  moduleNode->SetAndObserveMarkupsNodeID( (newMarkups!=NULL) ? newMarkups->GetID() : NULL );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateSelectionNode( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode )
{
  vtkMRMLMarkupsFiducialNode* markupsNode = markupsToModelModuleNode->GetMarkupsNode(); 
  if(markupsNode == NULL)
  {
    vtkWarningMacro("No markups yet");
    return;
  }
  std::string selectionNodeID = std::string("");

  if (!this->GetMRMLScene())
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
void vtkSlicerMarkupsToModelLogic::UpdateOutputCloseSurfaceModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{
  vtkMRMLMarkupsFiducialNode* markups = markupsToModelModuleNode->GetMarkupsNode(); 
  if(markups == NULL)
  {
    vtkWarningMacro("No markups yet");
    return;
  }
  int numberOfMarkups = markups->GetNumberOfFiducials();

  if(numberOfMarkups< MINIMUM_MARKUPS_CLOSED_SURFACE_NUMBER)
  {
    //vtkWarningMacro("Not enough fiducials for closed surface");
    if(markupsToModelModuleNode->GetModelNode()!=NULL)
    {
      vtkSmartPointer< vtkSphereSource > sphereSource = vtkSmartPointer< vtkSphereSource >::New();
      markupsToModelModuleNode->GetModelNode()->GetPolyData()->Reset();
      sphereSource->SetRadius(0.00001);
      markupsToModelModuleNode->GetModelNode()->SetPolyDataConnection(sphereSource->GetOutputPort());
      //vtkWarningMacro("RESET");
    }
    return;
  }

  vtkSmartPointer< vtkPoints > modelPoints = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkCellArray > modelCellArray = vtkSmartPointer< vtkCellArray >::New();

  modelPoints->SetNumberOfPoints(numberOfMarkups);
  double markupPoint [3] = {0.0, 0.0, 0.0};

  for (int i=0; i<numberOfMarkups;i++)
  {
    markups->GetNthFiducialPosition(i,markupPoint);
    modelPoints->SetPoint(i, markupPoint);
  }

  modelCellArray->InsertNextCell(numberOfMarkups);
  for (int i=0; i < numberOfMarkups;i++)
  {
    modelCellArray->InsertCellPoint(i);
  }

  vtkSmartPointer< vtkPolyData > pointPolyData = vtkSmartPointer< vtkPolyData >::New();
  pointPolyData->SetLines(modelCellArray);
  pointPolyData->SetPoints(modelPoints);

  vtkSmartPointer< vtkDelaunay3D > delaunay = vtkSmartPointer< vtkDelaunay3D >::New();
  delaunay->SetAlpha(markupsToModelModuleNode->GetDelaunayAlpha());
  if(markupsToModelModuleNode->GetCleanMarkups())
  {
    vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New();
#if (VTK_MAJOR_VERSION <= 5)
    cleanPointPolyData->SetInput(pointPolyData);
#else
    cleanPointPolyData->SetInputData(pointPolyData);
#endif
    cleanPointPolyData->SetTolerance(CLEAN_POLYDATA_TOLERANCE);
    delaunay->SetInputConnection(cleanPointPolyData->GetOutputPort()); //TODO SET VTK5
  }
  else
  {
#if (VTK_MAJOR_VERSION <= 5)
    delaunay->SetInput( pointPolyData );
#else
    delaunay->SetInputData(pointPolyData);
#endif
  }

  vtkSmartPointer< vtkDataSetSurfaceFilter > surfaceFilter = vtkSmartPointer< vtkDataSetSurfaceFilter >::New();
  surfaceFilter->SetInputConnection(delaunay->GetOutputPort());

  vtkSmartPointer< vtkMRMLModelNode > modelNode;
  if(markupsToModelModuleNode->GetModelNode() == NULL)
  {
    modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
    this->GetMRMLScene()->AddNode( modelNode );
    modelNode->SetName( markupsToModelModuleNode->GetModelNodeName().c_str() );
  }
  else
  {
    modelNode = markupsToModelModuleNode->GetModelNode();
  }

  if(markupsToModelModuleNode->GetButterflySubdivision())
  {
    vtkSmartPointer< vtkButterflySubdivisionFilter > subdivisionFilter = vtkSmartPointer< vtkButterflySubdivisionFilter >::New();
    subdivisionFilter->SetInputConnection(surfaceFilter->GetOutputPort());
    subdivisionFilter->SetNumberOfSubdivisions(3);
    subdivisionFilter->Update();
    modelNode->SetAndObservePolyData( subdivisionFilter->GetOutput() );
    if(markupsToModelModuleNode->GetConvexHull())
    {
      vtkSmartPointer<vtkDelaunay3D> convexHull = vtkSmartPointer<vtkDelaunay3D>::New();
      convexHull->SetAlpha(markupsToModelModuleNode->GetDelaunayAlpha());
      convexHull->SetInputConnection(subdivisionFilter->GetOutputPort());
      convexHull->Update();
      vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
      surfaceFilter->SetInputData(convexHull->GetOutput());
      surfaceFilter->Update();
      vtkPolyData* polyData = surfaceFilter->GetOutput();
      modelNode->SetAndObservePolyData(polyData);
    }
  }
  else
  {
    surfaceFilter->Update();
    modelNode->SetAndObservePolyData( surfaceFilter->GetOutput() );
  }

  if(markupsToModelModuleNode->GetModelNode() == NULL)
  {
    vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
    this->GetMRMLScene()->AddNode( displayNode );
    displayNode->SetName( markupsToModelModuleNode->GetDisplayNodeName().c_str());
    modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
    markupsToModelModuleNode->SetModelNode(modelNode);
  }
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
void vtkSlicerMarkupsToModelLogic::UpdateOutputLinearModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData)
{
  vtkSmartPointer< vtkMRMLModelNode > modelNode;
  modelNode = markupsToModelModuleNode->GetModelNode();
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
  modelNode->SetAndObservePolyData( append->GetOutput() );
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputHermiteSplineModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData)
{
  vtkSmartPointer< vtkMRMLModelNode > modelNode;
    modelNode = markupsToModelModuleNode->GetModelNode();

  int totalNumberOfPoints = markupsToModelModuleNode->GetNumberOfIntermediatePoints()*markupsPointsPolyData->GetNumberOfPoints();
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
  cardinalSplineTubeFilter->SetNumberOfSides(20);
  cardinalSplineTubeFilter->CappingOn();
  cardinalSplineTubeFilter->Update();

  modelNode->SetAndObservePolyData( cardinalSplineTubeFilter->GetOutput() );

  if(markupsToModelModuleNode->GetModelNode() == NULL)
  {
    vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
    this->GetMRMLScene()->AddNode( displayNode );
    displayNode->SetName( markupsToModelModuleNode->GetDisplayNodeName().c_str());
    modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
    markupsToModelModuleNode->SetModelNode(modelNode);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{

  vtkMRMLMarkupsFiducialNode* markupsNode=markupsToModelModuleNode->GetMarkupsNode(); 
  if(markupsNode==NULL)
  {
    //vtkWarningMacro("No markups yet");
    return;
  }

  int numberOfMarkups = markupsNode->GetNumberOfFiducials();
  if(numberOfMarkups< MINIMUM_MARKUPS_NUMBER )
  {
      if(markupsToModelModuleNode->GetModelNode()!=NULL)
      {
        vtkSmartPointer< vtkSphereSource > sphereSource = vtkSmartPointer< vtkSphereSource >::New();
        markupsToModelModuleNode->GetModelNode()->GetPolyData()->Reset();
        sphereSource->SetRadius(0.00001);
        markupsToModelModuleNode->GetModelNode()->SetPolyDataConnection(sphereSource->GetOutputPort());
      }
    return;
  }

  vtkSmartPointer< vtkMRMLModelNode > modelNode;
  if(markupsToModelModuleNode->GetModelNode() == NULL)
  {
    modelNode = vtkSmartPointer< vtkMRMLModelNode >::New();
    this->GetMRMLScene()->AddNode( modelNode );
    modelNode->SetName( markupsToModelModuleNode->GetModelNodeName().c_str() );
    vtkSmartPointer< vtkMRMLModelDisplayNode > displayNode = vtkSmartPointer< vtkMRMLModelDisplayNode >::New();
    this->GetMRMLScene()->AddNode( displayNode );
    displayNode->SetName( markupsToModelModuleNode->GetDisplayNodeName().c_str());
    modelNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
    markupsToModelModuleNode->SetModelNode(modelNode);
  }

  vtkSmartPointer< vtkPolyData > markupsPointsPolyData = vtkSmartPointer< vtkPolyData >::New();
  markupsToPath( markupsNode, markupsPointsPolyData);
  vtkSmartPointer< vtkPolyData > finalMarkupsPointsPolyData;

  vtkSmartPointer< vtkCleanPolyData > cleanPointPolyData = vtkSmartPointer< vtkCleanPolyData >::New();
  if(markupsToModelModuleNode->GetCleanMarkups())
  {
    //vtkWarningMacro("PUTOS " << markupsPointsPolyData->GetNumberOfPoints() ); 

#if (VTK_MAJOR_VERSION <= 5)
    cleanPointPolyData->SetInput(markupsPointsPolyData);
#else
    cleanPointPolyData->SetInputData(markupsPointsPolyData);
#endif
    cleanPointPolyData->SetTolerance(CLEAN_POLYDATA_TOLERANCE);
    cleanPointPolyData->Update();
    finalMarkupsPointsPolyData= cleanPointPolyData->GetOutput();
    //vtkWarningMacro("PUNTOS " << finalMarkupsPointsPolyData->GetNumberOfPoints() );
  }
  else
  {
    finalMarkupsPointsPolyData=markupsPointsPolyData;
  }

  switch(markupsToModelModuleNode->GetInterpolationType())
  {
  case vtkMRMLMarkupsToModelNode::Linear: UpdateOutputLinearModel(markupsToModelModuleNode,finalMarkupsPointsPolyData); break;
  case vtkMRMLMarkupsToModelNode::CardinalSpline: UpdateOutputHermiteSplineModel(markupsToModelModuleNode,finalMarkupsPointsPolyData); break;
  case vtkMRMLMarkupsToModelNode::KochanekSpline: UpdateOutputHermiteSplineModel(markupsToModelModuleNode,finalMarkupsPointsPolyData); break;
  }
}

//------------------------------------------------------------------------------
void vtkSlicerMarkupsToModelLogic::UpdateOutputModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode)
{
  if(ImportingScene==1)
  {
    return;
  }
  switch(markupsToModelModuleNode->GetModelType())
  {
  case vtkMRMLMarkupsToModelNode::ClosedSurface: UpdateOutputCloseSurfaceModel(markupsToModelModuleNode); break;
  case vtkMRMLMarkupsToModelNode::Curve: UpdateOutputCurveModel(markupsToModelModuleNode); break;
  }
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


