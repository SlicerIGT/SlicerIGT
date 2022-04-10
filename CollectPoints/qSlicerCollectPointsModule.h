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

#ifndef __qSlicerCollectPointsModule_h
#define __qSlicerCollectPointsModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerCollectPointsModuleExport.h"

class qSlicerCollectPointsModulePrivate;

/// \ingroup Slicer_QtModules_CollectPoints
class Q_SLICER_QTMODULES_COLLECTPOINTS_EXPORT qSlicerCollectPointsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerCollectPointsModule(QObject *parent=0);
  virtual ~qSlicerCollectPointsModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  /// Help to use the module
  virtual QString helpText() const override;

  /// Return acknowledgements
  virtual QString acknowledgementText() const override;

  /// Return the authors of the module
  virtual QStringList  contributors() const override;

  /// Return a custom icon for the module
  virtual QIcon icon() const override;

  /// Return the categories for the module
  virtual QStringList categories() const override;

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup() override;

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerCollectPointsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCollectPointsModule);
  Q_DISABLE_COPY(qSlicerCollectPointsModule);

};

#endif
