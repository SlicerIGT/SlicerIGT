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
class qSlicerPivotCalibrationModuleWidgetPrivate : public Ui_qSlicerPivotCalibrationModule
{
  Q_DECLARE_PUBLIC(qSlicerPivotCalibrationModuleWidget);
protected:
  qSlicerPivotCalibrationModuleWidget* const q_ptr;
public:
  qSlicerPivotCalibrationModuleWidgetPrivate(qSlicerPivotCalibrationModuleWidget& object);
  vtkSlicerPivotCalibrationLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerPivotCalibrationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidgetPrivate::qSlicerPivotCalibrationModuleWidgetPrivate(qSlicerPivotCalibrationModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic* qSlicerPivotCalibrationModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerPivotCalibrationModuleWidget);
  return vtkSlicerPivotCalibrationLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerPivotCalibrationModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerPivotCalibrationModuleWidget::qSlicerPivotCalibrationModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPivotCalibrationModuleWidgetPrivate(*this))
{
  this->pivotStartupTimer = new QTimer();
  this->pivotStartupTimer->setSingleShot(false);
  this->pivotStartupTimer->setInterval(1000); // 1 sec

  this->pivotSamplingTimer = new QTimer();
  this->pivotSamplingTimer->setSingleShot(false);
  this->pivotSamplingTimer->setInterval(1000); // 1 sec

  this->spinStartupTimer = new QTimer();
  this->spinStartupTimer->setSingleShot(false);
  this->spinStartupTimer->setInterval(1000); // 1 sec

  this->spinSamplingTimer = new QTimer();
  this->spinSamplingTimer->setSingleShot(false);
  this->spinSamplingTimer->setInterval(1000); // 1 sec
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

  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  d->logic()->SetAndObserveTransformNode(transformNode);
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::setup()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  d->setupUi(this);

  this->Superclass::setup();

  // Tab widget connections
  connect(d->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateLogicFromWidget()));

  // Manual calibration connections
  connect(pivotStartupTimer, SIGNAL(timeout()), this, SLOT(onPivotStartupTimeout()));
  connect(pivotSamplingTimer, SIGNAL(timeout()), this, SLOT(onPivotSamplingTimeout()));
  connect(spinStartupTimer, SIGNAL(timeout()), this, SLOT(onSpinStartupTimeout()));
  connect(spinSamplingTimer, SIGNAL(timeout()), this, SLOT(onSpinSamplingTimeout()));

  connect(d->InputComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(initializeObserver(vtkMRMLNode*)));

  connect(d->startPivotButton, SIGNAL(clicked()), this, SLOT(onStartPivotPart()));
  connect(d->startSpinButton, SIGNAL(clicked()), this, SLOT(onStartSpinPart()));

  connect(d->startupTimerEdit, SIGNAL(valueChanged(double)), this, SLOT(setStartupDurationSec(double)));
  connect(d->durationTimerEdit, SIGNAL(valueChanged(double)), this, SLOT(setSamplingDurationSec(double)));

  connect(d->flipButton, SIGNAL(clicked()), this, SLOT(onFlipButtonClicked()));

  // Auto calibration connections

  // Pivot calibration settings
  connect(d->pivotTargetPointSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotTargetErrorSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotMinOrientationDifferenceSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));

  connect(d->pivotAutoCalibrationButton, SIGNAL(clicked()), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotAutoStopCheckBox, SIGNAL(clicked()), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotResetButton, SIGNAL(clicked()), this, SLOT(onPivotResetButtonClicked()));

  connect(d->pivotBucketSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotMaxBucketSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotMaxBucketError, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));

  connect(d->pivotInputOrientationDifferenceThresholdSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));
  connect(d->pivotInputMinPositionDifferenceSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));

  // Spin calibration settings
  connect(d->spinTargetPointSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLogicFromWidget()));
  connect(d->spinTargetErrorSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));
  connect(d->spinMinOrientationDifferenceSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));

  connect(d->spinAutoCalibrationButton, SIGNAL(clicked()), this, SLOT(updateLogicFromWidget()));
  connect(d->spinAutoStopCheckBox, SIGNAL(clicked()), this, SLOT(updateLogicFromWidget()));
  connect(d->spinResetButton, SIGNAL(clicked()), this, SLOT(onSpinResetButtonClicked()));

  connect(d->spinBucketSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLogicFromWidget()));
  connect(d->spinMaxBucketSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateLogicFromWidget()));
  connect(d->spinMaxBucketError, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));

  connect(d->spinInputOrientationDifferenceThresholdSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));
  connect(d->spinInputMinPositionDifferenceSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLogicFromWidget()));

  // Logic observers
  qvtkConnect(this->logic(), vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromLogic()));
  qvtkConnect(this->logic(), vtkSlicerPivotCalibrationLogic::InputTransformAdded, this, SLOT(updateWidgetFromLogic()));
  qvtkConnect(this->logic(), vtkSlicerPivotCalibrationLogic::PivotInputTransformAdded, this, SLOT(updateWidgetFromLogic()));
  qvtkConnect(this->logic(), vtkSlicerPivotCalibrationLogic::SpinInputTransformAdded, this, SLOT(updateWidgetFromLogic()));
  qvtkConnect(this->logic(), vtkSlicerPivotCalibrationLogic::PivotCalibrationCompleteEvent, this, SLOT(onPivotAutoCalibrationComplete()));
  qvtkConnect(this->logic(), vtkSlicerPivotCalibrationLogic::SpinCalibrationCompleteEvent, this, SLOT(onSpinAutoCalibrationComplete()));

  this->updateWidgetFromLogic();
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

  this->pivotStartupTimer->start();

  this->updateWidgetFromLogic();
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

  this->spinStartupTimer->start();

  this->updateWidgetFromLogic();
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
    d->logic()->SetPivotCalibrationEnabled(true);
    d->logic()->SetSpinCalibrationEnabled(false);
    d->logic()->ClearPivotToolToReferenceMatrices();
    d->logic()->SetRecordingState(true);
    this->pivotSamplingTimer->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onPivotSamplingTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  --this->pivotSamplingRemainingTimerPeriodCount;

  std::stringstream ss;
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
    d->logic()->SetPivotCalibrationEnabled(false);
    d->logic()->SetSpinCalibrationEnabled(true);
    d->logic()->ClearSpinToolToReferenceMatrices();
    d->logic()->SetRecordingState(true);
    this->spinSamplingTimer->start();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinSamplingTimeout()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  --this->spinSamplingRemainingTimerPeriodCount;

  std::stringstream ss;
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

  d->logic()->SetRecordingState(false);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (outputTransform == NULL)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onPivotStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix(outputMatrix); // Sync logic's matrix with the scene's matrix

  if (d->logic()->ComputePivotCalibration())
  {
    d->logic()->GetToolTipToToolMatrix(outputMatrix);
    outputTransform->SetMatrixTransformToParent(outputMatrix);
    std::stringstream ss;
    ss << d->logic()->GetPivotRMSE();
    d->rmseLabel->setText(ss.str().c_str());
  }
  else
  {
    qWarning() << "qSlicerPivotCalibrationModuleWidget::onPivotStop failed: ComputePivotCalibration returned with error: " << d->logic()->GetErrorText().c_str();
    std::string fullMessage = std::string("Pivot calibration failed: ") + d->logic()->GetErrorText();
    d->CountdownLabel->setText(fullMessage.c_str());
    d->rmseLabel->setText("N/A");
  }

  d->logic()->ClearToolToReferenceMatrices();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinStop()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  d->logic()->SetRecordingState(false);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (!outputTransform)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onSpinStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix(outputMatrix); // Sync logic's matrix with the scene's matrix

  if (d->logic()->ComputeSpinCalibration(d->snapCheckBox->checkState() == Qt::Checked))
  {
    d->logic()->GetToolTipToToolMatrix(outputMatrix);
    outputTransform->SetMatrixTransformToParent(outputMatrix);

    // Set the rmse label for the circle fitting rms error
    std::stringstream ss;
    ss << d->logic()->GetSpinRMSE();
    d->rmseLabel->setText(ss.str().c_str());
  }
  else
  {
    qWarning() << "qSlicerPivotCalibrationModuleWidget::onSpinStop failed: ComputeSpinCalibration returned with error: " << d->logic()->GetErrorText().c_str();
    std::string fullMessage = std::string("Spin calibration failed: ") + d->logic()->GetErrorText();
    d->CountdownLabel->setText(fullMessage.c_str());
    d->rmseLabel->setText("N/A");
  }

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

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onFlipButtonClicked()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (!outputTransform)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onSpinStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix(outputMatrix); // Sync logic's matrix with the scene's matrix

  d->logic()->FlipShaftDirection();

  d->logic()->GetToolTipToToolMatrix(outputMatrix);
  outputTransform->SetMatrixTransformToParent(outputMatrix);

  // Set the rmse label for the circle fitting rms error
  d->rmseLabel->setText("Flipped.");
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::updateLogicFromWidget()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  // Pivot calibration settings
  d->logic()->SetPivotAutoCalibrationStopWhenComplete(d->pivotAutoStopCheckBox->isChecked());

  d->logic()->SetPivotAutoCalibrationTargetError(d->pivotTargetErrorSpinBox->value());
  d->logic()->SetPivotAutoCalibrationTargetNumberOfPoints(d->pivotTargetPointSpinBox->value());
  d->logic()->SetPivotMinimumOrientationDifferenceDegrees(d->pivotMinOrientationDifferenceSpinBox->value());

  d->logic()->SetPivotPoseBucketSize(d->pivotBucketSizeSpinBox->value());
  d->logic()->SetPivotMaximumNumberOfPoseBuckets(d->pivotMaxBucketSpinBox->value());
  d->logic()->SetPivotMaximumPoseBucketError(d->pivotMaxBucketError->value());

  d->logic()->SetPivotPositionDifferenceThresholdMm(d->pivotInputMinPositionDifferenceSpinBox->value());
  d->logic()->SetPivotOrientationDifferenceThresholdDegrees(d->pivotInputOrientationDifferenceThresholdSpinBox->value());

  // Spin calibration settings
  d->logic()->SetSpinAutoCalibrationStopWhenComplete(d->spinAutoStopCheckBox->isChecked());

  d->logic()->SetSpinAutoCalibrationTargetError(d->spinTargetErrorSpinBox->value());
  d->logic()->SetSpinAutoCalibrationTargetNumberOfPoints(d->spinTargetPointSpinBox->value());
  d->logic()->SetSpinMinimumOrientationDifferenceDegrees(d->spinMinOrientationDifferenceSpinBox->value());

  d->logic()->SetSpinPoseBucketSize(d->spinBucketSizeSpinBox->value());
  d->logic()->SetSpinMaximumNumberOfPoseBuckets(d->spinMaxBucketSpinBox->value());
  d->logic()->SetSpinMaximumPoseBucketError(d->spinMaxBucketError->value());

  d->logic()->SetSpinPositionDifferenceThresholdMm(d->spinInputMinPositionDifferenceSpinBox->value());
  d->logic()->SetSpinOrientationDifferenceThresholdDegrees(d->spinInputOrientationDifferenceThresholdSpinBox->value());

  if (d->tabWidget->currentWidget() == d->autoCalibrationTab)
  {
    // Enable calibration
    bool pivotAutoCalibration = d->pivotAutoCalibrationButton->isChecked();
    d->logic()->SetPivotCalibrationEnabled(pivotAutoCalibration);
    d->logic()->SetPivotAutoCalibrationEnabled(pivotAutoCalibration);
    bool spinAutoCalibration = d->spinAutoCalibrationButton->isChecked();
    d->logic()->SetSpinCalibrationEnabled(spinAutoCalibration);
    d->logic()->SetSpinAutoCalibrationEnabled(spinAutoCalibration);
    d->logic()->SetRecordingState(d->logic()->GetPivotCalibrationEnabled() || d->logic()->GetSpinCalibrationEnabled());
  }
  else
  {
    d->logic()->SetPivotCalibrationEnabled(true);
    d->logic()->SetPivotAutoCalibrationEnabled(false);
    d->logic()->SetSpinCalibrationEnabled(true);
    d->logic()->SetSpinAutoCalibrationEnabled(false);
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::updateWidgetFromLogic()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  bool calibrationRunning = d->logic()->GetRecordingState()
    || this->pivotSamplingTimer->isActive()
    || this->pivotStartupTimer->isActive()
    || this->spinSamplingTimer->isActive()
    || this->spinStartupTimer->isActive();

  vtkMRMLNode* inputTransformNode = d->InputComboBox->currentNode();

  d->tabWidget->tabBar()->setEnabled(!calibrationRunning);
  d->startPivotButton->setEnabled(!calibrationRunning && inputTransformNode);
  d->startSpinButton->setEnabled(!calibrationRunning && inputTransformNode);
  d->startupTimerEdit->setEnabled(!calibrationRunning);
  d->durationTimerEdit->setEnabled(!calibrationRunning);

  bool wasBlocking = false;

  // Pivot auto-calibration settings
  wasBlocking = d->pivotAutoCalibrationButton->blockSignals(true);
  d->pivotAutoCalibrationButton->setChecked(d->logic()->GetPivotCalibrationEnabled() && d->logic()->GetPivotAutoCalibrationEnabled());
  d->pivotAutoCalibrationButton->blockSignals(wasBlocking);
  wasBlocking = d->pivotAutoStopCheckBox->blockSignals(true);
  d->pivotAutoStopCheckBox->setChecked(d->logic()->GetPivotAutoCalibrationStopWhenComplete());
  d->pivotAutoStopCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->pivotTargetPointSpinBox->blockSignals(true);
  d->pivotTargetPointSpinBox->setValue(d->logic()->GetPivotAutoCalibrationTargetNumberOfPoints());
  d->pivotTargetPointSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->pivotTargetErrorSpinBox->blockSignals(true);
  d->pivotTargetErrorSpinBox->setValue(d->logic()->GetPivotAutoCalibrationTargetError());
  d->pivotTargetErrorSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->pivotMinOrientationDifferenceSpinBox->blockSignals(true);
  d->pivotMinOrientationDifferenceSpinBox->setValue(d->logic()->GetPivotMinimumOrientationDifferenceDegrees());
  d->pivotMinOrientationDifferenceSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->pivotBucketSizeSpinBox->blockSignals(true);
  d->pivotBucketSizeSpinBox->setValue(d->logic()->GetPivotPoseBucketSize());
  d->pivotBucketSizeSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->pivotMaxBucketSpinBox->blockSignals(true);
  d->pivotMaxBucketSpinBox->setValue(d->logic()->GetPivotMaximumNumberOfPoseBuckets());
  d->pivotMaxBucketSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->pivotMaxBucketError->blockSignals(true);
  d->pivotMaxBucketError->setValue(d->logic()->GetPivotMaximumPoseBucketError());
  d->pivotMaxBucketError->blockSignals(wasBlocking);

  wasBlocking = d->pivotInputOrientationDifferenceThresholdSpinBox->blockSignals(true);
  d->pivotInputOrientationDifferenceThresholdSpinBox->setValue(d->logic()->GetPivotOrientationDifferenceThresholdDegrees());
  d->pivotInputOrientationDifferenceThresholdSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->pivotInputMinPositionDifferenceSpinBox->blockSignals(true);
  d->pivotInputMinPositionDifferenceSpinBox->setValue(d->logic()->GetPivotPositionDifferenceThresholdMm());
  d->pivotInputMinPositionDifferenceSpinBox->blockSignals(wasBlocking);

  int numberOfPivotPoses = d->logic()->GetPivotNumberOfPoses();
  int targetNumberPivotPoses = d->logic()->GetPivotAutoCalibrationTargetNumberOfPoints();
  d->pivotCalibrationProgressBar->setValue(100.0 * (double)numberOfPivotPoses / targetNumberPivotPoses);

  // Spin auto-calibration settings
  wasBlocking = d->spinAutoCalibrationButton->blockSignals(true);
  d->spinAutoCalibrationButton->setChecked(d->logic()->GetSpinCalibrationEnabled() && d->logic()->GetSpinAutoCalibrationEnabled());
  d->spinAutoCalibrationButton->blockSignals(wasBlocking);
  wasBlocking = d->spinAutoStopCheckBox->blockSignals(true);
  d->spinAutoStopCheckBox->setChecked(d->logic()->GetSpinAutoCalibrationStopWhenComplete());
  d->spinAutoStopCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->spinTargetPointSpinBox->blockSignals(true);
  d->spinTargetPointSpinBox->setValue(d->logic()->GetSpinAutoCalibrationTargetNumberOfPoints());
  d->spinTargetPointSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->spinTargetErrorSpinBox->blockSignals(true);
  d->spinTargetErrorSpinBox->setValue(d->logic()->GetSpinAutoCalibrationTargetError());
  d->spinTargetErrorSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->spinMinOrientationDifferenceSpinBox->blockSignals(true);
  d->spinMinOrientationDifferenceSpinBox->setValue(d->logic()->GetSpinMinimumOrientationDifferenceDegrees());
  d->spinMinOrientationDifferenceSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->spinBucketSizeSpinBox->blockSignals(true);
  d->spinBucketSizeSpinBox->setValue(d->logic()->GetSpinPoseBucketSize());
  d->spinBucketSizeSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->spinMaxBucketSpinBox->blockSignals(true);
  d->spinMaxBucketSpinBox->setValue(d->logic()->GetSpinMaximumNumberOfPoseBuckets());
  d->spinMaxBucketSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->spinMaxBucketError->blockSignals(true);
  d->spinMaxBucketError->setValue(d->logic()->GetSpinMaximumPoseBucketError());
  d->spinMaxBucketError->blockSignals(wasBlocking);

  wasBlocking = d->spinInputOrientationDifferenceThresholdSpinBox->blockSignals(true);
  d->spinInputOrientationDifferenceThresholdSpinBox->setValue(d->logic()->GetSpinOrientationDifferenceThresholdDegrees());
  d->spinInputOrientationDifferenceThresholdSpinBox->blockSignals(wasBlocking);
  wasBlocking = d->spinInputMinPositionDifferenceSpinBox->blockSignals(true);
  d->spinInputMinPositionDifferenceSpinBox->setValue(d->logic()->GetSpinPositionDifferenceThresholdMm());
  d->spinInputMinPositionDifferenceSpinBox->blockSignals(wasBlocking);

  int numberOfSpinPoses = d->logic()->GetSpinNumberOfPoses();
  int targetNumberSpinPoses = d->logic()->GetSpinAutoCalibrationTargetNumberOfPoints();
  d->spinCalibrationProgressBar->setValue(100.0 * (double)numberOfSpinPoses / targetNumberSpinPoses);
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onPivotAutoCalibrationComplete()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (!outputTransform)
  {
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix(outputMatrix); // Sync logic's matrix with the scene's matrix

  if (d->logic()->ComputePivotCalibration())
  {
    d->logic()->GetToolTipToToolMatrix(outputMatrix);
    outputTransform->SetMatrixTransformToParent(outputMatrix);
    std::stringstream ss;
    ss << d->logic()->GetPivotRMSE();
    d->rmseLabel->setText(ss.str().c_str());
  }
  else
  {
    qWarning() << "qSlicerPivotCalibrationModuleWidget::onPivotStop failed: ComputePivotCalibration returned with error: " << d->logic()->GetErrorText().c_str();
    std::string fullMessage = std::string("Pivot calibration failed: ") + d->logic()->GetErrorText();
    d->CountdownLabel->setText(fullMessage.c_str());
    d->rmseLabel->setText("N/A");
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinAutoCalibrationComplete()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);

  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast(d->OutputComboBox->currentNode());
  if (!outputTransform)
  {
    qCritical("qSlicerPivotCalibrationModuleWidget::onSpinStop failed: cannot save output transform");
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> outputMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  outputTransform->GetMatrixTransformToParent(outputMatrix);
  d->logic()->SetToolTipToToolMatrix(outputMatrix); // Sync logic's matrix with the scene's matrix

  if (d->logic()->ComputeSpinCalibration(d->snapCheckBox->checkState() == Qt::Checked))
  {
    d->logic()->GetToolTipToToolMatrix(outputMatrix);
    outputTransform->SetMatrixTransformToParent(outputMatrix);

    // Set the rmse label for the circle fitting rms error
    std::stringstream ss;
    ss << d->logic()->GetSpinRMSE();
    d->rmseLabel->setText(ss.str().c_str());
  }
  else
  {
    qWarning() << "qSlicerPivotCalibrationModuleWidget::onSpinStop failed: ComputeSpinCalibration returned with error: " << d->logic()->GetErrorText().c_str();
    std::string fullMessage = std::string("Spin calibration failed: ") + d->logic()->GetErrorText();
    d->CountdownLabel->setText(fullMessage.c_str());
    d->rmseLabel->setText("N/A");
  }
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onPivotResetButtonClicked()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  d->logic()->ClearPivotToolToReferenceMatrices();
  this->updateWidgetFromLogic();
}

//-----------------------------------------------------------------------------
void qSlicerPivotCalibrationModuleWidget::onSpinResetButtonClicked()
{
  Q_D(qSlicerPivotCalibrationModuleWidget);
  d->logic()->ClearSpinToolToReferenceMatrices();
  this->updateWidgetFromLogic();
}
