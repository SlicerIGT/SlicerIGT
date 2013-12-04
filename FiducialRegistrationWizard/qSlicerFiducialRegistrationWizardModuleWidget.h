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

#ifndef __qSlicerFiducialRegistrationWizardModuleWidget_h
#define __qSlicerFiducialRegistrationWizardModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerSimpleMarkupsWidget.h"

#include "qSlicerFiducialRegistrationWizardModuleExport.h"

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

public slots:


protected slots:

  void UpdateToMRMLNode();
  void UpdateFromMRMLNode();

  void onRecordButtonClicked();

protected:
  QScopedPointer<qSlicerFiducialRegistrationWizardModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerFiducialRegistrationWizardModuleWidget);
  Q_DISABLE_COPY(qSlicerFiducialRegistrationWizardModuleWidget);

};

#endif
