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

// Slicer includes
#include "qSlicerCoreApplication.h"

// Qt includes
#include <QtPlugin>
#include <QTimer>

// TransformProcessor Logic includes
#include <vtkSlicerTransformProcessorLogic.h>
#include <vtkMRMLTransformProcessorNode.h>

// TransformProcessor includes
#include "qSlicerTransformProcessorModule.h"
#include "qSlicerTransformProcessorModuleWidget.h"

static const double UPDATE_OUTPUTS_PERIOD_SEC = 0.033; // about 30 fps update rate

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerTransformProcessorModule, qSlicerTransformProcessorModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformProcessor
class qSlicerTransformProcessorModulePrivate
{
public:
  qSlicerTransformProcessorModulePrivate();

  QTimer UpdateAllOutputsTimer;
};

//-----------------------------------------------------------------------------
// qSlicerTransformProcessorModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModulePrivate::qSlicerTransformProcessorModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTransformProcessorModule methods

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModule::qSlicerTransformProcessorModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformProcessorModulePrivate)
{
  Q_D(qSlicerTransformProcessorModule);
  connect(&d->UpdateAllOutputsTimer, SIGNAL(timeout()), this, SLOT(updateAllOutputs()));
  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
  if (scene)
  {
    // Need to listen for any new watchdog nodes being added to start/stop timer
    this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
    this->qvtkConnect(scene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
  }
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformProcessorModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerTransformProcessorModule::~qSlicerTransformProcessorModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformProcessorModule::helpText()const
{
  return "This module allows combining, inverting, stabilizing, termporal smoothing of transforms in real-time."
    " For more information, visit <a href='https://github.com/SlicerIGT/SlicerIGT/#user-documentation'>SlicerIGT project website</a>.";
}

//-----------------------------------------------------------------------------
QString qSlicerTransformProcessorModule::acknowledgementText()const
{
  return "This work was partially funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO),"
    " and by National Institute of Health (grants 5P01CA067165, 5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377, 5R42CA137886, 8P41EB015898).";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformProcessorModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Franklin King (PerkLab, Queen's University)")
    << QString("Thomas Vaughan (PerkLab, Queen's University)")
    << QString("Andras Lasso (PerkLab, Queen's University)")
    << QString("Tamas Ungi (PerkLab, Queen's University)")
    << QString("Laurent Chauvin (BWH)")
    << QString("Jayender Jagadeesan (BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTransformProcessorModule::icon()const
{
  return QIcon(":/Icons/TransformProcessor.png");
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTransformProcessorModule::createWidgetRepresentation()
{
  return new qSlicerTransformProcessorModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTransformProcessorModule::createLogic()
{
  return vtkSlicerTransformProcessorLogic::New();
}

// --------------------------------------------------------------------------
void qSlicerTransformProcessorModule::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerTransformProcessorModule);

  vtkMRMLTransformProcessorNode* processorNode = vtkMRMLTransformProcessorNode::SafeDownCast(node);
  if (processorNode)
  {
    // If the timer is not active
    if (!d->UpdateAllOutputsTimer.isActive())
    {
      d->UpdateAllOutputsTimer.start(UPDATE_OUTPUTS_PERIOD_SEC * 1000.0);
    }
  }
}

// --------------------------------------------------------------------------
void qSlicerTransformProcessorModule::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerTransformProcessorModule);

  vtkMRMLTransformProcessorNode* processorNode = vtkMRMLTransformProcessorNode::SafeDownCast(node);
  if (processorNode)
  {
    // If the timer is active
    if (d->UpdateAllOutputsTimer.isActive())
    {
      // Check if there is any other sequence browser node left in the Scene
      vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
      if (scene)
      {
        std::vector<vtkMRMLNode*> nodes;
        this->mrmlScene()->GetNodesByClass("vtkMRMLTransformProcessorNode", nodes);
        if (nodes.size() == 0)
        {
          // The last sequence browser was removed
          d->UpdateAllOutputsTimer.stop();
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformProcessorModule::updateAllOutputs()
{
  Q_D(qSlicerTransformProcessorModule);
  vtkSlicerTransformProcessorLogic* processorLogic = vtkSlicerTransformProcessorLogic::SafeDownCast(this->Superclass::logic());
  if (!processorLogic)
  {
    return;
  }
  processorLogic->UpdateAllOutputs();
}
