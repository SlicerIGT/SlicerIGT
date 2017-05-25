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

// vtk includes
#include <vtkDoubleArray.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerMarkupsToModelModuleLogicExport.h"

class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsToModelNode;
class vtkPolyData;

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
  void UpdateOutputModel( vtkMRMLMarkupsToModelNode* moduleNode );
  // Generates the closed surface from the markups using vtkDelaunay3D. Uses Delanauy alpha value, subdivision filter and clean markups
  // options from the module node.
  void UpdateOutputCloseSurfaceModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* outputPolyData );
  // Generates the curve model from the markups connecting consecutive segments.
  // Each segment can be linear, cardinal or Kochanek Splines (described and implemented in UpdateOutputCurveModel, UpdateOutputLinearModel
  // and UpdateOutputHermiteSplineModel methods). Uses Tube radius and clean markups option from the module node.
  void UpdateOutputCurveModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPolyData* outputPolyData );
  // Generates the linear curve model connecting linear tubes from each markup.
  void UpdateOutputLinearModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkPolyData* outputPolyData );
  // Generates Cardinal Spline curve model.
  void UpdateOutputCardinalSplineModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkPolyData* outputPolyData );
  // Generates Kochanek Spline curve model.
  void UpdateOutputKochanekSplineModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkPolyData* outputPolyData );
  // Generates a polynomial curve model.
  void UpdateOutputPolynomialFitModel( vtkMRMLMarkupsToModelNode* markupsToModelModuleNode, vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters, vtkPolyData* outputPolyData );
  // Assign parameter values to points based on their position in the markups list (good for ordered point sets)
  void ComputePointParametersRawIndices( vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters );
  // Assign parameter values to points based on their position in a minimum spanning tree between the two farthest points (good for unordered point sets)
  void ComputePointParametersMinimumSpanningTree( vtkPoints* controlPoints, vtkDoubleArray* markupsPointsParameters );

  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );

  static std::string GetMarkupsNodeName(vtkMRMLMarkupsToModelNode* );
  static std::string GetModelNodeName(vtkMRMLMarkupsToModelNode* );
  static std::string GetMarkupsDisplayNodeName(vtkMRMLMarkupsToModelNode* );
  static std::string GetModelDisplayNodeName(vtkMRMLMarkupsToModelNode* );

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

  enum PointArrangement
  {
    POINT_ARRANGEMENT_SINGULAR = 0,
    POINT_ARRANGEMENT_LINEAR,
    POINT_ARRANGEMENT_PLANAR,
    POINT_ARRANGEMENT_NONPLANAR,
    POINT_ARRANGEMENT_LAST // do not set to this type, insert valid types above this line
  };
  
  // Compute the best fit plane through the points, as well as the major and minor axes which describe variation in points.
  void ComputeTransformFromBoundingAxes( vtkPoints* points, vtkMatrix4x4* transformFromBoundingAxes );

  // Compute the range of points along the specified axes (total lengths along which points appear)
  void ComputeTransformedExtentRanges( vtkPoints* points, vtkMatrix4x4* transformMatrix, double outputExtentRanges[ 3 ] );

  // Compute the amount to extrude surfaces when closed surface is linear or planar.
  double ComputeSurfaceExtrusionAmount( const double extents[ 3 ] );

  // Find out what kind of arrangment the points are in (see PointArrangementEnum above).
  // If the arrangement is planar, stores the normal of the best fit plane in planeNormal.
  // If the arrangement is linear, stores the axis of the best fit line in lineAxis.
  PointArrangement ComputePointArrangement( const double smallestBoundingExtentRanges[ 3 ] );

  // helper utility functions
  void SetNthAxisInMatrix( vtkMatrix4x4* matrix, int n, const double axis[ 3 ] );
  void GetNthAxisInMatrix( vtkMatrix4x4* matrix, int n, double outputAxis[ 3 ] );
};

#endif
