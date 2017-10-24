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

#include "vtkSlicerOpenIGTLinkIFLogic.h"


// OpenIGTLinkRemote includes
#include "qSlicerOpenIGTLinkRemoteModule.h"
#include "qSlicerOpenIGTLinkRemoteModuleWidget.h"
#include "qSlicerOpenIGTLinkRemoteQueryWidget.h"
#include "qSlicerOpenIGTLinkRemoteCommandWidget.h"


#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"



//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerOpenIGTLinkRemoteModule, qSlicerOpenIGTLinkRemoteModule);
#endif

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
  return "<p>This work is part of SparKit, a project funded by an Applied Cancer Research Unit of Cancer Care Ontario with funds provided by the Ministry of Health and Long-Term Care and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO) to provide free, open-source toolset for radiotherapy and related image-guided interventions.</p><p>This work was was partially funded by NIH grant 3P41RR013218-12S1</p>";
}

//-----------------------------------------------------------------------------
QStringList qSlicerOpenIGTLinkRemoteModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  return moduleContributors;
}



QIcon qSlicerOpenIGTLinkRemoteModule
::icon()const
{
  return QIcon(":/Icons/OpenIGTLinkRemote.png");
}



QStringList qSlicerOpenIGTLinkRemoteModule
::categories() const
{
  return QStringList() << "IGT";
}



QStringList qSlicerOpenIGTLinkRemoteModule
::dependencies() const
{
  return QStringList() << "OpenIGTLinkIF";
}


void qSlicerOpenIGTLinkRemoteModule
::setup()
{
  this->Superclass::setup();

  qSlicerOpenIGTLinkRemoteModuleWidget *widget =
    dynamic_cast<qSlicerOpenIGTLinkRemoteModuleWidget*>(this->widgetRepresentation());

  qSlicerAbstractCoreModule* IFModule = qSlicerCoreApplication::application()->moduleManager()->module( "OpenIGTLinkIF" );
  if ( IFModule )
  {
    vtkSlicerOpenIGTLinkIFLogic* IFLogic = vtkSlicerOpenIGTLinkIFLogic::SafeDownCast( IFModule->logic() );
    widget->setIFLogic( IFLogic );
  }
}



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
