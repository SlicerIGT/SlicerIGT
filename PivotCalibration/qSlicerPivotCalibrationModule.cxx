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

// ExtensionTemplate Logic includes
#include <vtkSlicerPivotCalibrationLogic.h>

// ExtensionTemplate includes
#include "qSlicerPivotCalibrationModule.h"
#include "qSlicerPivotCalibrationModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerPivotCalibrationModule, qSlicerPivotCalibrationModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPivotCalibrationModulePrivate
{
public:
  qSlicerPivotCalibrationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPivotCalibrationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModulePrivate::qSlicerPivotCalibrationModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPivotCalibrationModule methods

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModule::qSlicerPivotCalibrationModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPivotCalibrationModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModule::~qSlicerPivotCalibrationModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPivotCalibrationModule::helpText()const
{
  return "";
}

//-----------------------------------------------------------------------------
QString qSlicerPivotCalibrationModule::acknowledgementText()const
{
  return "";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPivotCalibrationModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString( "Franklin King (Queen's University), Tamas Ungi (Queen's University)" );
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPivotCalibrationModule::icon()const
{
  return QIcon(":/Icons/PivotCalibration.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPivotCalibrationModule::categories() const
{
  return QStringList() << "PivotCalibration";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPivotCalibrationModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerPivotCalibrationModule::createWidgetRepresentation()
{
  return new qSlicerPivotCalibrationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPivotCalibrationModule::createLogic()
{
  return vtkSlicerPivotCalibrationLogic::New();
}
