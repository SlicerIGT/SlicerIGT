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
#include <QTimer>
#include <QtPlugin>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerApplication.h"
#include <vtkSlicerVersionConfigure.h> // For Slicer_VERSION_MAJOR, Slicer_VERSION_MINOR

// Watchdog Logic includes
#include <vtkSlicerWatchdogLogic.h>
#include "vtkMRMLSliceViewDisplayableManagerFactory.h"
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h" 

// Watchdog includes
#include "qSlicerWatchdogModule.h"
#include "qSlicerWatchdogModuleWidget.h"
#include "vtkMRMLWatchdogDisplayableManager.h"

#include "vtkMRMLWatchdogNode.h"

static const double UPDATE_WATCHDOG_NODES_PERIOD_SEC = 0.2;

// DisplayableManager initialization
#if Slicer_VERSION_MAJOR >= 5 || (Slicer_VERSION_MAJOR == 4 && Slicer_VERSION_MINOR >= 9)
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkSlicerWatchdogModuleMRMLDisplayableManager)
#endif

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerWatchdogModule, qSlicerWatchdogModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Watchdog
class qSlicerWatchdogModulePrivate
{
public:
  qSlicerWatchdogModulePrivate();
  ~qSlicerWatchdogModulePrivate();
  
  QTimer UpdateAllWatchdogNodesTimer;
  QPointer<QSound> WatchedNodeBecomeUpToDateSound;
  QPointer<QSound> WatchedNodeBecomeOutdatedSound;
};

//-----------------------------------------------------------------------------
// qSlicerWatchdogModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModulePrivate::qSlicerWatchdogModulePrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerWatchdogModulePrivate::~qSlicerWatchdogModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerWatchdogModule methods

//-----------------------------------------------------------------------------
qSlicerWatchdogModule::qSlicerWatchdogModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWatchdogModulePrivate)
{
  Q_D(qSlicerWatchdogModule);
  connect(&d->UpdateAllWatchdogNodesTimer, SIGNAL(timeout()), this, SLOT(updateAllWatchdogNodes()));
  vtkMRMLScene * scene = qSlicerCoreApplication::application()->mrmlScene();
  if (scene)
    {
    // Need to listen for any new watchdog nodes being added to start/stop timer
    this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*,vtkObject*)));
    this->qvtkConnect(scene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*,vtkObject*)));
    }
}

//-----------------------------------------------------------------------------
qSlicerWatchdogModule::~qSlicerWatchdogModule()
{
  Q_D(qSlicerWatchdogModule);
  if (!d->WatchedNodeBecomeUpToDateSound.isNull())
  {
    d->WatchedNodeBecomeUpToDateSound->stop();
  }
  if (!d->WatchedNodeBecomeOutdatedSound.isNull())
  {
    d->WatchedNodeBecomeOutdatedSound->stop();
  }
}

//-----------------------------------------------------------------------------
QString qSlicerWatchdogModule::helpText() const
{
  return "Displays warning if selected transforms are not continuously updated."
    " It is useful for detecting tracking errors, such as occluded marker or network connection error."
    " For help on how to use this module visit: <a href='http://www.slicerigt.org/'>SlicerIGT website</a>.";
}

//-----------------------------------------------------------------------------
QString qSlicerWatchdogModule::acknowledgementText() const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWatchdogModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jaime Garcia-Guevara (Queen's University)");
  moduleContributors << QString("Andras Lasso (Queen's University)");
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerWatchdogModule::icon() const
{
  return QIcon(":/Icons/Watchdog.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWatchdogModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWatchdogModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModule::setup()
{
  Q_D(qSlicerWatchdogModule);
  this->Superclass::setup();
  
  // Use the displayable manager class to make sure the the containing library is loaded
  vtkSmartPointer<vtkMRMLWatchdogDisplayableManager> dm=vtkSmartPointer<vtkMRMLWatchdogDisplayableManager>::New();

  connect(qSlicerApplication::application(), SIGNAL(lastWindowClosed()), this, SLOT(stopSound()));  

  vtkSlicerWatchdogLogic* watchdogLogic = vtkSlicerWatchdogLogic::SafeDownCast(this->logic());
  if (watchdogLogic)
  {
    if (d->WatchedNodeBecomeUpToDateSound == NULL)
    {
      d->WatchedNodeBecomeUpToDateSound = new QSound( QDir::toNativeSeparators( QString::fromStdString( watchdogLogic->GetModuleShareDirectory()+"/WatchedNodeUpToDate.wav" ) ) );
    }
    if (d->WatchedNodeBecomeOutdatedSound == NULL)
    {
      d->WatchedNodeBecomeOutdatedSound = new QSound( QDir::toNativeSeparators( QString::fromStdString( watchdogLogic->GetModuleShareDirectory()+"/WatchedNodeOutdated.wav" ) ) );
    }
  }
  else
  {
    qWarning("vtkSlicerWatchdogLogic is not available");
  }

  // Register displayable managers
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLWatchdogDisplayableManager");
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLWatchdogDisplayableManager"); 
}

//------------------------------------------------------------------------------
void qSlicerWatchdogModule::stopSound()
{
  Q_D(qSlicerWatchdogModule);
  if (!d->WatchedNodeBecomeUpToDateSound.isNull())
  {
    d->WatchedNodeBecomeUpToDateSound->stop();
    d->WatchedNodeBecomeUpToDateSound=NULL;
  }
  if (!d->WatchedNodeBecomeOutdatedSound.isNull())
  {
    d->WatchedNodeBecomeOutdatedSound->stop();
    d->WatchedNodeBecomeOutdatedSound=NULL;
  }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  this->Superclass::setMRMLScene(_mrmlScene);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerWatchdogModule::createWidgetRepresentation()
{
  Q_D(qSlicerWatchdogModule);
  qSlicerWatchdogModuleWidget * watchdogWidget = new qSlicerWatchdogModuleWidget;
  return watchdogWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerWatchdogModule::createLogic()
{
  return vtkSlicerWatchdogLogic::New();
}

// --------------------------------------------------------------------------
void qSlicerWatchdogModule::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerWatchdogModule);

  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast(node);
  if (watchdogNode)
    {
    // If the timer is not active
    if (!d->UpdateAllWatchdogNodesTimer.isActive())
      {
      d->UpdateAllWatchdogNodesTimer.start(UPDATE_WATCHDOG_NODES_PERIOD_SEC*1000.0);
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerWatchdogModule::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerWatchdogModule);

  vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast(node);
  if (watchdogNode)
    {
    // If the timer is active
    if (d->UpdateAllWatchdogNodesTimer.isActive())
      {
      // Check if there is any other sequence browser node left in the Scene
      vtkMRMLScene * scene = qSlicerCoreApplication::application()->mrmlScene();
      if (scene)
        {
        std::vector<vtkMRMLNode *> nodes;
        this->mrmlScene()->GetNodesByClass("vtkMRMLWatchdogNode", nodes);
        if (nodes.size() == 0)
          {
          // The last sequence browser was removed
          d->UpdateAllWatchdogNodesTimer.stop();
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerWatchdogModule::updateAllWatchdogNodes()
{
  Q_D(qSlicerWatchdogModule);
  vtkSlicerWatchdogLogic* watchdogLogic = vtkSlicerWatchdogLogic::SafeDownCast(this->Superclass::logic());
  if (!watchdogLogic)
    {
    return;
    }

  bool watchedNodeBecomeUpToDateSound=false;
  bool watchedNodeBecomeOutdatedSound=false;
  watchdogLogic->UpdateAllWatchdogNodes(watchedNodeBecomeUpToDateSound, watchedNodeBecomeOutdatedSound);

  // Play connected/disconnected sounds
  if (watchedNodeBecomeUpToDateSound && !d->WatchedNodeBecomeUpToDateSound.isNull())
  {
    d->WatchedNodeBecomeUpToDateSound->play();
  }
  if (watchedNodeBecomeOutdatedSound && !d->WatchedNodeBecomeOutdatedSound.isNull())
  {
    d->WatchedNodeBecomeOutdatedSound->play();
  }
}
