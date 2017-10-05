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

  // see if output exists
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( collectFiducialsNode->GetOutputNode() );
  if ( outputMarkupsNode == NULL )
  {
    vtkErrorMacro( "No output markups node set. Aborting." );
    return;
  }

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

  //TODO: Add ability to change glyph scale when feature is added to Markups module
  //this->MarkupsFiducialNode-> ->SetGlyphScale( glyphScale );
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
