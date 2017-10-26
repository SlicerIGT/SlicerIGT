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

// CollectPoints Logic includes
#include <vtkSlicerCollectPointsLogic.h>

// CollectPoints includes
#include "qSlicerCollectPointsModule.h"
#include "qSlicerCollectPointsModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerCollectPointsModule, qSlicerCollectPointsModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CollectPoints
class qSlicerCollectPointsModulePrivate
{
public:
  qSlicerCollectPointsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCollectPointsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCollectPointsModulePrivate::qSlicerCollectPointsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCollectPointsModule methods

//-----------------------------------------------------------------------------
qSlicerCollectPointsModule::qSlicerCollectPointsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCollectPointsModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollectPointsModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerCollectPointsModule::~qSlicerCollectPointsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerCollectPointsModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerCollectPointsModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollectPointsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Thomas Vaughan (Queen's University)");
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  moduleContributors << QString("Franklin King (Queen's University)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCollectPointsModule::icon()const
{
  return QIcon(":/Icons/CollectPoints.png");
}

//-----------------------------------------------------------------------------
void qSlicerCollectPointsModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerCollectPointsModule::createWidgetRepresentation()
{
  return new qSlicerCollectPointsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCollectPointsModule::createLogic()
{
  return vtkSlicerCollectPointsLogic::New();
}
