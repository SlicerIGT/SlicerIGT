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

#ifndef __qSlicerToolWatchdogModuleWidget_h
#define __qSlicerToolWatchdogModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerToolWatchdogModuleExport.h"

class QTimer;
class qSlicerToolWatchdogModuleWidgetPrivate;
class vtkMRMLNode;
class qMRMLToolWatchdogToolBar;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_TOOLWATCHDOG_EXPORT qSlicerToolWatchdogModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerToolWatchdogModuleWidget(QWidget *parent=0);
  virtual ~qSlicerToolWatchdogModuleWidget();

  //void SetCurrentNode( vtkMRMLNode* currentNode );

public slots:

  /// Update the table after a scene is set
  virtual void setMRMLScene( vtkMRMLScene* scene );

  /// Update the table after a scene is imported
  void onSceneImportedEvent();

protected slots:

  /// Refresh the gui from the currently active toolwatchdog node as determined by
  /// the selection node
  void updateFromMRMLNode();

  /// Update the selection node from the combobox
  void onToolWatchdogNodeChanged();
  /// When the user clicks in the combobox to create a new watchdog node,
  /// inserts a new toolbar
  void onToolWatchdogNodeAddedByUser(vtkMRMLNode* nodeAdded);
  /// Remove the tool bar when removed toolWatchdog if the node is active
  void onModuleNodeAboutToBeRemoved(vtkMRMLNode* nodeToBeRemoved);
  ///Turn ON or OFF the tool bar visibility and the current watchdog node
  void onToolbarVisibilityButtonClicked();

  /// Updates toolbar(s), table and displayable nodes observed everytime the timer shots
  void onTimeout();
  /// When the label column is clicked it connects the cellChanged signal, to update the toolbar accordingly
  void onTableItemDoubleClicked();
  /// Updates the toolbar accordingly to the label changed on the table
  void onCurrentCellChanged(int currentRow, int currentColumn);
  /// Updates the timer accordingly with the refreshing rate
  void onStatusRefreshRateSpinBoxChanged(int statusRefeshRate);
  /// Swaps the current table row with the down row 
  void onDownButtonClicked();
  /// Swaps the current table row with the upper row 
  void onUpButtonClicked();
  /// Deletes the current selected row(s)
  void onDeleteButtonClicked();
  /// Adds the observed tool to the toolbar and to the watchdog node
  void onToolNodeAdded( );
  /// Moves up/down row. Deletes slected row(s)
  void onToolsTableContextMenu(const QPoint& position);
  /// Deletes and creates a table. Updates gui accordingly to node state
  void updateWidget();
  /// Updates the toolbar visibility checkbox accordingly to the toolbar visibility, in case is deactivated from the menu
  void onToolbarVisibilityChanged( bool visible );
  //void onToolChanged();

protected:
  QScopedPointer<qSlicerToolWatchdogModuleWidgetPrivate> d_ptr;

  /// Updates all the active toolbars accordingly to their respective watchdog nodes states
  void updateToolbars();
  /// Updates the current watchdog node table
  void updateTable();
  /// Connects the gui signals
  virtual void setup();

  /// Set up the GUI from mrml when entering
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerToolWatchdogModuleWidget);
  Q_DISABLE_COPY(qSlicerToolWatchdogModuleWidget);

  QTimer* Timer;
  double StatusRefreshTimeSec;
  int CurrentCellPosition[2];
  double ElapsedTimeSec;
};

#endif
