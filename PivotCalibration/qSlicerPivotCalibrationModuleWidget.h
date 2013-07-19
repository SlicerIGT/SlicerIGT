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

#ifndef __qSlicerPivotCalibrationModuleWidget_h
#define __qSlicerPivotCalibrationModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include <QTimer>

#include <vtkMRMLNode.h>

#include "qSlicerPivotCalibrationModuleExport.h"

class qSlicerPivotCalibrationModuleWidgetPrivate;
class vtkAddSampleCallback;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PIVOTCALIBRATION_EXPORT qSlicerPivotCalibrationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPivotCalibrationModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPivotCalibrationModuleWidget();
  
  virtual void enter();

protected slots:
  void initializeObserver(vtkMRMLNode*);
  void onStartButton();
  void onStop();
  
  void setTimer(double);
  
  void onDelayTimeout();
  void onSamplingTimeout();
  
protected:
  QScopedPointer<qSlicerPivotCalibrationModuleWidgetPrivate> d_ptr;

  virtual void setup();
  
  int timerSetting;
  
  QTimer* delayTimer;
  int delayCount;
  
  QTimer* samplingTimer;
  int samplingCount;


private:
  Q_DECLARE_PRIVATE(qSlicerPivotCalibrationModuleWidget);
  Q_DISABLE_COPY(qSlicerPivotCalibrationModuleWidget);
};

#endif
