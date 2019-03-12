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

// CollectPoints includes
#include "vtkSlicerCollectPointsLogic.h"

// MRML includes
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

// STD includes
#include <cassert>
#include <sstream>


vtkStandardNewMacro(vtkSlicerCollectPointsLogic);

//------------------------------------------------------------------------------
vtkSlicerCollectPointsLogic::vtkSlicerCollectPointsLogic()
{
}

//------------------------------------------------------------------------------
vtkSlicerCollectPointsLogic::~vtkSlicerCollectPointsLogic()
{
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::AddPoint( vtkMRMLCollectPointsNode* collectPointsNode )
{
  // basic error checking
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "No parameter node set. Will not add any points." );
    return;
  }

  vtkMRMLNode* outputNode = collectPointsNode->GetOutputNode();
  if ( outputNode == NULL )
  {
    vtkErrorMacro( "No output node set. Will not add any points." );
    return;
  }

  // find the point coordinates
  double pointCoordinates[ 3 ] = { 0.0, 0.0, 0.0 }; // temporary values
  bool success = this->ComputePointCoordinates( collectPointsNode, pointCoordinates );
  if ( !success )
  {
    vtkErrorMacro( "Could not compute point coordinates. Will not add any points." );
    return;
  }

  // try downcasting the output node to different types.
  // If successfully downcasts, then call the relevant function to add the point.
  // If it does not downcast, output an error.
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
  vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
  if ( outputMarkupsNode != NULL )
  {
    this->AddPointToMarkups( collectPointsNode, pointCoordinates );

  }
  else if ( outputModelNode != NULL )
  {
    this->AddPointToModel( collectPointsNode, pointCoordinates );
  }
  else
  {
    vtkErrorMacro( "Could not recognize the type of output node. Will not add any points." );
    return;
  }
}

//------------------------------------------------------------------------------
bool vtkSlicerCollectPointsLogic::ComputePointCoordinates( vtkMRMLCollectPointsNode* collectPointsNode, double outputPointCoordinates[ 3 ] )
{
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "No parameter node set. Cannot compute point coordinates." );
    return false;
  }

  // find the point coordinates
  vtkSmartPointer< vtkMRMLTransformNode > samplingNode = vtkMRMLTransformNode::SafeDownCast( collectPointsNode->GetSamplingTransformNode() );
  vtkSmartPointer< vtkMRMLTransformNode > anchorNode = vtkMRMLTransformNode::SafeDownCast( collectPointsNode->GetAnchorTransformNode() );
  vtkSmartPointer < vtkGeneralTransform > samplingToAnchorTransform = vtkSmartPointer< vtkGeneralTransform >::New();
  vtkMRMLTransformNode::GetTransformBetweenNodes( samplingNode, anchorNode, samplingToAnchorTransform ); // parameters are: source, target, transform
  double samplingPoint[ 3 ] = { 0, 0, 0 }; // origin of coordinate system
  samplingToAnchorTransform->TransformPoint( samplingPoint, outputPointCoordinates );
  return true;
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::RemoveLastPoint( vtkMRMLCollectPointsNode* collectPointsNode )
{
  vtkMRMLNode* outputNode = collectPointsNode->GetOutputNode();
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
void vtkSlicerCollectPointsLogic::RemoveAllPoints( vtkMRMLCollectPointsNode* collectPointsNode )
{
  vtkMRMLNode* outputNode = collectPointsNode->GetOutputNode();
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
    outputMarkupsNode->RemoveAllControlPoints();
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
void vtkSlicerCollectPointsLogic::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLScene::NodeAddedEvent );
  events->InsertNextValue( vtkMRMLScene::NodeRemovedEvent );
  this->SetAndObserveMRMLSceneEventsInternal( newScene, events.GetPointer() );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::RegisterNodes()
{
  if( !this->GetMRMLScene() )
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLCollectPointsNode >::New() );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( node );
  if ( collectPointsNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( collectPointsNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLCollectPointsNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( collectPointsNode, events.GetPointer() );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or node");
    return;
  }
  
  if ( node->IsA( "vtkMRMLCollectPointsNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* vtkNotUsed(callData) )
{
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( caller );
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "No parameter node set. Aborting." );
    return;
  }
  
  if ( event == vtkMRMLCollectPointsNode::InputDataModifiedEvent )
  {
    if ( collectPointsNode->GetCollectMode() == vtkMRMLCollectPointsNode::Automatic )
    {
      if ( collectPointsNode->GetOutputNode() == NULL )
      {
        vtkWarningMacro( "Collect fiducials node is not fully set up, there needs to be an output node." );
        return;
      }
      this->AddPoint( collectPointsNode ); // Will create modified event to update widget
    }
  }
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::AddPointToModel( vtkMRMLCollectPointsNode* collectPointsNode, double pointCoordinates[ 3 ] )
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast( collectPointsNode->GetOutputNode() );
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
  if ( numberOfPoints > 0 )
  {
    double minimumDistanceFromPreviousPoint = collectPointsNode->GetMinimumDistance();
    // minimum distance for adding point only applies if in auto-collect mode
    if ( minimumDistanceFromPreviousPoint > 0.0 && collectPointsNode->GetCollectMode() == vtkMRMLCollectPointsNode::Automatic )
    {
      double previousCoordinates[ 3 ];
      points->GetPoint( numberOfPoints - 1, previousCoordinates );
      double distance = sqrt( vtkMath::Distance2BetweenPoints( pointCoordinates, previousCoordinates ) );
      if ( distance < minimumDistanceFromPreviousPoint )
      {
        return;
      }
    }
  }
  points->InsertNextPoint( pointCoordinates );

  this->UpdateCellsForPolyData( polyData );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectPointsLogic::RemoveLastPointFromModel( vtkMRMLModelNode* modelNode )
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
void vtkSlicerCollectPointsLogic::UpdateCellsForPolyData( vtkPolyData* polyData )
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
void vtkSlicerCollectPointsLogic::AddPointToMarkups( vtkMRMLCollectPointsNode* collectPointsNode, double pointCoordinates[ 3 ] )
{
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( collectPointsNode->GetOutputNode() );
  if ( markupsNode == NULL )
  {
    vtkErrorMacro( "Output node is null. No points added." );
    return;
  }

  // if in automatic collection mode, make sure sufficient there is sufficient distance from previous point
  int numberOfPoints = markupsNode->GetNumberOfFiducials();
  if ( numberOfPoints > 0 )
  {
    double minimumDistanceFromPreviousPoint = collectPointsNode->GetMinimumDistance();
    // minimum distance for adding point only applies if in auto-collect mode
    if ( minimumDistanceFromPreviousPoint > 0.0 && collectPointsNode->GetCollectMode() == vtkMRMLCollectPointsNode::Automatic )
    {
      double previousCoordinates[ 3 ];
      markupsNode->GetNthFiducialPosition( numberOfPoints - 1, previousCoordinates );
      double distance = sqrt( vtkMath::Distance2BetweenPoints( pointCoordinates, previousCoordinates ) );
      if ( distance < minimumDistanceFromPreviousPoint )
      {
        return;
      }
    }
  }

  // add the label to the point
  std::stringstream markupLabel;
  markupLabel << collectPointsNode->GetLabelBase() << collectPointsNode->GetNextLabelNumber();

  // Add point to the markups node
  int pointIndexInMarkups = markupsNode->AddFiducialFromArray( pointCoordinates );
  markupsNode->SetNthFiducialLabel( pointIndexInMarkups, markupLabel.str().c_str() );

  // always increase the label counter
  collectPointsNode->SetNextLabelNumber( collectPointsNode->GetNextLabelNumber() + 1 );
}
