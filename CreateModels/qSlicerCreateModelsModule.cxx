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

// ExtensionTemplate Logic includes
#include <vtkSlicerCreateModelsLogic.h>

// ExtensionTemplate includes
#include "qSlicerCreateModelsModule.h"
#include "qSlicerCreateModelsModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerCreateModelsModule, qSlicerCreateModelsModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCreateModelsModulePrivate
{
public:
  qSlicerCreateModelsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCreateModelsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCreateModelsModulePrivate::qSlicerCreateModelsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCreateModelsModule methods

//-----------------------------------------------------------------------------
qSlicerCreateModelsModule::qSlicerCreateModelsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCreateModelsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCreateModelsModule::~qSlicerCreateModelsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerCreateModelsModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerCreateModelsModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCreateModelsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString( "Tamas Ungi (Queen's University)" );
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCreateModelsModule::icon()const
{
  return QIcon(":/Icons/CreateModels.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCreateModelsModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCreateModelsModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerCreateModelsModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerCreateModelsModule::createWidgetRepresentation()
{
  return new qSlicerCreateModelsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCreateModelsModule::createLogic()
{
  return vtkSlicerCreateModelsLogic::New();
}
