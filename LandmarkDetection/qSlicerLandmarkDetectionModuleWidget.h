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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qSlicerLandmarkDetectionModuleWidget_h
#define __qSlicerLandmarkDetectionModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerLandmarkDetectionModuleExport.h"

class qSlicerLandmarkDetectionModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_LANDMARKDETECTION_EXPORT qSlicerLandmarkDetectionModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerLandmarkDetectionModuleWidget(QWidget *parent=0);
  virtual ~qSlicerLandmarkDetectionModuleWidget();

protected slots:
  virtual void onCurrentNodeChanged(vtkMRMLNode* newCurrentNode);
  virtual void updateWidgetFromMRML();
  virtual void updateMRMLFromWidget();
  virtual void startStopButonClicked();

protected:
  QScopedPointer<qSlicerLandmarkDetectionModuleWidgetPrivate> d_ptr;

  void setup() override;
  void enter() override;

private:
  Q_DECLARE_PRIVATE(qSlicerLandmarkDetectionModuleWidget);
  Q_DISABLE_COPY(qSlicerLandmarkDetectionModuleWidget);
};

#endif
