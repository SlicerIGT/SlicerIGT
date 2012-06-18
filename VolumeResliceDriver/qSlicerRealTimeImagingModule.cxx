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

// RealTimeImaging Logic includes
#include <vtkSlicerRealTimeImagingLogic.h>

// RealTimeImaging includes
#include "qSlicerRealTimeImagingModule.h"
#include "qSlicerRealTimeImagingModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerRealTimeImagingModule, qSlicerRealTimeImagingModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RealTimeImaging
class qSlicerRealTimeImagingModulePrivate
{
public:
  qSlicerRealTimeImagingModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerRealTimeImagingModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerRealTimeImagingModulePrivate::qSlicerRealTimeImagingModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerRealTimeImagingModule methods

//-----------------------------------------------------------------------------
qSlicerRealTimeImagingModule::qSlicerRealTimeImagingModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerRealTimeImagingModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerRealTimeImagingModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerRealTimeImagingModule::~qSlicerRealTimeImagingModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerRealTimeImagingModule::helpText()const
{
  QString help = 
    "This template module is meant to be used with the"
    "with the ModuleWizard.py script distributed with the"
    "Slicer source code (starting with version 4)."
    "Developers can generate their own source code using the"
    "wizard and then customize it to fit their needs.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerRealTimeImagingModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community...";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRealTimeImagingModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (Organization Inc.)");
  // moduleContributors << QString("Richard Roe (Other organization Inc.)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerRealTimeImagingModule::icon()const
{
  return QIcon(":/Icons/RealTimeImaging.png");
}

//-----------------------------------------------------------------------------
void qSlicerRealTimeImagingModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerRealTimeImagingModule::createWidgetRepresentation()
{
  return new qSlicerRealTimeImagingModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerRealTimeImagingModule::createLogic()
{
  return vtkSlicerRealTimeImagingLogic::New();
}
