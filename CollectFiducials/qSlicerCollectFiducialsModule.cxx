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

// CollectFiducials Logic includes
#include <vtkSlicerCollectFiducialsLogic.h>

// CollectFiducials includes
#include "qSlicerCollectFiducialsModule.h"
#include "qSlicerCollectFiducialsModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerCollectFiducialsModule, qSlicerCollectFiducialsModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CollectFiducials
class qSlicerCollectFiducialsModulePrivate
{
public:
  qSlicerCollectFiducialsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCollectFiducialsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCollectFiducialsModulePrivate::qSlicerCollectFiducialsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCollectFiducialsModule methods

//-----------------------------------------------------------------------------
qSlicerCollectFiducialsModule::qSlicerCollectFiducialsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCollectFiducialsModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollectFiducialsModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerCollectFiducialsModule::~qSlicerCollectFiducialsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerCollectFiducialsModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerCollectFiducialsModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCollectFiducialsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Tamas Ungi (Queen's University), Franklin King (Queen's University)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCollectFiducialsModule::icon()const
{
  return QIcon(":/Icons/CollectFiducials.png");
}

//-----------------------------------------------------------------------------
void qSlicerCollectFiducialsModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerCollectFiducialsModule::createWidgetRepresentation()
{
  return new qSlicerCollectFiducialsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCollectFiducialsModule::createLogic()
{
  return vtkSlicerCollectFiducialsLogic::New();
}
