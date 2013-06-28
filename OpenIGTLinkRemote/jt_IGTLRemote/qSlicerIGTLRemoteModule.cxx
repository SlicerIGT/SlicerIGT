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

// IGTLRemote Logic includes
#include <vtkSlicerIGTLRemoteLogic.h>

// IGTLRemote includes
#include "qSlicerIGTLRemoteModule.h"
#include "qSlicerIGTLRemoteModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerIGTLRemoteModule, qSlicerIGTLRemoteModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IGTLRemote
class qSlicerIGTLRemoteModulePrivate
{
public:
  qSlicerIGTLRemoteModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModulePrivate::qSlicerIGTLRemoteModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModule methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModule::qSlicerIGTLRemoteModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerIGTLRemoteModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerIGTLRemoteModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModule::~qSlicerIGTLRemoteModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerIGTLRemoteModule::helpText()const
{
  QString help = 
    "This module query a list of data available in the remote"
    "software connected with the 3D Slicer through OpenIGTLink,"
    "receives the list, and display in the table interface.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerIGTLRemoteModule::acknowledgementText()const
{
  return "This work was supported by NIH National Center for Image Guided Therapy, and "
    "National Alliance for Medical Image Computing.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerIGTLRemoteModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Junichi Tokuda (Brigham and Women's Hospital)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerIGTLRemoteModule::icon()const
{
  return QIcon(":/Icons/IGTLRemote.png");
}

//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerIGTLRemoteModule::createWidgetRepresentation()
{
  return new qSlicerIGTLRemoteModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerIGTLRemoteModule::createLogic()
{
  return vtkSlicerIGTLRemoteLogic::New();
}
