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
#include <QApplication>
#include <QTimer>

// SlicerQt includes
#include "qSlicerPivotCalibrationModuleWidget.h"
#include "ui_qSlicerPivotCalibrationModule.h"

#include "vtkSlicerPivotCalibrationLogic.h"

#include <vtkNew.h>
#include <vtkCommand.h>

#include <vtkMRMLLinearTransformNode.h>

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
  this->delayTimer = new QTimer();
  delayTimer->setSingleShot(false);
  delayTimer->setInterval(1000);
  this->delayCount = 5;
  
  this->samplingTimer = new QTimer();
  samplingTimer->setSingleShot(false);
  samplingTimer->setInterval(1000);
  this->samplingCount = 5;
  
  this->timerSetting = 5;
}

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidget::~qSlicerPivotCalibrationModuleWidget()
{
  delete this->delayTimer;
  delete this->samplingTimer;
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
  
  d->logic()->InitializeObserver(node);
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setup()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  d->setupUi(this);
  

  this->Superclass::setup();
  
  connect(delayTimer, SIGNAL( timeout() ), this, SLOT( onDelayTimeout() ));
  connect(samplingTimer, SIGNAL( timeout() ), this, SLOT( onSamplingTimeout() ));  
  
  connect( d->InputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(initializeObserver(vtkMRMLNode*)) );
  
  connect( d->startButton, SIGNAL( clicked() ), this, SLOT( onStartButton() ) );
  //connect( d->stopButton, SIGNAL( clicked() ), this, SLOT( onStop() ) );
  
  connect( d->timerEdit, SIGNAL( valueChanged(double) ), this, SLOT( setTimer(double) ) );
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onStartButton()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  std::stringstream ss;
  ss << this->delayCount << " seconds until start";
  d->CountdownLabel->setText(ss.str().c_str());  
  
  delayTimer->start();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onDelayTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  std::stringstream ss1;
  
  this->delayCount -= 1;
  ss1 << this->delayCount << " seconds until start";
  d->CountdownLabel->setText(ss1.str().c_str());

  if (this->delayCount <= 0)
  {
    std::stringstream ss2;
    this->samplingCount = this->timerSetting;
    ss2 << "Sampling time left: " << this->samplingCount;
    d->CountdownLabel->setText(ss2.str().c_str());
    
    this->delayTimer->stop();
    d->logic()->setRecordingState(true);
    this->samplingTimer->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSamplingTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  
  std::stringstream ss;

  std::cout<<this->samplingCount<<std::endl;
  
  this->samplingCount -= 1;
  ss << "Sampling time left: " << this->samplingCount;
  d->CountdownLabel->setText(ss.str().c_str());  
  
  if (this->samplingCount <= 0)
  {
    d->CountdownLabel->setText("Sampling complete");
    
    this->samplingTimer->stop();
    this->onStop();
    this->delayCount = 5;
  }  
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onStop()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  
  d->logic()->setRecordingState(false);
  d->logic()->ComputeCalibration();
  
  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  vtkMatrix4x4* outputMatrix = outputTransform->GetMatrixTransformToParent();
  outputMatrix->SetElement(0,3,d->logic()->Translation[0]);
  outputMatrix->SetElement(1,3,d->logic()->Translation[1]);
  outputMatrix->SetElement(2,3,d->logic()->Translation[2]);
  
  std::stringstream ss;
  ss << d->logic()->RMSE;
  d->rmseLabel->setText(ss.str().c_str());
  
  d->logic()->ClearSamples();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setTimer(double time)
{
  this->timerSetting = (int)time;
}

