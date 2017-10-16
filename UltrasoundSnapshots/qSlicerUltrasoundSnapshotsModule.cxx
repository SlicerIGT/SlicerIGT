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
#include <vtkSlicerUltrasoundSnapshotsLogic.h>

// ExtensionTemplate includes
#include "qSlicerUltrasoundSnapshotsModule.h"
#include "qSlicerUltrasoundSnapshotsModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerUltrasoundSnapshotsModule, qSlicerUltrasoundSnapshotsModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerUltrasoundSnapshotsModulePrivate
{
public:
  qSlicerUltrasoundSnapshotsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerUltrasoundSnapshotsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerUltrasoundSnapshotsModulePrivate::qSlicerUltrasoundSnapshotsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerUltrasoundSnapshotsModule methods

//-----------------------------------------------------------------------------
qSlicerUltrasoundSnapshotsModule::qSlicerUltrasoundSnapshotsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerUltrasoundSnapshotsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerUltrasoundSnapshotsModule::~qSlicerUltrasoundSnapshotsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerUltrasoundSnapshotsModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerUltrasoundSnapshotsModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerUltrasoundSnapshotsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString( "Tamas Ungi (Queen's University)" );
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerUltrasoundSnapshotsModule::icon()const
{
  return QIcon(":/Icons/UltrasoundSnapshots.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerUltrasoundSnapshotsModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerUltrasoundSnapshotsModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerUltrasoundSnapshotsModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerUltrasoundSnapshotsModule::createWidgetRepresentation()
{
  return new qSlicerUltrasoundSnapshotsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerUltrasoundSnapshotsModule::createLogic()
{
  return vtkSlicerUltrasoundSnapshotsLogic::New();
}
