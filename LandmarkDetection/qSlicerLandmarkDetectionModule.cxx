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
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// LandmarkDetection Logic includes
#include <vtkSlicerLandmarkDetectionLogic.h>

// LandmarkDetection includes
#include "qSlicerLandmarkDetectionModule.h"
#include "qSlicerLandmarkDetectionModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLandmarkDetectionModulePrivate
{
public:
  qSlicerLandmarkDetectionModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLandmarkDetectionModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerLandmarkDetectionModulePrivate::qSlicerLandmarkDetectionModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLandmarkDetectionModule methods

//-----------------------------------------------------------------------------
qSlicerLandmarkDetectionModule::qSlicerLandmarkDetectionModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerLandmarkDetectionModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLandmarkDetectionModule::~qSlicerLandmarkDetectionModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerLandmarkDetectionModule::helpText() const
{
  return "";
}

//-----------------------------------------------------------------------------
QString qSlicerLandmarkDetectionModule::acknowledgementText() const
{
  return "";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLandmarkDetectionModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerLandmarkDetectionModule::icon() const
{
  return QIcon(":/Icons/LandmarkDetection.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerLandmarkDetectionModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLandmarkDetectionModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerLandmarkDetectionModule
::createWidgetRepresentation()
{
  return new qSlicerLandmarkDetectionModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerLandmarkDetectionModule::createLogic()
{
  return vtkSlicerLandmarkDetectionLogic::New();
}
