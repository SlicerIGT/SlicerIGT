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

// TransformFusion Logic includes
#include <vtkSlicerTransformFusionLogic.h>

// TransformFusion includes
#include "qSlicerTransformFusionModule.h"
#include "qSlicerTransformFusionModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTransformFusionModule, qSlicerTransformFusionModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformFusion
class qSlicerTransformFusionModulePrivate
{
public:
  qSlicerTransformFusionModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTransformFusionModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTransformFusionModulePrivate::qSlicerTransformFusionModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTransformFusionModule methods

//-----------------------------------------------------------------------------
qSlicerTransformFusionModule::qSlicerTransformFusionModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformFusionModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformFusionModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerTransformFusionModule::~qSlicerTransformFusionModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformFusionModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerTransformFusionModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformFusionModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Franklin King (Queen's University), Tamas Ungi (Queen's University)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTransformFusionModule::icon()const
{
  return QIcon(":/Icons/TransformFusion.png");
}

//-----------------------------------------------------------------------------
void qSlicerTransformFusionModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTransformFusionModule::createWidgetRepresentation()
{
  return new qSlicerTransformFusionModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTransformFusionModule::createLogic()
{
  return vtkSlicerTransformFusionLogic::New();
}
