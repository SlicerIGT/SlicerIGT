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

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

#ifndef __vtkMRMLWatchdogDisplayableManager_h
#define __vtkMRMLWatchdogDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkSlicerWatchdogModuleMRMLDisplayableManagerExport.h"

/// \brief Display watchdog in 3D views
///
/// Displays watchdog in 3D viewers
///
class VTK_SLICER_WATCHDOG_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLWatchdogDisplayableManager
  : public vtkMRMLAbstractDisplayableManager
{
public:

  static vtkMRMLWatchdogDisplayableManager* New();
  vtkTypeMacro(vtkMRMLWatchdogDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLWatchdogDisplayableManager();
  virtual ~vtkMRMLWatchdogDisplayableManager();

  virtual void UnobserveMRMLScene() override;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Update Actors based on watchdog in the scene
  virtual void UpdateFromMRML() override;

  virtual void OnMRMLSceneStartClose() override;
  virtual void OnMRMLSceneEndClose() override;

  virtual void OnMRMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager
  virtual void Create() override;

private:

  vtkMRMLWatchdogDisplayableManager(const vtkMRMLWatchdogDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLWatchdogDisplayableManager&);                 // Not Implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
