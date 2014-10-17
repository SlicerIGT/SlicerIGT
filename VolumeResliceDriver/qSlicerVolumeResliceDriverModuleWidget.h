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

#ifndef __qSlicerVolumeResliceDriverModuleWidget_h
#define __qSlicerVolumeResliceDriverModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumeResliceDriverModuleExport.h"

class qSlicerVolumeResliceDriverModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_VolumeResliceDriver
class Q_SLICER_QTMODULES_VOLUMERESLICEDRIVER_EXPORT qSlicerVolumeResliceDriverModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeResliceDriverModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVolumeResliceDriverModuleWidget();

public slots:
  virtual void setMRMLScene(vtkMRMLScene *newScene);
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onLayoutChanged(int);
  void onAdvancedCheckBoxChanged( int );

protected:
  QScopedPointer<qSlicerVolumeResliceDriverModuleWidgetPrivate> d_ptr;
  
  virtual void setup();


private:
  Q_DECLARE_PRIVATE(qSlicerVolumeResliceDriverModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeResliceDriverModuleWidget);
};

#endif
