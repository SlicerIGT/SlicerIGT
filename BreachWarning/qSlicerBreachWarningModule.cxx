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

// BreachWarning Logic includes
#include <vtkSlicerBreachWarningLogic.h>

// BreachWarning includes
#include "qSlicerBreachWarningModule.h"
#include "qSlicerBreachWarningModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerBreachWarningModule, qSlicerBreachWarningModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_BreachWarning
class qSlicerBreachWarningModulePrivate
{
public:
  qSlicerBreachWarningModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerBreachWarningModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerBreachWarningModulePrivate::qSlicerBreachWarningModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerBreachWarningModule methods

//-----------------------------------------------------------------------------
qSlicerBreachWarningModule::qSlicerBreachWarningModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerBreachWarningModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerBreachWarningModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerBreachWarningModule::dependencies() const
{
  return QStringList() << "Markups";
}

//-----------------------------------------------------------------------------
qSlicerBreachWarningModule::~qSlicerBreachWarningModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerBreachWarningModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerBreachWarningModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerBreachWarningModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Matthew Holden (Queen's University)");
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerBreachWarningModule::icon()const
{
  return QIcon(":/Icons/BreachWarning.png");
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModule::setup()
{
  this->Superclass::setup();

  vtkSlicerBreachWarningLogic* BreachWarningLogic = vtkSlicerBreachWarningLogic::SafeDownCast( this->logic() );
  qSlicerAbstractCoreModule* MarkupsModule = qSlicerCoreApplication::application()->moduleManager()->module("Markups");

  if ( MarkupsModule )
  {
    BreachWarningLogic->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast( MarkupsModule->logic() );
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerBreachWarningModule::createWidgetRepresentation()
{
  return new qSlicerBreachWarningModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerBreachWarningModule::createLogic()
{
  return vtkSlicerBreachWarningLogic::New();
}
