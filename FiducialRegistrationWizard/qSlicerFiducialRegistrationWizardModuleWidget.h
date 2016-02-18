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

#ifndef __qSlicerFiducialRegistrationWizardModuleWidget_h
#define __qSlicerFiducialRegistrationWizardModuleWidget_h

#include "qSlicerFiducialRegistrationWizardModuleExport.h"

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

class qSlicerFiducialRegistrationWizardModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_FiducialRegistrationWizard
class Q_SLICER_QTMODULES_FIDUCIALREGISTRATIONWIZARD_EXPORT qSlicerFiducialRegistrationWizardModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerFiducialRegistrationWizardModuleWidget(QWidget *parent=0);
  virtual ~qSlicerFiducialRegistrationWizardModuleWidget();

  std::string GetCorrespondingFiducialString();

public slots:


protected slots:

  void UpdateToMRMLNode();
  void UpdateFromMRMLNode();

  // Update widget after node is added or deleted
  void PostProcessFromMarkupsWidget();
  void PostProcessToMarkupsWidget();

  void onRecordFromButtonClicked();
  void onRecordToButtonClicked();
  void onUpdateButtonClicked();
  void onUpdateButtonToggled(bool);
  void onFiducialRegistrationWizardNodeSelectionChanged();
  void onAutoUpdateSelected();
  void onManualUpdateSelected();

protected:
  QScopedPointer<qSlicerFiducialRegistrationWizardModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();
  virtual bool eventFilter(QObject * obj, QEvent *event);

private:
  Q_DECLARE_PRIVATE(qSlicerFiducialRegistrationWizardModuleWidget);
  Q_DISABLE_COPY(qSlicerFiducialRegistrationWizardModuleWidget);

};

#endif
