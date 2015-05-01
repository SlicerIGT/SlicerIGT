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

// .NAME vtkSlicerMarkupsToModelLogic - slicer logic class to convert markups to models
// .SECTION Description
// This class manages the logic associated with selecting, adding and removing markups and converting these markups to either a:
// a) closed surface using vtkDelaunay3D triangulation
// b) piece wise connected curve. The curve can be linear, Cardinal or Kochanek Splines


#ifndef __vtkSlicerMarkupsToModelLogic_h
#define __vtkSlicerMarkupsToModelLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerMarkupsLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerMarkupsToModelModuleLogicExport.h"

class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsToModelNode;
class vtkPolyData;

static const int MINIMUM_MARKUPS_NUMBER = 3;
static const int MINIMUM_MARKUPS_CLOSED_SURFACE_NUMBER = 4;
static const double CLEAN_POLYDATA_TOLERANCE=0.01;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_MARKUPSTOMODEL_MODULE_LOGIC_EXPORT vtkSlicerMarkupsToModelLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerMarkupsToModelLogic *New();
  vtkTypeMacro(vtkSlicerMarkupsToModelLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Sets the markups node to be transformed
  void SetMarkupsNode( vtkMRMLMarkupsFiducialNode* newMarkups, vtkMRMLMarkupsToModelNode* moduleNode );

  vtkSlicerMarkupsLogic* MarkupsLogic;
  // Updates the mouse selection type to create markups or to navigate the scene.
  void UpdateSelectionNode( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode );
  // Updates closed surface or curve output model from markups
  void UpdateOutputModel(vtkMRMLMarkupsToModelNode* moduleNode);
  // Generates the closed surface from the markups using vtkDelaunay3D. Uses Delanauy alpha value, subdivision filter and clean markups 
  // options from the module node.
  void UpdateOutputCloseSurfaceModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode);
  // Generates the curve model from the markups connecting consecutive segments. 
  // Each segment can be linear, cardinal or Kochanek Splines (described and implemented in UpdateOutputCurveModel, UpdateOutputLinearModel 
  // and UpdateOutputHermiteSplineModel methods). Uses Tube radius and clean markups option from the module node.
  void UpdateOutputCurveModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode);
  // Generates the linear curve model connecting linear tubes from each markup.
  void UpdateOutputLinearModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData);
  // Generates cardinal or Kochanek Spline curve model. If the Kochanek Spline the bias, continuity and tension parameters from de module node 
  // are used.
  void UpdateOutputHermiteSplineModel(vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData * markupsPointsPolyData);

  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );

protected:
  vtkSlicerMarkupsToModelLogic();
  virtual ~vtkSlicerMarkupsToModelLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  ///When a scene has been imported it will set the markups list and model.
  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneStartImport();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:

  vtkSlicerMarkupsToModelLogic(const vtkSlicerMarkupsToModelLogic&); // Not implemented
  void operator=(const vtkSlicerMarkupsToModelLogic&); // Not implemented
  int ImportingScene;
};

#endif
