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

// .NAME vtkSlicerBreachWarningLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic of the Breach Warning module.


#ifndef __vtkSlicerBreachWarningLogic_h
#define __vtkSlicerBreachWarningLogic_h


#include <string>
#include <deque>

// VTK includes
#include "vtkWeakPointer.h"

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>
#include "vtkSmartPointer.h"

// For referencing own MRML node
class vtkMRMLBreachWarningNode;

class vtkMRMLModelNode;
class vtkMRMLTransformNode;

// STD includes
#include <cstdlib>

#include "vtkSlicerBreachWarningModuleLogicExport.h"

/// \ingroup Slicer_QtModules_BreachWarning
class VTK_SLICER_BREACHWARNING_MODULE_LOGIC_EXPORT vtkSlicerBreachWarningLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerBreachWarningLogic *New();
  vtkTypeMacro(vtkSlicerBreachWarningLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Changes the watched model node, making sure the original color of the previously selected model node is restored
  void SetWatchedModelNode( vtkMRMLModelNode* newModel, vtkMRMLBreachWarningNode* moduleNode );

  /// Show a line from the tooltip to the closest point on the model. Creates/deletes a ruler node.
  void SetLineToClosestPointVisibility(bool visible, vtkMRMLBreachWarningNode* moduleNode);
  bool GetLineToClosestPointVisibility(vtkMRMLBreachWarningNode* moduleNode);

  /// Get color of the line from the tooltip to the closest point on the model. Returns default color if the line is not shown.
  double* GetLineToClosestPointColor(vtkMRMLBreachWarningNode* moduleNode);  
  void SetLineToClosestPointColor(double* color, vtkMRMLBreachWarningNode* moduleNode);  
  void SetLineToClosestPointColor(double r, double g, double b, vtkMRMLBreachWarningNode* moduleNode);  

  /// Get text size of the line from the tooltip to the closest point on the model.
  double GetLineToClosestPointTextScale(vtkMRMLBreachWarningNode* moduleNode);
  void SetLineToClosestPointTextScale(double scale, vtkMRMLBreachWarningNode* moduleNode);

  /// Get thickness of the line from the tooltip to the closest point on the model.
  double GetLineToClosestPointThickness(vtkMRMLBreachWarningNode* moduleNode);
  void SetLineToClosestPointThickness(double thickness, vtkMRMLBreachWarningNode* moduleNode);

  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );

  /// Returns true if a warning sound has to be played
  vtkGetMacro(WarningSoundPlaying, bool);
  vtkSetMacro(WarningSoundPlaying, bool);

protected:
  vtkSlicerBreachWarningLogic();
  virtual ~vtkSlicerBreachWarningLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  void UpdateToolState( vtkMRMLBreachWarningNode* bwNode );
  void UpdateModelColor( vtkMRMLBreachWarningNode* bwNode );
  void UpdateLineToClosestPoint(vtkMRMLBreachWarningNode* bwNode, double* toolTipPosition_Ras, double* closestPointOnModel_Ras, double closestPointDistance);
  
private:
  vtkSlicerBreachWarningLogic(const vtkSlicerBreachWarningLogic&); // Not implemented
  void operator=(const vtkSlicerBreachWarningLogic&);               // Not implemented

  void UpdateRuler(vtkMRMLBreachWarningNode* bwNode, double* toolTipPosition);

  std::deque< vtkWeakPointer< vtkMRMLBreachWarningNode > > WarningSoundPlayingNodes;
  bool WarningSoundPlaying;
  
  double DefaultLineToClosestPointColor[3];
  double DefaultLineToClosestPointTextScale;
  double DefaultLineToClosestPointThickness;
};

#endif
