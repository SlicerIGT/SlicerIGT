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

// FiducialRegistrationWizard Logic includes
#include <vtkSlicerFiducialRegistrationWizardLogic.h>

// FiducialRegistrationWizard includes
#include "qSlicerFiducialRegistrationWizardModule.h"
#include "qSlicerFiducialRegistrationWizardModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerFiducialRegistrationWizardModule, qSlicerFiducialRegistrationWizardModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_FiducialRegistrationWizard
class qSlicerFiducialRegistrationWizardModulePrivate
{
public:
  qSlicerFiducialRegistrationWizardModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModulePrivate::qSlicerFiducialRegistrationWizardModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModule methods

//-----------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModule::qSlicerFiducialRegistrationWizardModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerFiducialRegistrationWizardModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerFiducialRegistrationWizardModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerFiducialRegistrationWizardModule::dependencies() const
{
  return QStringList() << "Markups";
}

//-----------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModule::~qSlicerFiducialRegistrationWizardModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerFiducialRegistrationWizardModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerFiducialRegistrationWizardModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerFiducialRegistrationWizardModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Matthew Holden (Queen's University)");
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerFiducialRegistrationWizardModule::icon()const
{
  return QIcon(":/Icons/FiducialRegistrationWizard.png");
}

//-----------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModule::setup()
{
  this->Superclass::setup();

  vtkSlicerFiducialRegistrationWizardLogic* FiducialRegistrationWizardLogic = vtkSlicerFiducialRegistrationWizardLogic::SafeDownCast( this->logic() );
  qSlicerAbstractCoreModule* MarkupsModule = qSlicerCoreApplication::application()->moduleManager()->module("Markups");

  if ( MarkupsModule )
  {
    FiducialRegistrationWizardLogic->MarkupsLogic = vtkSlicerMarkupsLogic::SafeDownCast( MarkupsModule->logic() );
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerFiducialRegistrationWizardModule::createWidgetRepresentation()
{
  return new qSlicerFiducialRegistrationWizardModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerFiducialRegistrationWizardModule::createLogic()
{
  return vtkSlicerFiducialRegistrationWizardLogic::New();
}
