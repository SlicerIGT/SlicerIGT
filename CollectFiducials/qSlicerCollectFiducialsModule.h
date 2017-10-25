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

#ifndef __qSlicerCollectFiducialsModule_h
#define __qSlicerCollectFiducialsModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerCollectFiducialsModuleExport.h"

class qSlicerCollectFiducialsModulePrivate;

/// \ingroup Slicer_QtModules_CollectFiducials
class Q_SLICER_QTMODULES_COLLECTFIDUCIALS_EXPORT qSlicerCollectFiducialsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerCollectFiducialsModule(QObject *parent=0);
  virtual ~qSlicerCollectFiducialsModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  /// Help to use the module
  virtual QString helpText()const;

  /// Return acknowledgements
  virtual QString acknowledgementText()const;

  /// Return the authors of the module
  virtual QStringList  contributors()const;

  /// Return a custom icon for the module
  virtual QIcon icon()const;

  /// Return the categories for the module
  virtual QStringList categories()const;

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerCollectFiducialsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCollectFiducialsModule);
  Q_DISABLE_COPY(qSlicerCollectFiducialsModule);

};

#endif