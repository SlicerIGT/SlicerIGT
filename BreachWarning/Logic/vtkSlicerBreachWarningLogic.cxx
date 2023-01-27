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
#include "vtkMRMLBreachWarningNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
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
, DefaultLineToClosestPointTextScale(5.0)
, DefaultLineToClosestPointThickness(1.0)
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

    // Delete the line to closest point line
    vtkMRMLBreachWarningNode* moduleNode = vtkMRMLBreachWarningNode::SafeDownCast(node);
    if (moduleNode && moduleNode->GetLineToClosestPointNodeID())
    {
      // Need to get the ID and lookup the line node based on that ID,
      // because the node is not in the scene anymore and therefore cannot get pointer to the referenced node.
      vtkMRMLNode* lineNode = this->GetMRMLScene()->GetNodeByID(moduleNode->GetLineToClosestPointNodeID());
      if (lineNode)
      {
        this->GetMRMLScene()->RemoveNode(lineNode);
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
  vtkMRMLMarkupsLineNode* line = bwNode->GetLineToClosestPointNode();
  if (line == NULL)
  {
    // line to closest point is not displayed, no need to update
    return;
  }

  if (line->GetNumberOfControlPoints() != 2)
  {
    line->RemoveAllControlPoints();
    line->AddControlPointWorld(vtkVector3d(toolTipPosition_Ras));
    line->AddControlPointWorld(vtkVector3d(closestPointOnModel_Ras));
  }
  else
  {
    line->SetNthControlPointPositionWorld(0, toolTipPosition_Ras);
    line->SetNthControlPointPositionWorld(1, closestPointOnModel_Ras);
  }
  
  if (closestPointDistance<0)
  {    
    line->SetName("d (in)");
  }
  else
  {   
    line->SetName("d");
  }
}

//------------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* vtkSlicerBreachWarningLogic::GetLineDisplayNode(vtkMRMLBreachWarningNode* moduleNode)
{
  if (!moduleNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineDisplayNode failed: invalid moduleNode");
    return nullptr;
  }
  vtkMRMLMarkupsLineNode* line = moduleNode->GetLineToClosestPointNode();
  if (!line)
  {
    return nullptr;
  }
  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(line->GetDisplayNode());
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineDisplayNode failed: invalid displayNode");
  }
  return displayNode;
}

//------------------------------------------------------------------------------
bool vtkSlicerBreachWarningLogic::GetLineToClosestPointVisibility(vtkMRMLBreachWarningNode* moduleNode)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointVisibility failed: invalid displayNode");
    return false;
  }
  return displayNode->GetVisibility();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointVisibility(bool visible, vtkMRMLBreachWarningNode* moduleNode)
{
  if (!moduleNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointVisibility failed: invalid moduleNode");
    return;
  }
  vtkMRMLMarkupsLineNode* line = moduleNode->GetLineToClosestPointNode();
  if (line == NULL && visible)
  {
    // line does not exist yet, but should be visible - create it
    vtkMRMLScene* scene = this->GetMRMLScene();
    if (!scene)
    {
      vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointVisibility failed: MRML scene not yet created");
      return;
    }
    vtkMRMLMarkupsLineNode* newLine = vtkMRMLMarkupsLineNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsLineNode", "d"));
    if (!newLine)
    {
      vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointVisibility failed: could not create new line node");
      return;
    }
    newLine->SetLocked(true);
    newLine->CreateDefaultDisplayNodes();
    vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(newLine->GetDisplayNode());
    if (!displayNode)
    {
      vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointVisibility failed: could not create line display node");
      return;
    }
    // Specify line size relative to screen size, via setting the line thickness to the
    // same as control point scale.
    displayNode->SetGlyphScale(this->DefaultLineToClosestPointThickness);
    displayNode->SetUseGlyphScale(true);
    displayNode->SetLineThickness(1.0); // Make glyphs invisible to keep display simple
    displayNode->SetCurveLineSizeMode(vtkMRMLMarkupsDisplayNode::UseLineThickness);
    displayNode->SetTextScale(this->DefaultLineToClosestPointTextScale);
    displayNode->GetTextProperty()->ShadowOff(); // to keep display simple
    displayNode->SetSelectedColor(0, 1, 0); // green
    displayNode->SetGlyphScale(this->DefaultLineToClosestPointThickness);
    moduleNode->SetLineToClosestPointNodeID(newLine->GetID());
  }

  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (displayNode)
  {
    displayNode->SetVisibility(visible);
  }
}

//------------------------------------------------------------------------------
double* vtkSlicerBreachWarningLogic::GetLineToClosestPointColor(vtkMRMLBreachWarningNode* moduleNode)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointColor failed: displayNode is invalid");
    return this->DefaultLineToClosestPointColor;
  }
  return displayNode->GetSelectedColor();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointColor(double* color, vtkMRMLBreachWarningNode* moduleNode)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointColor failed: displayNode is invalid");
    return;
  }
  displayNode->SetSelectedColor(color);
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
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointTextScale failed: displayNode is invalid");
    return this->DefaultLineToClosestPointTextScale;
  }
  return displayNode->GetTextScale();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointTextScale(double scale, vtkMRMLBreachWarningNode* moduleNode)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointTextScale failed: displayNode is invalid");
    return;
  }
  displayNode->SetTextScale(scale);
}

//------------------------------------------------------------------------------
double vtkSlicerBreachWarningLogic::GetLineToClosestPointThickness(vtkMRMLBreachWarningNode* moduleNode)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::GetLineToClosestPointThickness failed: displayNode is invalid");
    return this->DefaultLineToClosestPointThickness;
  }
  return displayNode->GetGlyphScale();
}

//------------------------------------------------------------------------------
void vtkSlicerBreachWarningLogic::SetLineToClosestPointThickness(double thickness, vtkMRMLBreachWarningNode* moduleNode)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetLineDisplayNode(moduleNode);
  if (!displayNode)
  {
    vtkErrorMacro("vtkSlicerBreachWarningLogic::SetLineToClosestPointThickness failed: displayNode is invalid");
    return;
  }
  return displayNode->SetGlyphScale(thickness);
}
