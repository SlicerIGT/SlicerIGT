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

#ifndef __qSlicerCollectPointsModuleWidget_h
#define __qSlicerCollectPointsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerCollectPointsModuleExport.h"

class qSlicerCollectPointsModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_CollectPoints
class Q_SLICER_QTMODULES_COLLECTPOINTS_EXPORT qSlicerCollectPointsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCollectPointsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCollectPointsModuleWidget();

public slots:
  void setMRMLScene( vtkMRMLScene* scene );

protected slots:
  void blockAllSignals( bool block );
  void enableAllWidgets( bool enable );
  void onSceneImportedEvent();
  void onParameterNodeSelected();
  void onSamplingTransformNodeSelected();
  void onAnchorTransformNodeSelected();
  void onLabelBaseChanged();
  void onLabelCounterChanged();
  void onMinimumDistanceChanged();
  void onOutputNodeAdded( vtkMRMLNode* );
  void onOutputNodeSelected( vtkMRMLNode* );
  void onColorButtonChanged( QColor );
  void onDeleteButtonClicked();
  void onDeleteAllClicked();
  void onVisibilityButtonClicked();
  void onCollectClicked();
  void onCollectCheckboxToggled();
  void updateGUIFromMRML();
  
protected:
  QScopedPointer<qSlicerCollectPointsModuleWidgetPrivate> d_ptr;
  virtual void setup();
  virtual void enter();
  virtual void exit();

private:
  Q_DECLARE_PRIVATE(qSlicerCollectPointsModuleWidget);
  Q_DISABLE_COPY(qSlicerCollectPointsModuleWidget);
};

#endif
