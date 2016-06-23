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
  //void onModelNodeChanged();
  /// Refresh the gui from the currently active toolwatchdog node 
  void updateFromMRMLNode();
  /// Deletes and creates a table. Updates gui accordingly to node state
  void updateWidget();


  void onModelNodeChanged();

  /// Update the selection node from the combobox
  void onParameterNodeChanged();
  void onNodeAboutToBeEdited(vtkMRMLNode* node);


  void onCurrentMarkupsNodeChanged();
  /// When the user clicks the combobox to create a new MarkupsToModel node,
  /// connects the toolbar with widget for visibility control
  void onParameterNodeAddedByUser(vtkMRMLNode* nodeAdded);
  /// Clean up the table when toolMarkupsToModel node is about to be removed
  //void onMarkupsToModelModuleNodeAboutToBeRemoved(vtkMRMLNode* nodeToBeRemoved);

  void onUpdateOutputModelPushButton();

  void onDeleteAllPushButton();
  void onDeleteLastModelPushButton();
  void onPlacePushButtonClicked();

  void onInterpolationBoxClicked( bool nana );
  void onModeGroupBoxClicked( bool nana );
  void onOutputOpacityValueChanged( double outputTransparency );
  void onOutputColorChanged( QColor newColor );
  void onTubeRadiusDoubleChanged( double tubeRadius );
  void onKochanekContinuityDoubleChanged( double kochanekContinuity );
  void onKochanekBiasDoubleChanged( double kochanekBias );
  void onKochanekTensionDoubleChanged( double kochanekTension );

  void onDelaunayAlphaDoubleChanged( double delaunayAlpha );
  void onOutputVisibilityToggled( bool outputVisibility );
  void onTextScaleChanged( double textScale );
  void onOutputIntersectionVisibilityToggled( bool outputIntersectionVisibility );
  void onCleanMarkupsToggled( bool cleanMarkups );
  void onAutoUpdateOutputToggled( bool autoUpdateOutput );
  void onButterflySubdivisionToggled( bool butterflySubdivision );


protected:
  QScopedPointer<qSlicerMarkupsToModelModuleWidgetPrivate> d_ptr;

  void UpdateOutputModel();

  virtual void setup();
  virtual void enter();
  virtual void exit();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsToModelModuleWidget);
  Q_DISABLE_COPY(qSlicerMarkupsToModelModuleWidget);

  int previouslyPersistent;
};

#endif
