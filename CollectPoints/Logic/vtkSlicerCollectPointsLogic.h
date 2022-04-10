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

// .NAME vtkSlicerCollectPointsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerCollectPointsLogic_h
#define __vtkSlicerCollectPointsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelNode.h"

// STD includes
#include <string>
#include <cstdlib>

// includes related to CollectPoints
#include "vtkMRMLCollectPointsNode.h"
#include "vtkSlicerCollectPointsModuleLogicExport.h"

/// \ingroup Slicer_QtModules_CollectPoints
class VTK_SLICER_COLLECTPOINTS_MODULE_LOGIC_EXPORT vtkSlicerCollectPointsLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerCollectPointsLogic *New();
  vtkTypeMacro(vtkSlicerCollectPointsLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  
  void AddPoint( vtkMRMLCollectPointsNode* collectPointsNode );
  void RemoveLastPoint( vtkMRMLCollectPointsNode* collectPointsNode );
  void RemoveAllPoints( vtkMRMLCollectPointsNode* collectPointsNode );
  
  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData ) override;

protected:
  vtkSlicerCollectPointsLogic();
  virtual ~vtkSlicerCollectPointsLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes() override;
  virtual void UpdateFromMRMLScene() override;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  // Computes the sampling coordinates in the anchor coordinate system
  // returns true if it was able to compute point coordinates. Returns false otherwise.
  bool ComputePointCoordinates( vtkMRMLCollectPointsNode* collectPointsNode, double outputPointCoordinates[ 3 ] );

  void AddPointToModel( vtkMRMLCollectPointsNode* collectPointsNode,  double pointCoordinates[ 3 ] );
  void RemoveLastPointFromModel( vtkMRMLModelNode* modelNode );
  void UpdateCellsForPolyData( vtkPolyData* polyData );
  void AddPointToMarkups( vtkMRMLCollectPointsNode* collectPointsNode, double pointCoordinates[ 3 ] );
  vtkSlicerCollectPointsLogic(const vtkSlicerCollectPointsLogic&); // Not implemented
  void operator=(const vtkSlicerCollectPointsLogic&);               // Not implemented
  
protected:
  int Counter;
  
};

#endif

