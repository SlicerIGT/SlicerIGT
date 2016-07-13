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

#ifndef __qSlicerMarkupsToModelModuleWidget_h
#define __qSlicerMarkupsToModelModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMarkupsToModelModuleExport.h"

class qSlicerMarkupsToModelModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_MARKUPSTOMODEL_EXPORT qSlicerMarkupsToModelModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMarkupsToModelModuleWidget(QWidget *parent=0);
  virtual ~qSlicerMarkupsToModelModuleWidget();

public slots:
  void setMRMLScene( vtkMRMLScene* scene );
  void onSceneImportedEvent();

  void updateFromMRMLNode();
  void updateToMRMLNode();

  void blockAllSignals();
  void unblockAllSignals();
  void enableAllWidgets();
  void disableAllWidgets();
  
  void UpdateOutputModel();

  void onActionUpdateAuto();
  void onActionUpdateManual();

protected:
  QScopedPointer<qSlicerMarkupsToModelModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();
  virtual void exit();
  virtual bool eventFilter(QObject * obj, QEvent *event);

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsToModelModuleWidget);
  Q_DISABLE_COPY(qSlicerMarkupsToModelModuleWidget);

  int previouslyPersistent;
};

#endif
