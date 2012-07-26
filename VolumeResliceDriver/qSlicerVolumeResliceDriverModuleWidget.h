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

#ifndef __qSlicerRealTimeImagingModuleWidget_h
#define __qSlicerRealTimeImagingModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerRealTimeImagingModuleExport.h"

class qSlicerRealTimeImagingModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_RealTimeImaging
class Q_SLICER_QTMODULES_REALTIMEIMAGING_EXPORT qSlicerRealTimeImagingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerRealTimeImagingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerRealTimeImagingModuleWidget();

public slots:
  virtual void setMRMLScene(vtkMRMLScene *newScene);
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onLayoutChanged(int);

protected:
  QScopedPointer<qSlicerRealTimeImagingModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerRealTimeImagingModuleWidget);
  Q_DISABLE_COPY(qSlicerRealTimeImagingModuleWidget);
};

#endif
