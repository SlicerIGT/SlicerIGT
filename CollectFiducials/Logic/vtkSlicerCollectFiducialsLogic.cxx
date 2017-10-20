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

// CollectFiducials includes
#include "vtkSlicerCollectFiducialsLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

// STD includes
#include <cassert>
#include <sstream>


vtkStandardNewMacro(vtkSlicerCollectFiducialsLogic);

//------------------------------------------------------------------------------
vtkSlicerCollectFiducialsLogic::vtkSlicerCollectFiducialsLogic()
{
}

//------------------------------------------------------------------------------
vtkSlicerCollectFiducialsLogic::~vtkSlicerCollectFiducialsLogic()
{
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::AddPoint( vtkMRMLCollectFiducialsNode* collectFiducialsNode )
{
  if ( collectFiducialsNode == NULL )
  {
    vtkErrorMacro( "No parameter node set. Will not add any points." );
    return;
  }

  // find the point coordinates
  vtkSmartPointer< vtkMRMLLinearTransformNode > probeNode = vtkMRMLLinearTransformNode::SafeDownCast( collectFiducialsNode->GetProbeTransformNode() );
  if ( probeNode == NULL )
  {
    vtkErrorMacro( "No probe transform node set. Will not add any points." );
    return;
  }
  vtkSmartPointer< vtkMatrix4x4 > probeToWorld = vtkSmartPointer< vtkMatrix4x4 >::New();
  probeNode->GetMatrixTransformToWorld( probeToWorld );
  double pointCoordinates[ 3 ] = { probeToWorld->GetElement( 0, 3 ), probeToWorld->GetElement( 1, 3 ), probeToWorld->GetElement( 2, 3 ) };

  vtkMRMLNode* outputNode = collectFiducialsNode->GetOutputNode();
  if ( outputNode == NULL )
  {
    vtkErrorMacro( "No output node set. Will not add any points." );
    return;
  }

  // determine the minimum distance for adding the point (only applies if in auto-collect mode)
  double minimumDistanceMm = 0.0; // default value, 0 means there is no minimum
  if ( collectFiducialsNode->GetCollectMode() == vtkMRMLCollectFiducialsNode::Automatic )
  {
    minimumDistanceMm = collectFiducialsNode->GetMinimumDistanceMm();
  }

  // try downcasting the output node to different types.
  // If successfully downcasts, then call the relevant function to add the point.
  // If it does not downcast, output an error.
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
  if ( outputMarkupsNode != NULL )
  {
    // add the label to the point
    std::stringstream ss;
    ss << collectFiducialsNode->GetLabelBase() << collectFiducialsNode->GetLabelCounter();

    this->AddPointToMarkups( outputMarkupsNode, pointCoordinates, ss.str(), minimumDistanceMm );

    // always increase the label counter
    collectFiducialsNode->SetLabelCounter( collectFiducialsNode->GetLabelCounter() + 1 );
  }
  else if ( outputModelNode != NULL )
  {
    this->AddPointToModel( outputModelNode, pointCoordinates, minimumDistanceMm );
  }
  else
  {
    vtkErrorMacro( "Could not recognize the type of output node. Will not add any points." );
    return;
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::RemoveLastPoint( vtkMRMLCollectFiducialsNode* collectFiducialsNode )
{
  vtkMRMLNode* outputNode = collectFiducialsNode->GetOutputNode();
  if ( outputNode == NULL )
  {
    vtkErrorMacro( "No output node set. Will not remove any points." );
    return;
  }

  // try downcasting the output node to different types.
  // If successfully downcasts, then call the relevant function to remove the point(s).
  // If it does not downcast, output an error.
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
  if ( outputMarkupsNode != NULL )
  {
    int numberOfPoints = outputMarkupsNode->GetNumberOfFiducials();
    if ( numberOfPoints == 0 )
    {
      return; // nothing to do
    }
    vtkIdType lastPointId = numberOfPoints - 1;
    outputMarkupsNode->RemoveMarkup( lastPointId );
  }
  else if ( outputModelNode != NULL )
  {
    this->RemoveLastPointFromModel( outputModelNode );
  }
  else
  {
    vtkErrorMacro( "Could not recognize the type of output node. Will not remove any points." );
    return;
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::RemoveAllPoints( vtkMRMLCollectFiducialsNode* collectFiducialsNode )
{
  vtkMRMLNode* outputNode = collectFiducialsNode->GetOutputNode();
  if ( outputNode == NULL )
  {
    vtkErrorMacro( "No output node set. Will not remove any points." );
    return;
  }

  // try downcasting the output node to different types.
  // If successfully downcasts, then call the relevant function to remove the point(s).
  // If it does not downcast, output an error.
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
  if ( outputMarkupsNode != NULL )
  {
    outputMarkupsNode->RemoveAllMarkups();
  }
  else if ( outputModelNode != NULL )
  {
    outputModelNode->SetAndObservePolyData( vtkSmartPointer< vtkPolyData >::New() );
  }
  else
  {
    vtkErrorMacro( "Could not recognize the type of output node. Will not remove any points." );
    return;
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLScene::NodeAddedEvent );
  events->InsertNextValue( vtkMRMLScene::NodeRemovedEvent );
  this->SetAndObserveMRMLSceneEventsInternal( newScene, events.GetPointer() );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::RegisterNodes()
{
  if( !this->GetMRMLScene() )
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLCollectFiducialsNode >::New() );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLCollectFiducialsNode* collectFiducialsNode = vtkMRMLCollectFiducialsNode::SafeDownCast( node );
  if ( collectFiducialsNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( collectFiducialsNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLCollectFiducialsNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( collectFiducialsNode, events.GetPointer() );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or node");
    return;
  }
  
  if ( node->IsA( "vtkMRMLCollectFiducialsNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* vtkNotUsed(callData) )
{
  vtkMRMLCollectFiducialsNode* collectFiducialsNode = vtkMRMLCollectFiducialsNode::SafeDownCast( caller );
  if ( collectFiducialsNode == NULL )
  {
    vtkErrorMacro( "No parameter node set. Aborting." );
    return;
  }
  
  if ( event == vtkMRMLCollectFiducialsNode::InputDataModifiedEvent )
  {
    if ( collectFiducialsNode->GetCollectMode() == vtkMRMLCollectFiducialsNode::Automatic )
    {
      if ( collectFiducialsNode->GetOutputNode() == NULL || collectFiducialsNode->GetProbeTransformNode() == NULL )
      {
        vtkWarningMacro( "Collect fiducials node is not fully set up. Setting to manual collection." );
        collectFiducialsNode->SetCollectModeToManual();
        return;
      }
      this->AddPoint( collectFiducialsNode ); // Will create modified event to update widget
    }
  }
}

//------------------------------------------------------------------------------
// note: point coordinates MUST contain 3 values
void vtkSlicerCollectFiducialsLogic::AddPointToModel( vtkMRMLModelNode* modelNode, double* pointCoordinates, double minimumDistanceFromPreviousPointMm )
{
  if ( modelNode == NULL )
  {
    vtkErrorMacro( "Output node is null. No points added." );
    return;
  }

  vtkSmartPointer< vtkPolyData > polyData = modelNode->GetPolyData();
  if ( polyData == NULL )
  {
    polyData = vtkSmartPointer< vtkPolyData >::New();
    modelNode->SetAndObservePolyData( polyData );
  }

  vtkSmartPointer< vtkPoints > points = polyData->GetPoints();
  if ( points == NULL )
  {
    points = vtkSmartPointer< vtkPoints >::New();
    polyData->SetPoints( points );
  }

  int numberOfPoints = ( int )points->GetNumberOfPoints();
  if ( minimumDistanceFromPreviousPointMm > 0.0 && numberOfPoints > 0 )
  {
    double previousCoordinates[ 3 ];
    points->GetPoint( numberOfPoints - 1, previousCoordinates );
    double distanceMm = sqrt( vtkMath::Distance2BetweenPoints( pointCoordinates, previousCoordinates ) );
    if ( distanceMm < minimumDistanceFromPreviousPointMm )
    {
      return;
    }
  }
  points->InsertNextPoint( pointCoordinates );

  this->UpdateCellsForPolyData( polyData );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::RemoveLastPointFromModel( vtkMRMLModelNode* modelNode )
{
  if ( modelNode == NULL )
  {
    vtkErrorMacro( "Output node is null. No points removed." );
    return;
  }
  
  vtkSmartPointer< vtkPolyData > polyData = modelNode->GetPolyData();
  if ( polyData == NULL )
  {
    polyData = vtkSmartPointer< vtkPolyData >::New();
    modelNode->SetAndObservePolyData( polyData );
  }

  vtkSmartPointer< vtkPoints > oldPoints = polyData->GetPoints();
  if ( oldPoints == NULL || oldPoints->GetNumberOfPoints() == 0 )
  {
    polyData->SetPoints( vtkSmartPointer< vtkPoints >::New() );
    return; // nothing to do
  }

  vtkSmartPointer< vtkPoints > newPoints = vtkSmartPointer< vtkPoints >::New();
  int numberOfPointsToRetain = ( int )oldPoints->GetNumberOfPoints() - 1;
  newPoints->SetNumberOfPoints( numberOfPointsToRetain );
  for ( vtkIdType ptId = 0; ptId < numberOfPointsToRetain; ptId++ )
  {
    double* point = oldPoints->GetPoint( ptId );
    newPoints->SetPoint( ptId, point );
  }
  polyData->SetPoints( newPoints );

  this->UpdateCellsForPolyData( polyData );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::UpdateCellsForPolyData( vtkPolyData* polyData )
{
  if ( polyData == NULL )
  {
    vtkErrorMacro( "Poly data is null. Will not update cells." );
    return;
  }

  // update vertices
  // TODO: Allocate more intelligently...? Would be better to simply add/remove vertices as needed, but this causes a crash
  int numberOfPoints = ( int )polyData->GetNumberOfPoints();
  vtkSmartPointer< vtkCellArray > verticesCellArray = vtkSmartPointer< vtkCellArray >::New();
  verticesCellArray->Allocate( verticesCellArray->EstimateSize( numberOfPoints, 1 ) );
  for ( vtkIdType ptId = 0; ptId < numberOfPoints; ptId++ )
  {
    verticesCellArray->InsertNextCell( 1 );
    verticesCellArray->InsertCellPoint( ptId );
  }
  polyData->SetVerts( verticesCellArray );

  // edges and faces are likely to become meaningless as individual points are added or removed
  polyData->SetLines( NULL );
  polyData->SetPolys( NULL );
}

//------------------------------------------------------------------------------
// note: point coordinates MUST contain 3 values
void vtkSlicerCollectFiducialsLogic::AddPointToMarkups( vtkMRMLMarkupsFiducialNode* markupsNode, double* pointCoordinates, std::string markupLabel, double minimumDistanceFromPreviousPointMm )
{
  if ( markupsNode == NULL )
  {
    vtkErrorMacro( "Output node is null. No points added." );
    return;
  }

  // if in automatic collection mode, make sure sufficient there is sufficient distance from previous point
  int numberOfPoints = markupsNode->GetNumberOfFiducials();
  if ( minimumDistanceFromPreviousPointMm > 0.0 && numberOfPoints > 0 )
  {
    double previousCoordinates[ 3 ];
    markupsNode->GetNthFiducialPosition( numberOfPoints - 1, previousCoordinates );
    double distanceMm = sqrt( vtkMath::Distance2BetweenPoints( pointCoordinates, previousCoordinates ) );
    if ( distanceMm < minimumDistanceFromPreviousPointMm )
    {
      return;
    }
  }

  // Add point to the markups node
  int pointIndexInMarkups = markupsNode->AddFiducialFromArray( pointCoordinates );
  markupsNode->SetNthFiducialLabel( pointIndexInMarkups, markupLabel.c_str() );
}
