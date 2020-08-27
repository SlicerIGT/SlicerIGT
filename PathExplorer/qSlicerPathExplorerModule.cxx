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

// PathExplorer Logic includes
#include <vtkSlicerPathExplorerLogic.h>

// PathExplorer includes
#include "qSlicerPathExplorerModule.h"
#include "qSlicerPathExplorerModuleWidget.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyPathExplorerPlugin.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerPathExplorerModule, qSlicerPathExplorerModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPathExplorerModulePrivate
{
public:
  qSlicerPathExplorerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPathExplorerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPathExplorerModulePrivate
::qSlicerPathExplorerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPathExplorerModule methods

//-----------------------------------------------------------------------------
qSlicerPathExplorerModule
::qSlicerPathExplorerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPathExplorerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerModule::~qSlicerPathExplorerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPathExplorerModule::helpText()const
{
  return QString("Path Explorer is a module designed to facilitate the creation of trajectory, and visualization of volumes along these trajectories"
    " For help on how to use this module visit: <a href='https://www.slicerigt.org'>SlicerIGT website</a>.");
}

//-----------------------------------------------------------------------------
QString qSlicerPathExplorerModule::acknowledgementText()const
{
  return QString("It is supported by grants 5P01CA067165, 5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,"
                 "5R42CA137886, 8P41EB015898");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathExplorerModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Laurent Chauvin (SNR), Atsushi Yamada (SNR), Junichi Tokuda (SNR)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPathExplorerModule::icon()const
{
  return QIcon(":/Icons/PathExplorer.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathExplorerModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathExplorerModule::dependencies() const
{
  return QStringList() << "Markups";
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerModule::setup()
{
  this->Superclass::setup();
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyPathExplorerPlugin());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerPathExplorerModule
::createWidgetRepresentation()
{
  return new qSlicerPathExplorerModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPathExplorerModule::createLogic()
{
  return vtkSlicerPathExplorerLogic::New();
}
