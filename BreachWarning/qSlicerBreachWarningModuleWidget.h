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

#ifndef __qSlicerBreachWarningModuleWidget_h
#define __qSlicerBreachWarningModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"


#include "qSlicerBreachWarningModuleExport.h"

class qSlicerBreachWarningModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_BreachWarning
class Q_SLICER_QTMODULES_BREACHWARNING_EXPORT qSlicerBreachWarningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerBreachWarningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerBreachWarningModuleWidget();

public slots:

  virtual void setMRMLScene( vtkMRMLScene* scene );
  void onSceneImportedEvent();

protected slots:

  void onParameterNodeChanged();

  void onModelNodeChanged();
  void onToolTransformChanged();
  void PlayWarningSound(bool warningSound);
  void DisplayWarningColor(bool displayWarningColor);
  void UpdateWarningColor( QColor newColor );
  void UpdateFromMRMLNode();
  void LineToClosestPointVisibilityChanged(bool displayRuler);
  void LineToClosestPointColorChanged( QColor newColor );  
  void LineToClosestPointTextSizeChanged(double size);
  void LineToClosestPointThicknessChanged(double thickness);
  void WarningDistanceMMChanged(double warningDistanceMM);

protected:
  QScopedPointer<qSlicerBreachWarningModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerBreachWarningModuleWidget);
  Q_DISABLE_COPY(qSlicerBreachWarningModuleWidget);

};

#endif
