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

#ifndef __qSlicerWatchdogModuleWidget_h
#define __qSlicerWatchdogModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerWatchdogModuleExport.h"

class qSlicerWatchdogModuleWidgetPrivate;
class vtkMRMLNode;
//class vtkMRMLWatchdogNode;
//class qMRMLWatchdogToolBar;
class qSlicerToolBarManagerWidget;

/// \ingroup Slicer_QtModules_ToolWatchdog
class Q_SLICER_QTMODULES_WATCHDOG_EXPORT qSlicerWatchdogModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWatchdogModuleWidget(QWidget *parent=0);
  virtual ~qSlicerWatchdogModuleWidget();

  /// ToolBarManagerWidget is a helper class to manage the toolbar when the module widget is not created  yet
  void SetToolBarManager(qSlicerToolBarManagerWidget * );

public slots:

  /// Set the current MRML scene to the widget
  virtual void setMRMLScene( vtkMRMLScene* scene );
  /// Process loaded scene, updates the table after a scene is set
  void onSceneImportedEvent();

protected slots:

  /// Refresh the gui from the currently active toolwatchdog node 
  void updateFromMRMLNode();
  /// Update the selection node from the combobox
  void onWatchdogNodeChanged();
  /// When the user clicks the combobox to create a new watchdog node,
  /// connects the toolbar with widget for visibility control
  void onWatchdogNodeAddedByUser(vtkMRMLNode* nodeAdded);
  /// Clean up the table when toolWatchdog node is about to be removed
  void onModuleNodeAboutToBeRemoved(vtkMRMLNode* nodeToBeRemoved);
  /// Turn ON or OFF the toolbar visibility and the current watchdog node
  void onToolBarVisibilityButtonClicked();
  /// Update table every time the timer shots
  void onTimeout();
  /// When the label column is clicked it connects the cellChanged signal, to update the toolbar accordingly
  void onTableItemDoubleClicked();
  /// Updates the toolbar accordingly to the label changed on the table
  void onCurrentCellChanged(int currentRow, int currentColumn);
  /// Updates the timer accordingly with the refreshing time
  void onStatusRefreshTimeSpinBoxChanged(int statusRefeshRate);
  /// Swaps the current table row with the lower row
  void onDownButtonClicked();
  /// Swaps the current table row with the upper row
  void onUpButtonClicked();
  /// Deletes the current selected row(s)
  void onDeleteButtonClicked();
  /// Adds the tool to the watchdog node
  void onToolNodeAdded( );
  /// Moves up/down row. Deletes selected row(s)
  void onToolsTableContextMenu(const QPoint& position);
  /// Deletes and creates a table. Updates gui accordingly to node state
  void updateWidget();
  /// Updates the toolbar visibility checkbox accordingly to the toolbar visibility, in case is deactivated from the menu
  void onToolBarVisibilityChanged( bool visible );
  /// Sets the playSound option of the displayable node accordingly to the table checkbox
  void onSoundCheckBoxStateChanged(int state);

protected:
  QScopedPointer<qSlicerWatchdogModuleWidgetPrivate> d_ptr;

  /// Updates the current watchdog node table
  void updateTable();
  /// Connects the gui signals
  virtual void setup();
  /// Set up the GUI from mrml when entering
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerWatchdogModuleWidget);
  Q_DISABLE_COPY(qSlicerWatchdogModuleWidget);

  int CurrentCellPosition[2];
};

#endif
