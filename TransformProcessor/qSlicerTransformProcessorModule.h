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

#ifndef __qSlicerTransformProcessorModule_h
#define __qSlicerTransformProcessorModule_h

// CTK includes
#include <ctkVTKObject.h> 

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTransformProcessorModuleExport.h"

class qSlicerTransformProcessorModulePrivate;

/// \ingroup Slicer_QtModules_TransformProcessor
class Q_SLICER_QTMODULES_TRANSFORMPROCESSOR_EXPORT qSlicerTransformProcessorModule :
  public qSlicerLoadableModule
{
  Q_OBJECT;
  QVTK_OBJECT;
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerTransformProcessorModule(QObject *parent=0);
  virtual ~qSlicerTransformProcessorModule();

  qSlicerGetTitleMacro(tr("Transform Processor"));
  
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

public slots:
  void onNodeAddedEvent(vtkObject*, vtkObject*);
  void onNodeRemovedEvent(vtkObject*, vtkObject*);
  void updateAllOutputs();

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup() override;

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerTransformProcessorModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformProcessorModule);
  Q_DISABLE_COPY(qSlicerTransformProcessorModule);

};

#endif
