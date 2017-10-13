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
    vtkErrorMacro( "No parameter node set. Aborting." );
    return;
  }

  // find the point coordinates
  vtkSmartPointer< vtkMRMLLinearTransformNode > probeNode = vtkMRMLLinearTransformNode::SafeDownCast( collectFiducialsNode->GetProbeTransformNode() );
  if ( probeNode == NULL )
  {
    vtkErrorMacro( "No probe transform node set. Aborting." );
    return;
  }
  vtkSmartPointer< vtkMatrix4x4 > probeToWorld = vtkSmartPointer< vtkMatrix4x4 >::New();
  probeNode->GetMatrixTransformToWorld( probeToWorld );
  double pointCoordinates[ 3 ] = { probeToWorld->GetElement( 0, 3 ), probeToWorld->GetElement( 1, 3 ), probeToWorld->GetElement( 2, 3 ) };

  vtkMRMLNode* outputNode = collectFiducialsNode->GetOutputNode();
  if ( outputNode == NULL )
  {
    vtkErrorMacro( "No output node set. Aborting." );
    return;
  }

  // try downcasting the output node to different types.
  // If successfully downcasts, then call the relevant function to add the point.
  // If it does not downcast, output an error.
  if ( vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode ) )
  {
    vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( outputNode );
    this->AddPointToMarkups( collectFiducialsNode, outputMarkupsNode, pointCoordinates );
  }
  else if ( vtkMRMLModelNode::SafeDownCast( outputNode ) )
  {
    vtkMRMLModelNode* outputModelNode = vtkMRMLModelNode::SafeDownCast( outputNode );
    this->AddPointToModel( collectFiducialsNode, outputModelNode, pointCoordinates );
  }
  else
  {
    vtkErrorMacro( "Could not recognize the type of output node. Aborting." );
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
      this->AddPoint( collectFiducialsNode ); // Will create modified event to update widget
    }
  }
}

//------------------------------------------------------------------------------
// note: point coordinates MUST contain 3 values, neither input MRML node can be null
void vtkSlicerCollectFiducialsLogic::AddPointToModel( vtkMRMLCollectFiducialsNode* collectFiducialsNode, vtkMRMLModelNode* outputModelNode, double* pointCoordinates )
{
  vtkSmartPointer< vtkPolyData > outputPolyData = outputModelNode->GetPolyData();
  if ( outputPolyData == NULL )
  {
    outputPolyData = vtkSmartPointer< vtkPolyData >::New();
    outputModelNode->SetAndObservePolyData( outputPolyData );
  }

  vtkSmartPointer< vtkPoints > outputPoints = outputPolyData->GetPoints();
  if ( outputPoints == NULL )
  {
    outputPoints = vtkSmartPointer< vtkPoints >::New();
    outputPolyData->SetPoints( outputPoints );
  }

  int numberOfPoints = ( int )outputPoints->GetNumberOfPoints();
  if ( collectFiducialsNode->GetCollectMode() == vtkMRMLCollectFiducialsNode::Automatic && numberOfPoints > 0 )
  {
    double previousCoordinates[ 3 ];
    outputPoints->GetPoint( numberOfPoints - 1, previousCoordinates );
    double distanceMm = sqrt( vtkMath::Distance2BetweenPoints( pointCoordinates, previousCoordinates ) );
    if ( distanceMm < collectFiducialsNode->GetMinimumDistanceMm() )
    {
      return;
    }
  }

  outputPoints->InsertNextPoint( pointCoordinates );
  outputPolyData->Modified();
  outputModelNode->Modified();
}

//------------------------------------------------------------------------------
// note: point coordinates MUST contain 3 values, neither input MRML node can be null
void vtkSlicerCollectFiducialsLogic::AddPointToMarkups( vtkMRMLCollectFiducialsNode* collectFiducialsNode, vtkMRMLMarkupsFiducialNode* outputMarkupsNode, double* pointCoordinates )
{
  // if in automatic collection mode, make sure sufficient there is sufficient distance from previous point
  int numberOfPoints = outputMarkupsNode->GetNumberOfFiducials();
  if ( collectFiducialsNode->GetCollectMode() == vtkMRMLCollectFiducialsNode::Automatic && numberOfPoints > 0 )
  {
    double previousCoordinates[ 3 ];
    outputMarkupsNode->GetNthFiducialPosition( numberOfPoints - 1, previousCoordinates );
    double distanceMm = sqrt( vtkMath::Distance2BetweenPoints( pointCoordinates, previousCoordinates ) );
    if ( distanceMm < collectFiducialsNode->GetMinimumDistanceMm() )
    {
      return;
    }
  }

  // Add point to the markups node
  int pointIndexInMarkups = outputMarkupsNode->AddFiducialFromArray( pointCoordinates );

  // add the label to the point
  std::stringstream ss;
  ss << collectFiducialsNode->GetLabelBase() << collectFiducialsNode->GetLabelCounter();
  outputMarkupsNode->SetNthFiducialLabel( pointIndexInMarkups, ss.str().c_str() );

  // always increase the label counter automatically
  collectFiducialsNode->SetLabelCounter( collectFiducialsNode->GetLabelCounter() + 1 );
}
