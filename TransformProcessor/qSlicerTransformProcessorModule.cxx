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

// Qt includes
#include <QtPlugin>

// TransformProcessor Logic includes
#include <vtkSlicerTransformProcessorLogic.h>

// TransformProcessor includes
#include "qSlicerTransformProcessorModule.h"
#include "qSlicerTransformProcessorModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerTransformProcessorModule, qSlicerTransformProcessorModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformProcessor
class qSlicerTransformProcessorModulePrivate
{
public:
  qSlicerTransformProcessorModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTransformProcessorModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModulePrivate::qSlicerTransformProcessorModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTransformProcessorModule methods

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModule::qSlicerTransformProcessorModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformProcessorModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformProcessorModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModule::~qSlicerTransformProcessorModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformProcessorModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.slicerigt.org'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerTransformProcessorModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformProcessorModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Franklin King (Queen's University), Tamas Ungi (Queen's University), Thomas Vaughan (Queen's University)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTransformProcessorModule::icon()const
{
  return QIcon(":/Icons/TransformProcessor.png");
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTransformProcessorModule::createWidgetRepresentation()
{
  return new qSlicerTransformProcessorModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTransformProcessorModule::createLogic()
{
  return vtkSlicerTransformProcessorLogic::New();
}
