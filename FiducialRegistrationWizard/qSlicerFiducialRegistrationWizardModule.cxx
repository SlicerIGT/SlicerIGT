/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QtPlugin>

// FiducialRegistrationWizard Logic includes
#include <vtkSlicerFiducialRegistrationWizardLogic.h>

// FiducialRegistrationWizard includes
#include "qSlicerFiducialRegistrationWizardModule.h"
#include "qSlicerFiducialRegistrationWizardModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerFiducialRegistrationWizardModule, qSlicerFiducialRegistrationWizardModule);
#endif

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

  vtkSlicerFiducialRegistrationWizardLogic* fiducialRegistrationWizardLogic = vtkSlicerFiducialRegistrationWizardLogic::SafeDownCast( this->logic() );
  
  qSlicerAbstractCoreModule* markupsModule = qSlicerCoreApplication::application()->moduleManager()->module("Markups");
  if (markupsModule)
    {
    vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(markupsModule->logic());
    fiducialRegistrationWizardLogic->SetMarkupsLogic(markupsLogic);
    }
  else
    {
    qWarning("Markups module is not found. qSlicerFiducialRegistrationWizardModule module initialization is incomplete.");
    }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerFiducialRegistrationWizardModule::createWidgetRepresentation()
{
  return new qSlicerFiducialRegistrationWizardModuleWidget();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerFiducialRegistrationWizardModule::createLogic()
{
  return vtkSlicerFiducialRegistrationWizardLogic::New();
}
