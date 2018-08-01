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

// PlusSupport Logic includes
#include <vtkSlicerPlusSupportLogic.h>

// PlusSupport includes
#include "qSlicerPlusSupportModule.h"
#include "qSlicerPlusSupportModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  #include <QtPlugin>
  Q_EXPORT_PLUGIN2(qSlicerPlusSupportModule, qSlicerPlusSupportModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPlusSupportModulePrivate
{
public:
  qSlicerPlusSupportModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPlusSupportModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPlusSupportModulePrivate::qSlicerPlusSupportModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPlusSupportModule methods

//-----------------------------------------------------------------------------
qSlicerPlusSupportModule::qSlicerPlusSupportModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPlusSupportModulePrivate)
{
  this->setWidgetRepresentationCreationEnabled(false);
}

//-----------------------------------------------------------------------------
qSlicerPlusSupportModule::~qSlicerPlusSupportModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPlusSupportModule::helpText() const
{
  return "This is a loadable module that exposes Plus related tools for use in Slicer modules";
}

//-----------------------------------------------------------------------------
QString qSlicerPlusSupportModule::acknowledgementText() const
{
  return "This work was supported by the VASST Lab, Robarts Research Institute, Western University.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlusSupportModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Adam Rankin (Robarts Research Institute.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPlusSupportModule::icon() const
{
  return QIcon(":/Icons/PlusSupport.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlusSupportModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlusSupportModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerPlusSupportModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerPlusSupportModule
::createWidgetRepresentation()
{
  return new qSlicerPlusSupportModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPlusSupportModule::createLogic()
{
  return vtkSlicerPlusSupportLogic::New();
}
