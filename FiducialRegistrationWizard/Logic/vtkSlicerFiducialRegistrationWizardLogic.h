/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

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
  void AddFiducial( vtkMRMLLinearTransformNode* probeTransformNode, vtkMRMLMarkupsFiducialNode* fiducialNode );

  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );
  //void ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData );

  void UpdateCalibration( vtkMRMLNode* node );

  vtkGetMacro(MarkupsLogic, vtkSlicerMarkupsLogic*);
  vtkSetMacro(MarkupsLogic, vtkSlicerMarkupsLogic*);
  
  std::string GetOutputMessage( std::string nodeID );
  
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

  std::map< std::string, std::string > OutputMessages;

  void SetOutputMessage( std::string nodeID, std::string newOutputMessage ); // The modified event will tell the widget to update (only needs to update when transform is calculated)

  vtkSlicerMarkupsLogic* MarkupsLogic;
};

#endif

