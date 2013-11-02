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

// .NAME vtkSlicerCreateModelsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerCreateModelsLogic_h
#define __vtkSlicerCreateModelsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerCreateModelsModuleLogicExport.h"

#include "vtkPolyData.h"
#include "vtkSmartPointer.h"



/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_CREATEMODELS_MODULE_LOGIC_EXPORT vtkSlicerCreateModelsLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerCreateModelsLogic *New();
  vtkTypeMacro(vtkSlicerCreateModelsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void CreateNeedle( double length, double radius, double tipRadius );
  void CreateCube( double x, double y, double z );
  void CreateCylinder( double h, double r );
  void CreateSphere( double radius );
  
  
protected:
  vtkSlicerCreateModelsLogic();
  virtual ~vtkSlicerCreateModelsLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:
  
  void CreateCylinderData( vtkPolyData* polyData, double height, double radius );
  void CreateConeData( vtkPolyData* polyData, double height, double radius );
  
  vtkSlicerCreateModelsLogic(const vtkSlicerCreateModelsLogic&); // Not implemented
  void operator=(const vtkSlicerCreateModelsLogic&);             // Not implemented
  
};

#endif
