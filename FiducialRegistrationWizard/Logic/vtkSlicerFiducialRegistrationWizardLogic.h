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

// .NAME vtkSlicerFiducialRegistrationWizardLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerFiducialRegistrationWizardLogic_h
#define __vtkSlicerFiducialRegistrationWizardLogic_h


#include <string>

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerMarkupsLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>
#include "vtkSmartPointer.h"
#include "vtkMRMLFiducialRegistrationWizardNode.h"

class vtkMRMLMarkupsFiducialNode;
class vtkMRMLLinearTransformNode;


// STD includes
#include <cstdlib>

#include "vtkSlicerFiducialRegistrationWizardModuleLogicExport.h"




/// \ingroup Slicer_QtModules_FiducialRegistrationWizard
class VTK_SLICER_FIDUCIALREGISTRATIONWIZARD_MODULE_LOGIC_EXPORT vtkSlicerFiducialRegistrationWizardLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerFiducialRegistrationWizardLogic *New();
  vtkTypeMacro(vtkSlicerFiducialRegistrationWizardLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void AddFiducial( vtkMRMLLinearTransformNode* probeTransformNode );
  std::string CalculateTransform( vtkMRMLMarkupsFiducialNode* fromFiducials, vtkMRMLMarkupsFiducialNode* toFiducials, vtkMRMLLinearTransformNode* outputTransform, std::string transformType );

  vtkMRMLNode* GetFiducialRegistrationWizardNode();

  vtkSlicerMarkupsLogic* MarkupsLogic;
  
  
protected:
  vtkSlicerFiducialRegistrationWizardLogic();
  virtual ~vtkSlicerFiducialRegistrationWizardLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:
  vtkSlicerFiducialRegistrationWizardLogic(const vtkSlicerFiducialRegistrationWizardLogic&); // Not implemented
  void operator=(const vtkSlicerFiducialRegistrationWizardLogic&);               // Not implemented
    
  double CalculateRegistrationError( vtkPoints* fromPoints, vtkPoints* toPoints, vtkLinearTransform* transform );
  bool CheckCollinear( vtkPoints* points );
};

#endif

