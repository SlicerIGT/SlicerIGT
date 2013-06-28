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

// .NAME vtkSlicerIGTLRemoteLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerIGTLRemoteLogic_h
#define __vtkSlicerIGTLRemoteLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerIGTLRemoteModuleLogicExport.h"


/// \ingroup Slicer_QtModules_IGTLRemote
class VTK_SLICER_IGTLREMOTE_MODULE_LOGIC_EXPORT vtkSlicerIGTLRemoteLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerIGTLRemoteLogic *New();
  vtkTypeMacro(vtkSlicerIGTLRemoteLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerIGTLRemoteLogic();
  virtual ~vtkSlicerIGTLRemoteLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerIGTLRemoteLogic(const vtkSlicerIGTLRemoteLogic&); // Not implemented
  void operator=(const vtkSlicerIGTLRemoteLogic&);               // Not implemented
};

#endif

