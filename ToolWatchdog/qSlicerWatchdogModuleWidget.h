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

/// \ingroup Slicer_QtModules_ToolWatchdog
class Q_SLICER_QTMODULES_WATCHDOG_EXPORT qSlicerWatchdogModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWatchdogModuleWidget(QWidget *parent=0);
  virtual ~qSlicerWatchdogModuleWidget();

  bool visibility()const;
  int fontSize()const;
  double opacity()const;
  QColor backgroundColor()const;
  QColor textColor()const;

public slots:

  /// Set the current MRML scene to the widget
  virtual void setMRMLScene( vtkMRMLScene* scene );
  /// Process loaded scene, updates the table after a scene is set
  void onSceneImportedEvent();

  void setVisibility(bool);
  void setFontSize(int);
  void setOpacity(double);
  void setBackgroundColor(QColor);
  void setTextColor(QColor);

  void updateMRMLDisplayNodeViewsFromWidget();

protected slots:

  /// Starts/stops playing sound based on the status of all current tools
  //void onUpdateSound();

  /// Refresh the gui from the currently active toolwatchdog node 
  void updateFromMRMLNode();
  /// Update the selection node from the combobox
  void onWatchdogNodeSelectionChanged();
  /// Update the selection node from the combobox
  void onWatchdogNodeModified();
  /// When the label column is clicked it connects the cellChanged signal, to update the toolbar accordingly
  void onTableItemDoubleClicked();
  /// Updates the toolbar accordingly to the label changed on the table
  void onCurrentCellChanged(int currentRow, int currentColumn);
  /// Deletes the current selected row(s)
  void onRemoveToolNode();
  /// Adds the tool to the watchdog node
  void onAddToolNode( );
  /// Moves up/down row. Deletes selected row(s)
  void onToolsTableContextMenu(const QPoint& position);
  /// Deletes and creates a table. Updates gui accordingly to node state
  void updateWidget();
  /// Sets the playSound option of the displayable node accordingly to the table checkbox
  void onSoundCheckBoxStateChanged(int state);

protected:
  QScopedPointer<qSlicerWatchdogModuleWidgetPrivate> d_ptr;

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
