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

//------------------------------------------------------------------------------
class vtkImplicitPolyDataDistanceNew : public vtkImplicitPolyDataDistance
{
public:
  static vtkImplicitPolyDataDistanceNew *New();
  vtkTypeMacro(vtkImplicitPolyDataDistanceNew, vtkImplicitPolyDataDistance);
  void PrintSelf(ostream &os, vtkIndent indent);

protected:
  vtkImplicitPolyDataDistanceNew(){}
  ~vtkImplicitPolyDataDistanceNew(){}

public:
  double EvaluateFunction(double x[3], double p[3])
  {
    double n[3];
    return this->SharedEvaluate(x, n, p); // distance value returned and point on vtkPolyData stored in p (normal not used).
  }

  double SharedEvaluate(double x[3], double n[3], double mp[3])
  {
    double ret = this->NoValue;
    for( int i=0; i < 3; i++ )
      {
      n[i] = this->NoGradient[i];
      }

    // See if data set with polygons has been specified
    if (this->Input == NULL || Input->GetNumberOfCells() == 0)
      {
      vtkErrorMacro(<<"No polygons to evaluate function!");
      return ret;
      }

    double p[3];
    vtkIdType cellId;
    int subId;
    double vlen2;

    vtkDataArray* cnorms = 0;
    if ( this->Input->GetCellData() && this->Input->GetCellData()->GetNormals() )
      {
      cnorms = this->Input->GetCellData()->GetNormals();
      }

    // Get point id of closest point in data set.
    vtkSmartPointer<vtkGenericCell> cell =
      vtkSmartPointer<vtkGenericCell>::New();
    this->Locator->FindClosestPoint(x, p, cell, cellId, subId, vlen2);

    if (cellId != -1)	// point located
      {
      // dist = | point - x |
      ret = sqrt(vlen2);
      // grad = (point - x) / dist
      for (int i = 0; i < 3; i++)
        {
        n[i] = (p[i] - x[i]) / (ret == 0. ? 1. : ret);
        }

      double dist2, weights[3], pcoords[3], awnorm[3] = {0, 0, 0};
      double closestPoint[3];
      cell->EvaluatePosition(p, closestPoint, subId, pcoords, dist2, weights);

      // Get point p on vtkPolyData
      mp[0] = pcoords[0];
      mp[1] = pcoords[1];
      mp[2] = pcoords[2];

      vtkIdList* idList = vtkIdList::New();
      int count = 0;
      for (int i = 0; i < 3; i++)
        {
        count += (fabs(weights[i]) < this->Tolerance ? 1 : 0);
        }
      // Face case - weights contains no 0s
      if ( count == 0 )
        {
        // Compute face normal.
        if ( cnorms )
          {
          cnorms->GetTuple(cellId, awnorm);
          }
        else
          {
          vtkPolygon::ComputeNormal(cell->Points, awnorm);
          }
        }
      // Edge case - weights contain one 0
      else if ( count == 1 )
        {
        // ... edge ... get two adjacent faces, compute average normal
        int a = -1, b = -1;
        for ( int edge = 0; edge < 3; edge++ )
          {
          if ( fabs(weights[edge]) < this->Tolerance )
            {
            a = cell->PointIds->GetId((edge + 1) % 3);
            b = cell->PointIds->GetId((edge + 2) % 3);
            break;
            }
          }

        if ( a == -1 )
          {
          vtkErrorMacro( << "Could not find edge when closest point is "
                         << "expected to be on an edge." );
          return this->NoValue;
          }

        // The first argument is the cell ID. We pass a bogus cell ID so that
        // all face IDs attached to the edge are returned in the idList.
        this->Input->GetCellEdgeNeighbors(VTK_ID_MAX, a, b, idList);
        for (int i = 0; i < idList->GetNumberOfIds(); i++)
          {
          double norm[3];
          if (cnorms)
            {
            cnorms->GetTuple(idList->GetId(i), norm);
            }
          else
            {
            vtkPolygon::ComputeNormal(this->Input->GetCell(idList->GetId(i))->GetPoints(), norm);
            }
          awnorm[0] += norm[0];
          awnorm[1] += norm[1];
          awnorm[2] += norm[2];
          }
        vtkMath::Normalize(awnorm);
        }

      // Vertex case - weights contain two 0s
      else if ( count == 2 )
        {
        // ... vertex ... this is the expensive case, get all adjacent
        // faces and compute sum(a_i * n_i) Angle-Weighted Pseudo
        // Normals, J. Andreas Baerentzen and Henrik Aanaes
        int a = -1;
        for (int i = 0; i < 3; i++)
          {
          if ( fabs( weights[i] ) > this->Tolerance )
            {
            a = cell->PointIds->GetId(i);
            }
          }

        if ( a == -1 )
          {
          vtkErrorMacro( << "Could not find point when closest point is "
                         << "expected to be a point." );
          return this->NoValue;
          }

        this->Input->GetPointCells(a, idList);
        for (int i = 0; i < idList->GetNumberOfIds(); i++)
          {
          double norm[3];
          if ( cnorms )
            {
            cnorms->GetTuple(idList->GetId(i), norm);
            }
          else
            {
            vtkPolygon::ComputeNormal(this->Input->GetCell(idList->GetId(i))->GetPoints(), norm);
            }

          // Compute angle at point a
          int b = this->Input->GetCell(idList->GetId(i))->GetPointId(0);
          int c = this->Input->GetCell(idList->GetId(i))->GetPointId(1);
          if (a == b)
            {
            b = this->Input->GetCell(idList->GetId(i))->GetPointId(2);
            }
          else if (a == c)
            {
            c = this->Input->GetCell(idList->GetId(i))->GetPointId(2);
            }
          double pa[3], pb[3], pc[3];
          this->Input->GetPoint(a, pa);
          this->Input->GetPoint(b, pb);
          this->Input->GetPoint(c, pc);
          for (int j = 0; j < 3; j++) { pb[j] -= pa[j]; pc[j] -= pa[j]; }
          vtkMath::Normalize(pb);
          vtkMath::Normalize(pc);
          double alpha = acos(vtkMath::Dot(pb, pc));
          awnorm[0] += alpha * norm[0];
          awnorm[1] += alpha * norm[1];
          awnorm[2] += alpha * norm[2];
          }
        vtkMath::Normalize(awnorm);
        }
      idList->Delete();

      // sign(dist) = dot(grad, cell normal)
      if (ret == 0)
        {
        for (int i = 0; i < 3; i++)
          {
          n[i] = awnorm[i];
          }
        }
      ret *= (vtkMath::Dot(n, awnorm) < 0.) ? 1. : -1.;

      if (ret > 0.)
        {
        for (int i = 0; i < 3; i++)
          {
          n[i] = -n[i];
          }
        }
      }

    return ret;
  }
};

vtkStandardNewMacro(vtkImplicitPolyDataDistanceNew);
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
  
  vtkSmartPointer< vtkImplicitPolyDataDistanceNew > implicitDistanceFilter = vtkSmartPointer< vtkImplicitPolyDataDistanceNew >::New();

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

  double p[3] = {0, 0, 0};
  bwNode->SetClosestDistanceToModelFromToolTip(implicitDistanceFilter->EvaluateFunction( toolTipPosition_Ras, p ));
  bwNode->SetPointOnModel(p);
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
    if(bwNode->GetDisplayWarningColor())
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
void vtkSlicerBreachWarningLogic::UpdateTrajectory( vtkMRMLBreachWarningNode* bwNode )
{
  vtkMRMLTransformNode* toolToRasNode = bwNode->GetToolTransformNode();
  vtkSmartPointer<vtkGeneralTransform> toolToRasTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  toolToRasNode->GetTransformToWorld( toolToRasTransform ); 
  double toolTipPosition_Tool[4] = { 0.0, 0.0, 0.0, 1.0 };
  double* toolTipPosition_Ras = toolToRasTransform->TransformDoublePoint( toolTipPosition_Tool);

  double tipPosition[3];
  tipPosition[0] = toolTipPosition_Ras[0];
  tipPosition[1] = toolTipPosition_Ras[1];
  tipPosition[2] = toolTipPosition_Ras[2];
  
  double modelPosition[3];
  bwNode->GetPointOnModel(modelPosition);

  vtkMRMLAnnotationRulerNode* trajectory = bwNode->GetTrajectory();
  trajectory->SetPosition1(tipPosition);
  trajectory->SetPosition2(modelPosition);

  if (!this->TrajectoryInitialized)
  {
    trajectory->Initialize(this->GetMRMLScene());
    this->TrajectoryInitialized = true;
    // Add to scene?
  }
}

