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

// .NAME vtkSlicerCreateModelsLogic - slicer logic class for creating simple models
// .SECTION Description
// This class contains methods that create model nodes useful for setting up IGT scenes.
// These methods can be called from other modules.


#ifndef __vtkSlicerCreateModelsLogic_h
#define __vtkSlicerCreateModelsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerCreateModelsModuleLogicExport.h"

class vtkMRMLModelNode;
class vtkPolyData;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_CREATEMODELS_MODULE_LOGIC_EXPORT vtkSlicerCreateModelsLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerCreateModelsLogic *New();
  vtkTypeMacro(vtkSlicerCreateModelsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkMRMLModelNode* CreateNeedle( double length, double radius, double tipRadius, bool markers );
  vtkMRMLModelNode* CreateCube( double x, double y, double z );
  vtkMRMLModelNode* CreateCylinder( double height, double radius );
  vtkMRMLModelNode* CreateSphere( double radius );
  vtkMRMLModelNode* CreateCoordinate( double axisLength, double axisRadius );  

protected:
  vtkSlicerCreateModelsLogic();
  virtual ~vtkSlicerCreateModelsLogic();

private:

  void CreateCylinderData( vtkPolyData* polyData, double height, double radius );
  void CreateConeData( vtkPolyData* polyData, double height, double radius );

  vtkSlicerCreateModelsLogic(const vtkSlicerCreateModelsLogic&); // Not implemented
  void operator=(const vtkSlicerCreateModelsLogic&);             // Not implemented

};

#endif
