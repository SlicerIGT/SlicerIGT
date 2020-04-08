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

// MetafileImporter Logic includes
#include "vtkSlicerMetafileImporterLogic.h"
#include "vtkSlicerSequencesLogic.h"

// MetafileImporter includes
#include "qSlicerMetafileImporterModule.h"
#include "qSlicerMetafileImporterModuleWidget.h"
#include "qSlicerMetafileReader.h"
#include "qSlicerSequencesReader.h"

// Slicer includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerNodeWriter.h"
#include "qSlicerSequencesModule.h"
#include "qSlicerSequencesModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerMetafileImporterModule, qSlicerMetafileImporterModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMetafileImporterModulePrivate
{
public:
  qSlicerMetafileImporterModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMetafileImporterModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMetafileImporterModulePrivate
::qSlicerMetafileImporterModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMetafileImporterModule methods

//-----------------------------------------------------------------------------
qSlicerMetafileImporterModule
::qSlicerMetafileImporterModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMetafileImporterModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerMetafileImporterModule::~qSlicerMetafileImporterModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerMetafileImporterModule::helpText()const
{
  return "This is a module for importing imageing and tracking information from a sequence metafile into node sequences. See more information in the  <a href=\"http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/Sequences\">online documentation</a>.";
}

//-----------------------------------------------------------------------------
QString qSlicerMetafileImporterModule::acknowledgementText()const
{
  return "This work was funded by CCO ACRU and OCAIRO grants.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMetafileImporterModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's), Matthew Holden (PerkLab, Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerMetafileImporterModule::icon()const
{
  return QIcon(":/Icons/MetafileImporter.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMetafileImporterModule::categories() const
{
  return QStringList() << "Sequences";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMetafileImporterModule::dependencies() const
{
  return QStringList() << "Sequences";
}

//-----------------------------------------------------------------------------
void qSlicerMetafileImporterModule::setup()
{
  this->Superclass::setup();

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  vtkSlicerMetafileImporterLogic* metafileImporterLogic = vtkSlicerMetafileImporterLogic::SafeDownCast( this->logic() );
  qSlicerAbstractCoreModule* sequencesModule = app->moduleManager()->module( "Sequences" );  
  if (sequencesModule)
  {
    vtkSlicerSequencesLogic* sequencesLogic = vtkSlicerSequencesLogic::SafeDownCast(sequencesModule->logic());
    metafileImporterLogic->SetSequencesLogic(sequencesLogic);
  }
  else
  {
    qWarning("Could not register Sequences reader, as Sequences module is not registered.");
  }

  // Register the IO
  app->coreIOManager()->registerIO( new qSlicerMetafileReader( metafileImporterLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerNodeWriter( "MetafileImporter", QString( "SequenceMetafile" ), QStringList(), true, this ) );
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerMetafileImporterModule
::createWidgetRepresentation()
{
  return new qSlicerMetafileImporterModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMetafileImporterModule::createLogic()
{
  return vtkSlicerMetafileImporterLogic::New();
}
