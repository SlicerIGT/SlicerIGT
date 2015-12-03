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
#include <QDir>
#include <QPointer>
#include <QSound>
#include <QTime>
#include <QTimer>
#include <QtPlugin>

#include "qSlicerApplication.h"

// BreachWarning Logic includes
#include <vtkSlicerBreachWarningLogic.h>

// BreachWarning includes
#include "qSlicerBreachWarningModule.h"
#include "qSlicerBreachWarningModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerBreachWarningModule, qSlicerBreachWarningModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_BreachWarning
class qSlicerBreachWarningModulePrivate
{
public:
  qSlicerBreachWarningModulePrivate();

  vtkSlicerBreachWarningLogic* ObservedLogic; // should be the same as logic(), it is used for adding/removing observer safely
  QTimer UpdateWarningSoundTimer;
  QPointer<QSound> WarningSound;
  double WarningSoundPeriodSec;
};

//-----------------------------------------------------------------------------
// qSlicerBreachWarningModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerBreachWarningModulePrivate::qSlicerBreachWarningModulePrivate()
: ObservedLogic(NULL)
{
}

//-----------------------------------------------------------------------------
// qSlicerBreachWarningModule methods

//-----------------------------------------------------------------------------
qSlicerBreachWarningModule::qSlicerBreachWarningModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerBreachWarningModulePrivate)
{
  Q_D(qSlicerBreachWarningModule);
  d->WarningSoundPeriodSec = 0.5;
}

//-----------------------------------------------------------------------------
QStringList qSlicerBreachWarningModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerBreachWarningModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
qSlicerBreachWarningModule::~qSlicerBreachWarningModule()
{
  Q_D(qSlicerBreachWarningModule);
  if (!d->WarningSound.isNull())
  {
    d->WarningSound->stop();
  }
  disconnect(&d->UpdateWarningSoundTimer, SIGNAL(timeout()), this, SLOT(updateWarningSound()));
  this->qvtkReconnect(d->ObservedLogic, NULL, vtkCommand::ModifiedEvent, this, SLOT(updateWarningSound()));
  d->ObservedLogic = NULL;
}

//-----------------------------------------------------------------------------
QString qSlicerBreachWarningModule::helpText()const
{
  return "This module can alert the user by color change and sound signal if a tool enters a restricted area. The restricted area is defined by a surface model, the tool position is defined by a linear transform. For help on how to use this module visit: <a href='http://www.slicerigt.org/'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerBreachWarningModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerBreachWarningModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Matthew Holden (Queen's University)");
  moduleContributors << QString("Jaime Garcia Guevara (Queen's University)");
  moduleContributors << QString("Andras Lasso (Queen's University)");
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  moduleContributors << QString("Mikael Brudfors (UCL)");  
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerBreachWarningModule::icon()const
{
  return QIcon(":/Icons/BreachWarning.png");
}

//-----------------------------------------------------------------------------
void qSlicerBreachWarningModule::setup()
{
  Q_D(qSlicerBreachWarningModule);

  this->Superclass::setup();

  connect(qSlicerApplication::application(), SIGNAL(lastWindowClosed()), this, SLOT(stopSound()));  

  vtkSlicerBreachWarningLogic* moduleLogic = vtkSlicerBreachWarningLogic::SafeDownCast(logic());

  if (d->WarningSound == NULL)
  {
    d->WarningSound = new QSound( QDir::toNativeSeparators( QString::fromStdString( moduleLogic->GetModuleShareDirectory()+"/alarm.wav" ) ) );
  }

  this->qvtkReconnect(d->ObservedLogic, moduleLogic, vtkCommand::ModifiedEvent, this, SLOT(updateWarningSound()));
  d->ObservedLogic = moduleLogic;

  d->UpdateWarningSoundTimer.setSingleShot(true);
  connect(&d->UpdateWarningSoundTimer, SIGNAL(timeout()), this, SLOT(updateWarningSound()));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerBreachWarningModule::createWidgetRepresentation()
{
  return new qSlicerBreachWarningModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerBreachWarningModule::createLogic()
{
  return vtkSlicerBreachWarningLogic::New();
}

//------------------------------------------------------------------------------
void qSlicerBreachWarningModule::updateWarningSound()
{
  Q_D(qSlicerBreachWarningModule);
  if (d->WarningSound.isNull())
  {
    qWarning("Warning sound object is invalid");
    return;
  }
  if (d->ObservedLogic==NULL)
  {
    qWarning("ObservedLogic is invalid");
    return;
  }
  bool warningSoundShouldPlay = d->ObservedLogic->GetWarningSoundPlaying();
  if (warningSoundShouldPlay)
  {
    d->WarningSound->setLoops(1);
    d->WarningSound->play();
  }
  else
  {
    d->WarningSound->stop();
  }
  d->UpdateWarningSoundTimer.start(warningSoundPeriodSec()*1000);
}


//------------------------------------------------------------------------------
void qSlicerBreachWarningModule::stopSound()
{
  Q_D(qSlicerBreachWarningModule);
  if (!d->WarningSound.isNull())
  {
    d->WarningSound->stop();
    d->WarningSound=NULL;
  }
}

//------------------------------------------------------------------------------
void qSlicerBreachWarningModule::setWarningSoundPeriodSec(double periodTimeSec)
{
  Q_D(qSlicerBreachWarningModule);
  d->WarningSoundPeriodSec = periodTimeSec;
}

//------------------------------------------------------------------------------
double qSlicerBreachWarningModule::warningSoundPeriodSec()
{
  Q_D(qSlicerBreachWarningModule);
  return d->WarningSoundPeriodSec;
}
