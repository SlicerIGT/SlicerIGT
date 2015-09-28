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

// BreachWarning includes
#include "vtkSlicerBreachWarningLogic.h"

// MRML includes
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLBreachWarningNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkGeneralTransform.h>
#include <vtkGenericCell.h>
#include <vtkImplicitPolyDataDistancePointPos.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes

// Slicer methods 

vtkStandardNewMacro(vtkSlicerBreachWarningLogic);

//------------------------------------------------------------------------------
vtkSlicerBreachWarningLogic::vtkSlicerBreachWarningLogic()
: WarningSoundPlaying(false)
, TrajectoryInitialized(false)
{}


//------------------------------------------------------------------------------
vtkSlicerBreachWarningLogic::~vtkSlicerBreachWarningLogic()
{}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    vtkWarningMacro( "MRML scene not yet created" );
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLBreachWarningNode >::New() );
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::UpdateToolState( vtkMRMLBreachWarningNode* bwNode )
{
  if ( bwNode == NULL )
  {
    return;
  }

  vtkMRMLModelNode* modelNode = bwNode->GetWatchedModelNode();
  vtkMRMLTransformNode* toolToRasNode = bwNode->GetToolTransformNode();

  if ( modelNode == NULL || toolToRasNode == NULL )
  {
    bwNode->SetClosestDistanceToModelFromToolTip(0);
    return;
  }

  vtkPolyData* body = modelNode->GetPolyData();
  if ( body == NULL )
  {
    vtkWarningMacro( "No surface model in node" );
    return;
  }
  
  vtkSmartPointer< vtkImplicitPolyDataDistancePointPos > implicitDistanceFilter = vtkSmartPointer< vtkImplicitPolyDataDistancePointPos >::New();

  // Transform the body poly data if there is a parent transform.
  vtkMRMLTransformNode* bodyParentTransform = modelNode->GetParentTransformNode();
  if ( bodyParentTransform != NULL )
  {
    vtkSmartPointer< vtkGeneralTransform > bodyToRasTransform = vtkSmartPointer< vtkGeneralTransform >::New();
    bodyParentTransform->GetTransformToWorld( bodyToRasTransform );

    vtkSmartPointer< vtkTransformPolyDataFilter > bodyToRasFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
#if (VTK_MAJOR_VERSION <= 5)
    bodyToRasFilter->SetInput( body );
#else
    bodyToRasFilter->SetInputData( body );
#endif
    bodyToRasFilter->SetTransform( bodyToRasTransform );
    bodyToRasFilter->Update();

    implicitDistanceFilter->SetInput( bodyToRasFilter->GetOutput() ); // expensive: builds a locator
  }
  else
  {
    implicitDistanceFilter->SetInput( body ); // expensive: builds a locator
  }
  
  // Note: Performance could be improved by
  // - keeping the filter in memory for all the observed nodes.
  // - in case of linear transform of model and tooltip: transform only the tooltip (with the tooltip to model transform),
  //   and not transform the model at all

  vtkSmartPointer<vtkGeneralTransform> toolToRasTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  toolToRasNode->GetTransformToWorld( toolToRasTransform ); 
  double toolTipPosition_Tool[4] = { 0.0, 0.0, 0.0, 1.0 };
  double* toolTipPosition_Ras = toolToRasTransform->TransformDoublePoint( toolTipPosition_Tool);

  double cp[3] = {0, 0, 0};
  bwNode->SetClosestDistanceToModelFromToolTip(implicitDistanceFilter->EvaluateFunctionAndGetClosestPoint( toolTipPosition_Ras, cp ));
  bwNode->SetPointOnModel(cp);
  if (bwNode->GetDisplayDistance() || bwNode->GetDisplayTrajectory())
  {
    this->UpdateTrajectory(bwNode, toolTipPosition_Ras);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::UpdateModelColor( vtkMRMLBreachWarningNode* bwNode )
{
  if ( bwNode == NULL )
  {
    return;
  }
  vtkMRMLModelNode* modelNode = bwNode->GetWatchedModelNode();
  if ( modelNode == NULL )
  {
    return;
  }
  if ( modelNode->GetDisplayNode() == NULL )
  {
    return;
  }

  if ( bwNode->IsToolTipInsideModel())
  {
    double* color = bwNode->GetWarningColor();
    modelNode->GetDisplayNode()->SetColor(color);
  }
  else
  {
    double* color = bwNode->GetOriginalColor();
    modelNode->GetDisplayNode()->SetColor(color);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::UpdateDistanceSign( vtkMRMLBreachWarningNode* bwNode )
{
  if ( bwNode == NULL )
  {
    return;
  }
  vtkMRMLAnnotationRulerNode* trajectory = bwNode->GetTrajectory(); 
  if ( trajectory == NULL )
  {
    return;
  }
  if ( trajectory->GetDisplayNode() == NULL )
  {
    return;
  }

  if ( bwNode->IsToolTipInsideModel())
  {    
    //trajectory->SetDistanceAnnotationFormat("-%.0f mm");
    trajectory->SetTextScale(0); // Hidden when inside of model
  }
  else
  {   
    //trajectory->SetDistanceAnnotationFormat("%.0f mm");
    trajectory->SetTextScale(4);
  }
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast(node);
  if ( bwNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( bwNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLBreachWarningNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( bwNode, events.GetPointer() );
    if(bwNode->GetPlayWarningSound() && bwNode->IsToolTipInsideModel())
    {
      // Add to list of playing nodes (if not there already)
      std::deque< vtkWeakPointer< vtkMRMLBreachWarningNode > >::iterator foundPlayingNodeIt = this->WarningSoundPlayingNodes.begin();    
      for (; foundPlayingNodeIt!=this->WarningSoundPlayingNodes.end(); ++foundPlayingNodeIt)
      {
        if (foundPlayingNodeIt->GetPointer()==bwNode)
        {
          // found, current bw node is already in the playing list
          break;
        }
      }
      if (foundPlayingNodeIt==this->WarningSoundPlayingNodes.end())
      {
        this->WarningSoundPlayingNodes.push_back(bwNode);
      }
      this->SetWarningSoundPlaying(true);
    }
  }
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLBreachWarningNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
    for (std::deque< vtkWeakPointer< vtkMRMLBreachWarningNode > >::iterator it=this->WarningSoundPlayingNodes.begin(); it!=this->WarningSoundPlayingNodes.end(); ++it)
    {
      if (it->GetPointer()==node)
      {
        this->WarningSoundPlayingNodes.erase(it);
        break;
      }
    }
    this->SetWarningSoundPlaying(!this->WarningSoundPlayingNodes.empty());
  }
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetWatchedModelNode( vtkMRMLModelNode* newModel, vtkMRMLBreachWarningNode* moduleNode )
{
  if ( moduleNode == NULL )
  {
    vtkWarningMacro( "SetWatchedModelNode: Module node is invalid" );
    return;
  }

  // Get the original color of the old model node
  vtkMRMLModelNode* previousModel=moduleNode->GetWatchedModelNode();

  if (previousModel==newModel)
  {
    // no change
    return;
  }

  double previousOriginalColor[3]={0.5,0.5,0.5};
  if(previousModel)
  {
    moduleNode->GetOriginalColor(previousOriginalColor);
  }

  // Save the original color of the new model node
  if(newModel!=NULL)
  {
    double originalColor[3]={0.5,0.5,0.5};
    if ( newModel->GetDisplayNode() != NULL )
    {
      newModel->GetDisplayNode()->GetColor(originalColor);
    }
    moduleNode->SetOriginalColor(originalColor);
  }

  // Switch to the new model node
  moduleNode->SetAndObserveWatchedModelNodeID( (newModel!=NULL) ? newModel->GetID() : NULL );

  // Restore the color of the old model node
  if(previousModel!=NULL && previousModel->GetDisplayNode()!=NULL)
  {
    previousModel->GetDisplayNode()->SetColor(previousOriginalColor[0],previousOriginalColor[1],previousOriginalColor[2]);
  }
}

/*
//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if (callerNode == NULL)
  {
    return;
  }
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( callerNode );
  if (bwNode == NULL)
  {
    return;
  }
  
  if (event==vtkMRMLBreachWarningNode::InputDataModifiedEvent)
  {
    // only recompute output if the input is changed
    // (for example we do not recompute the distance if the computed distance is changed)
    this->UpdateToolState(bwNode);
    if(bwNode->GetDisplayWarningColor())
    {
      this->UpdateModelColor(bwNode);
    }
    if(bwNode->GetPlayWarningSound() && bwNode->IsToolTipInsideModel())
    {
      this->SetWarningSoundPlaying(true);
    }
    else
    {
      this->SetWarningSoundPlaying(false);
    }
  }
}
*/

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if (callerNode == NULL)
  {
    return;
  }
  
  vtkMRMLBreachWarningNode* bwNode = vtkMRMLBreachWarningNode::SafeDownCast( callerNode );
  if (bwNode == NULL)
  {
    return;
  }
  
  if (event==vtkMRMLBreachWarningNode::InputDataModifiedEvent)
  {
    // only recompute output if the input is changed
    // (for example we do not recompute the distance if the computed distance is changed)
    this->UpdateToolState(bwNode);    
    if (bwNode->GetDisplayWarningColor())
    {
      this->UpdateModelColor(bwNode);
    }
    if (bwNode->GetDisplayDistance())
    {
      this->UpdateDistanceSign(bwNode);
    }
    std::deque< vtkWeakPointer< vtkMRMLBreachWarningNode > >::iterator foundPlayingNodeIt = this->WarningSoundPlayingNodes.begin();    
    for (; foundPlayingNodeIt!=this->WarningSoundPlayingNodes.end(); ++foundPlayingNodeIt)
    {
      if (foundPlayingNodeIt->GetPointer()==bwNode)
      {
        // found current bw node is already in the playing list
        break;
      }
    }
    if(bwNode->GetPlayWarningSound() && bwNode->IsToolTipInsideModel())
    {
      // Add to list of playing nodes (if not there already)
      if (foundPlayingNodeIt==this->WarningSoundPlayingNodes.end())
      {
        this->WarningSoundPlayingNodes.push_back(bwNode);
      }
    }
    else
    {
      // Remove from list of playing nodes (if still there)
      if (foundPlayingNodeIt!=this->WarningSoundPlayingNodes.end())
      {
        this->WarningSoundPlayingNodes.erase(foundPlayingNodeIt);
      }
    }
    this->SetWarningSoundPlaying(!this->WarningSoundPlayingNodes.empty());
  }
}


//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::UpdateTrajectory( vtkMRMLBreachWarningNode* bwNode, double* toolTipPosition )
{
  vtkMRMLAnnotationRulerNode* trajectory = bwNode->GetTrajectory();
  if (trajectory)
  {        
    double closestPointOnModelPosition[3];
    bwNode->GetPointOnModel(closestPointOnModelPosition);    

    trajectory->SetPosition1(toolTipPosition);
    trajectory->SetPosition2(closestPointOnModelPosition);  

    if (!this->TrajectoryInitialized)
    {
      trajectory->Initialize(this->GetMRMLScene());
      trajectory->SetLocked(true);
      trajectory->SetTextScale(4);
      trajectory->SetName("d");
      vtkMRMLAnnotationLineDisplayNode* displayNode = vtkMRMLAnnotationLineDisplayNode::SafeDownCast(trajectory->GetModelDisplayNode());
      if (displayNode)
      {                
        double color[3] = {0, 0, 0};
        bwNode->GetTrajectoryColor(color);
        displayNode->SetColor(color);
        displayNode->SetLineThickness(3);
      }  
      this->TrajectoryInitialized = true;
    }
  }  
}

