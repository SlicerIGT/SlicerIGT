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

  This file was originally developed by Franklin King, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerTransformFusionModuleWidget_h
#define __qSlicerTransformFusionModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTransformFusionModuleExport.h"

class qSlicerTransformFusionModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_TransformFusion
class Q_SLICER_QTMODULES_TRANSFORMFUSION_EXPORT qSlicerTransformFusionModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTransformFusionModuleWidget( QWidget *parent = 0 );
  virtual ~qSlicerTransformFusionModuleWidget();

  virtual void enter();

public slots:
  virtual void setMRMLScene( vtkMRMLScene* );
  void onSceneImportedEvent();
  void onLogicModified();
  void setTransformFusionParametersNode( vtkMRMLNode* );
  void updateWidget();
  void updateButtons();
  void updateInputFieldVisibility();
  void updateInputList();
  void updateRateSpinBoxVisibility();

  void setSignalsBlocked( bool );
  bool getSignalsBlocked();

protected slots:

  void onAddInputTransform();
  void onRemoveInputTransform();
  void onSingleInputTransformNodeSelected( vtkMRMLNode* node );
  void onReferenceTransformNodeSelected( vtkMRMLNode* node );
  void onRestingTransformNodeSelected( vtkMRMLNode* node );
  void onOutputTransformNodeSelected( vtkMRMLNode* node );

  void onFusionModeChanged( int );

  void onActionUpdateManual();
  void onActionUpdateAuto();
  void onActionUpdateTimed();
  void onUpdateButtonPressed();
  void stopExistingUpdates();
  void singleUpdate();
  void handleEventAutoUpdate();
  void handleEventTimedUpdate();

  void setUpdatesPerSecond( double );
  
protected:
  QScopedPointer< qSlicerTransformFusionModuleWidgetPrivate > d_ptr;
  
  virtual bool eventFilter(QObject * obj, QEvent *event);
  virtual void setup();
  void onEnter();

  QTimer* updateTimer;

private:
  Q_DECLARE_PRIVATE( qSlicerTransformFusionModuleWidget );
  Q_DISABLE_COPY( qSlicerTransformFusionModuleWidget );
};

#endif
