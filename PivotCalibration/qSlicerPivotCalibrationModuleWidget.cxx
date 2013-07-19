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
#include <QDebug>
#include <QMessageBox>

// SlicerQt includes
#include "qSlicerPivotCalibrationModuleWidget.h"
#include "ui_qSlicerPivotCalibrationModule.h"

#include "vtkSlicerPivotCalibrationLogic.h"

#include <vtkNew.h>
#include <vtkCommand.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPivotCalibrationModuleWidgetPrivate: public Ui_qSlicerPivotCalibrationModule
{
  Q_DECLARE_PUBLIC( qSlicerPivotCalibrationModuleWidget );
protected:
  qSlicerPivotCalibrationModuleWidget* const q_ptr;
public:
  qSlicerPivotCalibrationModuleWidgetPrivate( qSlicerPivotCalibrationModuleWidget& object );
  vtkSlicerPivotCalibrationLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerPivotCalibrationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidgetPrivate::qSlicerPivotCalibrationModuleWidgetPrivate( qSlicerPivotCalibrationModuleWidget& object )
 : q_ptr( &object )
{
}


vtkSlicerPivotCalibrationLogic* qSlicerPivotCalibrationModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerPivotCalibrationModuleWidget );
  return vtkSlicerPivotCalibrationLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerPivotCalibrationModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidget::qSlicerPivotCalibrationModuleWidget(QWidget* _parent) : Superclass( _parent ) , d_ptr( new qSlicerPivotCalibrationModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidget::~qSlicerPivotCalibrationModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::enter()
{
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::initializeObserver(vtkMRMLNode* node)
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  
  d->logic()->AddSample(node);
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setup()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  d->setupUi(this);
  

  this->Superclass::setup();
  
  connect(d->InputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(initializeObserver(vtkMRMLNode*)));
  //connect( d->addDataButton, SIGNAL( clicked() ), this, SLOT( onAddData() ) );
  //connect( d->loadSceneButton, SIGNAL( clicked() ), this, SLOT( onLoadScene() ) );
  
}

