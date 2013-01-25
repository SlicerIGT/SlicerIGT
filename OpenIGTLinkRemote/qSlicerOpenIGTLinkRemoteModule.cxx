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

// OpenIGTLinkRemote Logic includes
#include <vtkSlicerOpenIGTLinkRemoteLogic.h>

// OpenIGTLinkRemote includes
#include "qSlicerOpenIGTLinkRemoteModule.h"
#include "qSlicerOpenIGTLinkRemoteModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerOpenIGTLinkRemoteModule, qSlicerOpenIGTLinkRemoteModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerOpenIGTLinkRemoteModulePrivate
{
public:
  qSlicerOpenIGTLinkRemoteModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteModulePrivate
::qSlicerOpenIGTLinkRemoteModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteModule methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteModule
::qSlicerOpenIGTLinkRemoteModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerOpenIGTLinkRemoteModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteModule::~qSlicerOpenIGTLinkRemoteModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerOpenIGTLinkRemoteModule::helpText()const
{
  return "Use this module to control another program through OpenIGTLink messages.";
}

//-----------------------------------------------------------------------------
QString qSlicerOpenIGTLinkRemoteModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerOpenIGTLinkRemoteModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Perk Lab (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerOpenIGTLinkRemoteModule::icon()const
{
  return QIcon(":/Icons/OpenIGTLinkRemote.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerOpenIGTLinkRemoteModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerOpenIGTLinkRemoteModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerOpenIGTLinkRemoteModule
::createWidgetRepresentation()
{
  return new qSlicerOpenIGTLinkRemoteModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerOpenIGTLinkRemoteModule::createLogic()
{
  return vtkSlicerOpenIGTLinkRemoteLogic::New();
}
