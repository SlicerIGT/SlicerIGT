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

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898

  ==============================================================================*/

#ifndef __qSlicerPathExplorerReslicingWidget_h
#define __qSlicerPathExplorerReslicingWidget_h

#include <sstream>

// VTK includes
#include <ctkVTKObject.h>
#include "vtkMRMLScene.h"

// SlicerQt includes
#include "qSlicerPathExplorerModuleWidgetsExport.h"
#include "qSlicerWidget.h"

class qSlicerPathExplorerReslicingWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLSliceNode;
class vtkMRMLMarkupsLineNode;

class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerReslicingWidget
: public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

 public:
  typedef qSlicerWidget Superclass;

  qSlicerPathExplorerReslicingWidget(QWidget *parent=0);
  virtual ~qSlicerPathExplorerReslicingWidget();

 public slots:
  void setSliceNode(vtkMRMLSliceNode* sliceNode);
  void setReslicingRulerNode(vtkMRMLMarkupsLineNode* ruler);
  void onMRMLSceneChanged(vtkMRMLScene* newScene);
  void onMRMLNodeRemoved(vtkObject*, void* callData);
  void onResliceToggled(bool buttonStatus);
  void onPerpendicularToggled(bool status);
  void onResliceValueChanged(int resliceValue);
  void onRulerModified();

 protected:
  QScopedPointer<qSlicerPathExplorerReslicingWidgetPrivate> d_ptr;

 private:
  Q_DECLARE_PRIVATE(qSlicerPathExplorerReslicingWidget);
  Q_DISABLE_COPY(qSlicerPathExplorerReslicingWidget);
};

#endif // __qSlicerPathExplorerReslicingWidget_h
