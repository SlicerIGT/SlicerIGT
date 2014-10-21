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

// ToolWatchdog Logic includes
#include <vtkSlicerToolWatchdogLogic.h>

// ToolWatchdog includes
#include "qSlicerToolWatchdogModule.h"
#include "qSlicerToolWatchdogModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerToolWatchdogModule, qSlicerToolWatchdogModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerToolWatchdogModulePrivate
{
public:
  qSlicerToolWatchdogModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModulePrivate::qSlicerToolWatchdogModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogModule methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModule::qSlicerToolWatchdogModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerToolWatchdogModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerToolWatchdogModule::~qSlicerToolWatchdogModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerToolWatchdogModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerToolWatchdogModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerToolWatchdogModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerToolWatchdogModule::icon() const
{
  return QIcon(":/Icons/ToolWatchdog.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerToolWatchdogModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerToolWatchdogModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerToolWatchdogModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerToolWatchdogModule
::createWidgetRepresentation()
{
  return new qSlicerToolWatchdogModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerToolWatchdogModule::createLogic()
{
  return vtkSlicerToolWatchdogLogic::New();
}
