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

#ifndef __qSlicerWatchdogModule_h
#define __qSlicerWatchdogModule_h

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerWatchdogModuleExport.h"

class qSlicerWatchdogModulePrivate;

/// \ingroup Slicer_QtModules_Watchdog
class Q_SLICER_QTMODULES_WATCHDOG_EXPORT qSlicerWatchdogModule
  : public qSlicerLoadableModule
{
  Q_OBJECT;
  QVTK_OBJECT;
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerWatchdogModule(QObject *parent=0);
  virtual ~qSlicerWatchdogModule();

  qSlicerGetTitleMacro(tr("Watchdog"));

  virtual QString helpText() const override;
  virtual QString acknowledgementText() const override;
  virtual QStringList contributors() const override;

  virtual QIcon icon() const override;

  virtual QStringList categories() const override;
  virtual QStringList dependencies() const override;

protected:

  /// Initialize the module.
  virtual void setup() override;

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic() override;

public slots:
  virtual void setMRMLScene(vtkMRMLScene*) override;
  void onNodeAddedEvent(vtkObject*, vtkObject*);
  void onNodeRemovedEvent(vtkObject*, vtkObject*);
  void updateAllWatchdogNodes();
  void stopSound();

protected:
  QScopedPointer<qSlicerWatchdogModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWatchdogModule);
  Q_DISABLE_COPY(qSlicerWatchdogModule);

};

#endif
