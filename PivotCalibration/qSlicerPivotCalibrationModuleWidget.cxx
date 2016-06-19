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
  this->startupDurationSec = 5;
  this->samplingDurationSec = 5;
  
  this->pivotStartupTimer = new QTimer();
  pivotStartupTimer->setSingleShot(false);
  pivotStartupTimer->setInterval(1000); // 1 sec
  this->pivotStartupRemainingTimerPeriodCount = 0;
  
  this->pivotSamplingTimer = new QTimer();
  pivotSamplingTimer->setSingleShot(false);
  pivotSamplingTimer->setInterval(1000); // 1 sec
  this->pivotSamplingRemainingTimerPeriodCount = 0;
  
  this->spinStartupTimer = new QTimer();
  spinStartupTimer->setSingleShot(false);
  spinStartupTimer->setInterval(1000); // 1 sec
  this->spinStartupRemainingTimerPeriodCount = 0;
  
  this->spinSamplingTimer = new QTimer();
  spinSamplingTimer->setSingleShot(false);
  spinSamplingTimer->setInterval(1000); // 1 sec
  this->spinSamplingRemainingTimerPeriodCount = 0;
}

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidget::~qSlicerPivotCalibrationModuleWidget()
{
  delete this->pivotStartupTimer;
  delete this->pivotSamplingTimer;
  delete this->spinStartupTimer;
  delete this->spinSamplingTimer;
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
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  d->logic()->SetAndObserveTransformNode( transformNode );
}


//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setup()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  d->setupUi(this);
  

  this->Superclass::setup();
  
  connect(pivotStartupTimer, SIGNAL( timeout() ), this, SLOT( onPivotStartupTimeout() ));
  connect(pivotSamplingTimer, SIGNAL( timeout() ), this, SLOT( onPivotSamplingTimeout() ));
  connect(spinStartupTimer, SIGNAL( timeout() ), this, SLOT( onSpinStartupTimeout() ));
  connect(spinSamplingTimer, SIGNAL( timeout() ), this, SLOT( onSpinSamplingTimeout() )); 
  
  connect( d->InputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(initializeObserver(vtkMRMLNode*)) );
  
  connect( d->startPivotButton, SIGNAL( clicked() ), this, SLOT( onStartPivotPart() ) );
  connect( d->startSpinButton, SIGNAL( clicked() ), this, SLOT( onStartSpinPart() ) );
  
  connect( d->startupTimerEdit, SIGNAL( valueChanged(double) ), this, SLOT( setStartupDurationSec(double) ) );
  connect( d->durationTimerEdit, SIGNAL( valueChanged(double) ), this, SLOT( setSamplingDurationSec(double) ) );

  connect( d->flipButton, SIGNAL( clicked() ), this, SLOT( onFlipButtonClicked() ) );
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onStartPivotPart()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  this->pivotStartupRemainingTimerPeriodCount = this->startupDurationSec;
  this->pivotSamplingRemainingTimerPeriodCount = this->samplingDurationSec;

  std::stringstream ss;
  ss << this->pivotStartupRemainingTimerPeriodCount << " seconds until start";
  d->CountdownLabel->setText(ss.str().c_str());  
  
  pivotStartupTimer->start();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onStartSpinPart()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  this->spinStartupRemainingTimerPeriodCount = this->startupDurationSec;
  this->spinSamplingRemainingTimerPeriodCount = this->samplingDurationSec;

  std::stringstream ss;
  ss << this->spinStartupRemainingTimerPeriodCount << " seconds until start";
  d->CountdownLabel->setText(ss.str().c_str());  
  
  spinStartupTimer->start();
}


//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onPivotStartupTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  std::stringstream ss1;
  
  --this->pivotStartupRemainingTimerPeriodCount;
  ss1 << this->pivotStartupRemainingTimerPeriodCount << " seconds until start";
  d->CountdownLabel->setText(ss1.str().c_str());

  if (this->pivotStartupRemainingTimerPeriodCount <= 0)
  {
    std::stringstream ss2;
    this->pivotSamplingRemainingTimerPeriodCount = this->samplingDurationSec;
    ss2 << "Sampling time left: " << this->pivotSamplingRemainingTimerPeriodCount;
    d->CountdownLabel->setText(ss2.str().c_str());
    
    this->pivotStartupTimer->stop();
    d->logic()->SetRecordingState(true);
    this->pivotSamplingTimer->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onPivotSamplingTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  
  std::stringstream ss;

  //std::cout<<this->samplingCount<<std::endl;
  
  --this->pivotSamplingRemainingTimerPeriodCount;
  ss << "Sampling time left: " << this->pivotSamplingRemainingTimerPeriodCount;
  d->CountdownLabel->setText(ss.str().c_str());  
  
  if (this->pivotSamplingRemainingTimerPeriodCount <= 0)
  {
    d->CountdownLabel->setText("Sampling complete");
    
    this->pivotSamplingTimer->stop();
    this->onPivotStop();
  }  
}


//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinStartupTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  std::stringstream ss1;
  
  --this->spinStartupRemainingTimerPeriodCount;
  ss1 << this->spinStartupRemainingTimerPeriodCount << " seconds until start";
  d->CountdownLabel->setText(ss1.str().c_str());

  if (this->spinStartupRemainingTimerPeriodCount <= 0)
  {
    std::stringstream ss2;
    this->spinSamplingRemainingTimerPeriodCount = this->samplingDurationSec;
    ss2 << "Sampling time left: " << this->spinSamplingRemainingTimerPeriodCount;
    d->CountdownLabel->setText(ss2.str().c_str());
    
    this->spinStartupTimer->stop();
    d->logic()->SetRecordingState(true);
    this->spinSamplingTimer->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinSamplingTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  
  std::stringstream ss;

  //std::cout<<this->samplingCount<<std::endl;
  
  --this->spinSamplingRemainingTimerPeriodCount;
  ss << "Sampling time left: " << this->spinSamplingRemainingTimerPeriodCount;
  d->CountdownLabel->setText(ss.str().c_str());  
  
  if (this->spinSamplingRemainingTimerPeriodCount <= 0)
  {
    d->CountdownLabel->setText("Sampling complete");
    
    this->spinSamplingTimer->stop();
    this->onSpinStop();
  }  
}


//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onPivotStop()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (outputTransform==NULL)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onPivotStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix( outputMatrix ); // Sync logic's matrix with the scene's matrix
  
  d->logic()->SetRecordingState(false);  
  d->logic()->ComputePivotCalibration();  

  d->logic()->GetToolTipToToolMatrix( outputMatrix );
  outputTransform->SetMatrixTransformToParent(outputMatrix);
  
  std::stringstream ss;
  ss << d->logic()->GetPivotRMSE();
  d->rmseLabel->setText(ss.str().c_str());
  
  d->logic()->ClearToolToReferenceMatrices();
}


//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinStop()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (outputTransform==NULL)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onSpinStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix( outputMatrix ); // Sync logic's matrix with the scene's matrix
  
  d->logic()->SetRecordingState(false);  
  d->logic()->ComputeSpinCalibration( d->snapCheckBox->checkState() == Qt::Checked ); 

  d->logic()->GetToolTipToToolMatrix( outputMatrix );
  outputTransform->SetMatrixTransformToParent(outputMatrix);
   
  // Set the rmse label for the circle fitting rms error
  std::stringstream ss;
  ss << d->logic()->GetSpinRMSE();
  d->rmseLabel->setText(ss.str().c_str());
  
  d->logic()->ClearToolToReferenceMatrices();
}


//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setStartupDurationSec(double timeSec)
{
  this->startupDurationSec = (int)timeSec;
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setSamplingDurationSec(double timeSec)
{
  this->samplingDurationSec = (int)timeSec;
}

void qSlicerPivotCalibrationModuleWidget::onFlipButtonClicked()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (outputTransform==NULL)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onSpinStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix( outputMatrix ); // Sync logic's matrix with the scene's matrix

  d->logic()->FlipShaftDirection();

  d->logic()->GetToolTipToToolMatrix( outputMatrix );
  outputTransform->SetMatrixTransformToParent(outputMatrix);

  // Set the rmse label for the circle fitting rms error
  d->rmseLabel->setText("Flipped.");
}