/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin, Robarts Research Institute

==============================================================================*/

#ifndef __qSlicerPlusSupportTransformWidget_h
#define __qSlicerPlusSupportTransformWidget_h

// Qt includes
#include <QWidget>

// Transform Widgets includes
#include "qSlicerPlusSupportModuleWidgetsExport.h"

class qSlicerPlusSupportTransformWidgetPrivate;
class qMRMLNodeComboBox;
class vtkMRMLNode;

class QSize;
class vtkObject;

/// \ingroup Slicer_QtModules_PlusSupport
class Q_SLICER_MODULE_PLUSSUPPORT_WIDGETS_EXPORT qSlicerPlusSupportTransformWidget : public QWidget
{
  Q_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerPlusSupportTransformWidget(QWidget* parent = 0, QSize pixmapSize = QSize(32, 32));
  virtual ~qSlicerPlusSupportTransformWidget();

  Q_INVOKABLE void setTransformSelector(qMRMLNodeComboBox* comboBox);
  Q_INVOKABLE void updateLabel();
  Q_INVOKABLE void setLabelSize(QSize& newSize);

protected slots:
  void onTransformNodeChanged(vtkMRMLNode* newNode);
  void onTimerTick();

protected:
  void onTransformNodeModified(vtkObject* caller, long unsigned int eventId, void* callData);

protected:
  QScopedPointer<qSlicerPlusSupportTransformWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPlusSupportTransformWidget);
  Q_DISABLE_COPY(qSlicerPlusSupportTransformWidget);
};

#endif
