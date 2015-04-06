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

// MarkupsToModel Logic includes
#include <vtkSlicerMarkupsToModelLogic.h>

// MarkupsToModel includes
#include "qSlicerMarkupsToModelModule.h"
#include "qSlicerMarkupsToModelModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMarkupsToModelModule, qSlicerMarkupsToModelModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMarkupsToModelModulePrivate
{
public:
  qSlicerMarkupsToModelModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsToModelModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModulePrivate::qSlicerMarkupsToModelModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsToModelModule methods

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModule::qSlicerMarkupsToModelModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsToModelModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelModule::~qSlicerMarkupsToModelModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsToModelModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsToModelModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsToModelModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerMarkupsToModelModule::icon() const
{
  return QIcon(":/Icons/MarkupsToModel.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsToModelModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsToModelModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsToModelModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerMarkupsToModelModule
::createWidgetRepresentation()
{
  return new qSlicerMarkupsToModelModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMarkupsToModelModule::createLogic()
{
  return vtkSlicerMarkupsToModelLogic::New();
}
