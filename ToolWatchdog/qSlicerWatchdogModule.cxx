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

// Watchdog Logic includes
#include <vtkSlicerWatchdogLogic.h>

// Watchdog includes
#include "qSlicerWatchdogModule.h"
#include "qSlicerWatchdogModuleWidget.h"
#include "qSlicerToolBarManagerWidget.h"


//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWatchdogModule, qSlicerWatchdogModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ToolWatchdog
class qSlicerWatchdogModulePrivate
{
public:
  qSlicerWatchdogModulePrivate();
  ~qSlicerWatchdogModulePrivate();
  qSlicerToolBarManagerWidget * ToolBarManager;
};

//-----------------------------------------------------------------------------
// qSlicerWatchdogModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModulePrivate::qSlicerWatchdogModulePrivate()
{
  this->ToolBarManager=NULL;

}

//-----------------------------------------------------------------------------
qSlicerWatchdogModulePrivate::~qSlicerWatchdogModulePrivate()
{
  if(this->ToolBarManager)
  {
    delete this->ToolBarManager;
  }
}

//-----------------------------------------------------------------------------
// qSlicerWatchdogModule methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModule::qSlicerWatchdogModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWatchdogModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerWatchdogModule::~qSlicerWatchdogModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerWatchdogModule::helpText() const
{
  return "For help on how to use this module visit: <a href='http://www.slicerigt.org/'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerWatchdogModule::acknowledgementText() const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWatchdogModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jaime Garcia-Guevara (Queen's University)");
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerWatchdogModule::icon() const
{
  return QIcon(":/Icons/Watchdog.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWatchdogModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWatchdogModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  this->Superclass::setMRMLScene(_mrmlScene);
  Q_D(qSlicerWatchdogModule);
  if (d->ToolBarManager == NULL)
  {
    d->ToolBarManager = new qSlicerToolBarManagerWidget;
  }
  d->ToolBarManager->setMRMLScene(_mrmlScene);
  vtkSlicerWatchdogLogic* watchdogLogic = vtkSlicerWatchdogLogic::SafeDownCast(this->Superclass::logic());
  d->ToolBarManager->setLogic(watchdogLogic);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerWatchdogModule::createWidgetRepresentation()
{
  Q_D(qSlicerWatchdogModule);
  qSlicerWatchdogModuleWidget * watchdogWidget = new qSlicerWatchdogModuleWidget;
  watchdogWidget->SetToolBarManager(d->ToolBarManager);
  return watchdogWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerWatchdogModule::createLogic()
{
  return vtkSlicerWatchdogLogic::New();
}
