/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

#include "qSlicerCoreApplication.h"

// VolumeReconstruction module includes
#include "qSlicerVolumeReconstructionModule.h"
#include "qSlicerVolumeReconstructionModuleWidget.h"

// VolumeReconstruction Logic includes
#include "vtkSlicerVolumeReconstructionLogic.h"

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// vtkAddon include
#include <vtkStreamingVolumeCodecFactory.h>

//-----------------------------------------------------------------------------
#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  #include <QtPlugin>
  Q_EXPORT_PLUGIN2(qSlicerVolumeReconstructionModule, qSlicerVolumeReconstructionModule);
#endif
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeReconstruction
class qSlicerVolumeReconstructionModulePrivate
{
public:
  qSlicerVolumeReconstructionModulePrivate();
  ~qSlicerVolumeReconstructionModulePrivate();

  QTimer LiveVolumeReconstructionTimer;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeReconstructionModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModulePrivate::qSlicerVolumeReconstructionModulePrivate()
{
  this->LiveVolumeReconstructionTimer.setSingleShot(false);
  this->LiveVolumeReconstructionTimer.setInterval(100);
  this->LiveVolumeReconstructionTimer.start();
}

qSlicerVolumeReconstructionModulePrivate::~qSlicerVolumeReconstructionModulePrivate()
{;
}

//-----------------------------------------------------------------------------
// qSlicerVolumeReconstructionModule methods

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModule::qSlicerVolumeReconstructionModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumeReconstructionModulePrivate)
{
  Q_D(qSlicerVolumeReconstructionModule);

  connect(&d->LiveVolumeReconstructionTimer, SIGNAL(timeout()), this, SLOT(updateLiveVolumeReconstruction()));
}

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModule::~qSlicerVolumeReconstructionModule()
{
  Q_D(qSlicerVolumeReconstructionModule);
  disconnect(&d->LiveVolumeReconstructionTimer, SIGNAL(timeout()), this, SLOT(updateLiveVolumeReconstruction()));
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeReconstructionModule::helpText()const
{
  return tr("This is a module for reconstructing image volumes from sequences. If you have questions, or encounter an problem, submit an issue on the <a href=\"https://github.com/SlicerIGT/SlicerIGT\">GitHub page</a>.");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeReconstructionModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's University)");
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's University)");
  return moduleContributors;
}


//-----------------------------------------------------------------------------
QString qSlicerVolumeReconstructionModule::acknowledgementText()const
{
  return tr("This module was developed through support from CANARIE's Research Software Program, and Cancer Care Ontario.");
}

//-----------------------------------------------------------------------------
QIcon qSlicerVolumeReconstructionModule::icon()const
{
  return QIcon(":/Icons/VolumeReconstruction.png");
}


//-----------------------------------------------------------------------------
QStringList qSlicerVolumeReconstructionModule::categories() const
{
  return QStringList() << qSlicerAbstractCoreModule::tr("IGT");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeReconstructionModule::dependencies() const
{
  return QStringList() << tr("Sequences");
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVolumeReconstructionModule::createWidgetRepresentation()
{
  return new qSlicerVolumeReconstructionModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVolumeReconstructionModule::createLogic()
{
  return vtkSlicerVolumeReconstructionLogic::New();
}


//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModule::updateLiveVolumeReconstruction()
{
  vtkSlicerVolumeReconstructionLogic* logic = vtkSlicerVolumeReconstructionLogic::SafeDownCast(this->logic());
  if (!logic)
  {
    return;
  }
  logic->UpdateLiveVolumeReconstruction();
}
