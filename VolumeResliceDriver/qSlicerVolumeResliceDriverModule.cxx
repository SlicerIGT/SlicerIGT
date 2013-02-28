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

// VolumeResliceDriver Logic includes
#include <vtkSlicerVolumeResliceDriverLogic.h>

// VolumeResliceDriver includes
#include "qSlicerVolumeResliceDriverModule.h"
#include "qSlicerVolumeResliceDriverModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumeResliceDriverModule, qSlicerVolumeResliceDriverModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeResliceDriver
class qSlicerVolumeResliceDriverModulePrivate
{
public:
  qSlicerVolumeResliceDriverModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVolumeResliceDriverModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeResliceDriverModulePrivate::qSlicerVolumeResliceDriverModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVolumeResliceDriverModule methods

//-----------------------------------------------------------------------------
qSlicerVolumeResliceDriverModule::qSlicerVolumeResliceDriverModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumeResliceDriverModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeResliceDriverModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerVolumeResliceDriverModule::~qSlicerVolumeResliceDriverModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeResliceDriverModule::helpText()const
{
  QString help = 
    "This module allows to set reslicing planes by using linear transforms nodes"
    "or positions/orientations in image nodes.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeResliceDriverModule::acknowledgementText()const
{
  return "This work was supported by NIH National Center for Image Guided Therapy, and "
    "National Alliance for Medical Image Computing.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeResliceDriverModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Junichi Tokuda (Brigham and Women's Hospital), Tamas Ungi (Queen's University)");
  // moduleContributors << QString("Richard Roe (Other organization Inc.)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVolumeResliceDriverModule::icon()const
{
  return QIcon(":/Icons/VolumeResliceDriver.png");
}

//-----------------------------------------------------------------------------
void qSlicerVolumeResliceDriverModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerVolumeResliceDriverModule::createWidgetRepresentation()
{
  return new qSlicerVolumeResliceDriverModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVolumeResliceDriverModule::createLogic()
{
  return vtkSlicerVolumeResliceDriverLogic::New();
}
