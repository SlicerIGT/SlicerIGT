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
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
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
#include <vtkImplicitPolyDataDistance.h>
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
, DefaultLineToClosestPointTextScale(2.0)
, DefaultLineToClosestPointThickness(3.0)
{
  this->DefaultLineToClosestPointColor[0]=0;
  this->DefaultLineToClosestPointColor[1]=1;
  this->DefaultLineToClosestPointColor[2]=0;
}


//------------------------------------------------------------------------------
vtkSlicerBreachWarningLogic::~vtkSlicerBreachWarningLogic()
{
}

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
  
  vtkSmartPointer< vtkImplicitPolyDataDistance > implicitDistanceFilter = vtkSmartPointer< vtkImplicitPolyDataDistance >::New();

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
    bodyToRasFilter->Update(); // expensive: transforms all the points of the polydata

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

  double closestPointOnModel_Ras[3] = {0};
  double closestPointDistance = implicitDistanceFilter->EvaluateFunctionAndGetClosestPoint( toolTipPosition_Ras, closestPointOnModel_Ras);
  bwNode->SetClosestDistanceToModelFromToolTip(closestPointDistance);
  bwNode->SetClosestPointOnModel(closestPointOnModel_Ras);

  this->UpdateLineToClosestPoint(bwNode, toolTipPosition_Ras, closestPointOnModel_Ras, closestPointDistance);
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

    // Delete the line to closest point ruler
    vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast(node);
    if (moduleNode && moduleNode->GetLineToClosestPointNodeID())
    {
      // Need to get the ID and lookup the ruler node based on that ID,
      // because the node is not in the scene anymore and therefore cannot get pointer to the referenced node.
      vtkMRMLNode* rulerNode = this->GetMRMLScene()->GetNodeByID(moduleNode->GetLineToClosestPointNodeID());
      if (rulerNode)
      {
        this->GetMRMLScene()->RemoveNode(rulerNode);
      }
    }
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

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* vtkNotUsed(callData) )
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
void vtkSlicerBreachWarningLogic::UpdateLineToClosestPoint(vtkMRMLBreachWarningNode* bwNode, double* toolTipPosition_Ras, double* closestPointOnModel_Ras, double closestPointDistance)
{
  if ( bwNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::UpdateLineToClosestPoint failed: invalid bwNode");
    return;
  }
  vtkMRMLAnnotationRulerNode* ruler = bwNode->GetLineToClosestPointNode();
  if (ruler == NULL)
  {
    // line to closest point is not displayed, no need to update
    return;
  }

  ruler->SetPosition1(toolTipPosition_Ras);
  ruler->SetPosition2(closestPointOnModel_Ras);
  if (closestPointDistance<0)
  {    
    ruler->SetName("d (in)");
  }
  else
  {   
    ruler->SetName("d");
  }
}

//------------------------------------------------------------------------------
bool vtkSlicerBreachWarningLogic::GetLineToClosestPointVisibility(vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointColor failed: invalid moduleNode");
    return false;
  }
  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if ( ruler == NULL )
  {
    return false;
  }
  vtkMRMLDisplayNode* displayNode = ruler->GetDisplayNode();
  if (displayNode == NULL)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointVisibility failed: invalid displayNode");
    return false;
  }
  return displayNode->GetVisibility();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointVisibility(bool visible, vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointVisible failed: invalid moduleNode");
    return;
  }
  vtkMRMLScene* scene = this->GetMRMLScene();
  if ( scene == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointVisible failed: invalid scene");
    return;
  }

  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();

  if (ruler == NULL && visible)
  {
    // line does not exist yet, but should be visible - create it
    vtkSmartPointer<vtkMRMLAnnotationRulerNode> newRuler = vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
    newRuler->SetName("d");
    newRuler->Initialize(scene);
    newRuler->SetLocked(true);
    newRuler->SetTextScale(this->DefaultLineToClosestPointTextScale); // RulerTextSize
    double color[3] = {0,1,0}; // green
    newRuler->GetAnnotationLineDisplayNode()->SetColor(color);
    newRuler->GetAnnotationPointDisplayNode()->SetColor(color);
    newRuler->GetAnnotationTextDisplayNode()->SetColor(color);
    newRuler->GetAnnotationPointDisplayNode()->SetGlyphScale(this->DefaultLineToClosestPointThickness);
    newRuler->GetAnnotationLineDisplayNode()->SetLineThickness(this->DefaultLineToClosestPointThickness);
    moduleNode->SetLineToClosestPointNodeID(newRuler->GetID());
  }
  else if (ruler != NULL)
  {
    // line exists, but it is not needed - hide it (don't delete it because it stores thickness, color, etc. parameters)
    ruler->SetDisplayVisibility(visible);
  }
}

//------------------------------------------------------------------------------
double* vtkSlicerBreachWarningLogic::GetLineToClosestPointColor(vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointColor failed: invalid moduleNode");
    return this->DefaultLineToClosestPointColor;
  }
  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if ( ruler == NULL )
  {
    return this->DefaultLineToClosestPointColor;
  }
  vtkMRMLAnnotationLineDisplayNode* displayNode = ruler->GetAnnotationLineDisplayNode();
  if ( displayNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointColor failed: displayNode is invalid")
    return this->DefaultLineToClosestPointColor;
  }
  return displayNode->GetColor();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointColor(double* color, vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointColor failed: invalid moduleNode");
    return;
  }

  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if (ruler == NULL)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointColor failed: invalid ruler");
    return;
  }

  if (ruler->GetAnnotationLineDisplayNode())
  {
    ruler->GetAnnotationLineDisplayNode()->SetColor(color);
  }
  if (ruler->GetAnnotationPointDisplayNode())
  {
    ruler->GetAnnotationPointDisplayNode()->SetColor(color);
  }
  if (ruler->GetAnnotationTextDisplayNode())
  {
    ruler->GetAnnotationTextDisplayNode()->SetColor(color);
  }
}
//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointColor(double r, double g, double b, vtkMRMLBreachWarningNode* moduleNode)
{
  double color[3]={r,g,b};
  this->SetLineToClosestPointColor(color, moduleNode);
}

//------------------------------------------------------------------------------
double vtkSlicerBreachWarningLogic::GetLineToClosestPointTextScale(vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointTextScale failed: invalid moduleNode");
    return this->DefaultLineToClosestPointTextScale;
  }
  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if ( ruler == NULL )
  {
    return this->DefaultLineToClosestPointTextScale;
  }
  return ruler->GetTextScale();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointTextScale(double scale, vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointTextScale failed: invalid moduleNode");
    return;
  }

  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if (ruler == NULL)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointTextScale failed: invalid ruler");
    return;
  }
  
  ruler->SetTextScale(scale);
}

//------------------------------------------------------------------------------
double vtkSlicerBreachWarningLogic::GetLineToClosestPointThickness(vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointColor failed: invalid moduleNode");
    return this->DefaultLineToClosestPointThickness;
  }
  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if ( ruler == NULL )
  {
    return this->DefaultLineToClosestPointThickness;
  }
  vtkMRMLAnnotationLineDisplayNode* displayNode = ruler->GetAnnotationLineDisplayNode();
  if ( displayNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointColor failed: displayNode is invalid")
    return this->DefaultLineToClosestPointThickness;
  }
  return displayNode->GetLineThickness();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointThickness(double thickness, vtkMRMLBreachWarningNode* moduleNode)
{
  if ( moduleNode == NULL )
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointThickness failed: invalid moduleNode");
    return;
  }

  vtkMRMLAnnotationRulerNode* ruler = moduleNode->GetLineToClosestPointNode();
  if (ruler == NULL)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointThickness failed: invalid ruler");
    return;
  }

  if (ruler->GetAnnotationLineDisplayNode())
  {
    ruler->GetAnnotationLineDisplayNode()->SetLineThickness(thickness);
  }
  if (ruler->GetAnnotationPointDisplayNode())
  {
    ruler->GetAnnotationPointDisplayNode()->SetGlyphScale(thickness);
  }
}

