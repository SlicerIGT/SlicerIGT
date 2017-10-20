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

#ifndef __qSlicerCollectFiducialsModuleWidget_h
#define __qSlicerCollectFiducialsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerCollectFiducialsModuleExport.h"

class qSlicerCollectFiducialsModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_CollectFiducials
class Q_SLICER_QTMODULES_COLLECTFIDUCIALS_EXPORT qSlicerCollectFiducialsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCollectFiducialsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCollectFiducialsModuleWidget();

public slots:
  void setMRMLScene( vtkMRMLScene* scene );

protected slots:
  void blockAllSignals( bool block );
  void enableAllWidgets( bool enable );
  void onSceneImportedEvent();
  void onParameterNodeSelected();
  void onProbeTransformNodeSelected();
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
  QScopedPointer<qSlicerCollectFiducialsModuleWidgetPrivate> d_ptr;
  virtual void setup();
  virtual void enter();
  virtual void exit();

private:
  Q_DECLARE_PRIVATE(qSlicerCollectFiducialsModuleWidget);
  Q_DISABLE_COPY(qSlicerCollectFiducialsModuleWidget);
};

#endif
