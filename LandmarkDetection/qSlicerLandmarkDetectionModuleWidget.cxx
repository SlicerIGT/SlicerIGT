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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerLandmarkDetectionModuleWidget.h"
#include "ui_qSlicerLandmarkDetectionModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>

// Markups MRML include 
#include <vtkMRMLMarkupsFiducialNode.h> 

// Landmark detection MRML includes
#include <vtkMRMLLandmarkDetectionNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLandmarkDetectionModuleWidgetPrivate : public Ui_qSlicerLandmarkDetectionModuleWidget
{
public:
  qSlicerLandmarkDetectionModuleWidgetPrivate();

  vtkWeakPointer<vtkMRMLLandmarkDetectionNode> LandmarkDetectionNode{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerLandmarkDetectionModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerLandmarkDetectionModuleWidgetPrivate::qSlicerLandmarkDetectionModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLandmarkDetectionModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerLandmarkDetectionModuleWidget::qSlicerLandmarkDetectionModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerLandmarkDetectionModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLandmarkDetectionModuleWidget::~qSlicerLandmarkDetectionModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModuleWidget::setup()
{
  Q_D(qSlicerLandmarkDetectionModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->parameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)));

  connect(d->stylusTipNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->landmarksNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->referenceNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->useMarkupsReferenceCheckBox, SIGNAL(toggled(bool)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->acquisitionRateDoubleSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->filterWindowDoubleSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->detectionTimeDoubleSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->shaftMinimumDisplacementDoubleSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->tipMaximumDisplacementDoubleSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->minimumLandmarkDistanceDoubleSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(updateMRMLFromWidget()));
  connect(d->startStopButton, SIGNAL(clicked()),
    this, SLOT(startStopButonClicked()));

}

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModuleWidget::enter()
{
  Q_D(qSlicerLandmarkDetectionModuleWidget);
  Superclass::enter();

  if (!this->mrmlScene())
  {
    return;
  }

  if (d->parameterNodeSelector->currentNode() == nullptr)
  {
    vtkMRMLLandmarkDetectionNode* node = vtkMRMLLandmarkDetectionNode::SafeDownCast(
      this->mrmlScene()->GetFirstNodeByClass("vtkMRMLLandmarkDetectionNode"));
    if (!node)
    {
      node = vtkMRMLLandmarkDetectionNode::SafeDownCast(d->parameterNodeSelector->addNode());
    }
    d->parameterNodeSelector->setCurrentNode(node);
  }
}

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModuleWidget::onCurrentNodeChanged(vtkMRMLNode * newCurrentNode)
{
  Q_D(qSlicerLandmarkDetectionModuleWidget);
  qvtkReconnect(d->LandmarkDetectionNode, newCurrentNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->LandmarkDetectionNode = vtkMRMLLandmarkDetectionNode::SafeDownCast(newCurrentNode);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerLandmarkDetectionModuleWidget);

  bool widgetsEnabled = (d->LandmarkDetectionNode != nullptr);
  d->stylusTipNodeSelector->setEnabled(widgetsEnabled);
  d->referenceNodeSelector->setEnabled(widgetsEnabled);
  d->useMarkupsReferenceCheckBox->setEnabled(widgetsEnabled);
  d->landmarksNodeSelector->setEnabled(widgetsEnabled);
  d->acquisitionRateDoubleSpinBox->setEnabled(widgetsEnabled);
  d->filterWindowDoubleSpinBox->setEnabled(widgetsEnabled);
  d->detectionTimeDoubleSpinBox->setEnabled(widgetsEnabled);
  d->shaftMinimumDisplacementDoubleSpinBox->setEnabled(widgetsEnabled);
  d->tipMaximumDisplacementDoubleSpinBox->setEnabled(widgetsEnabled);
  d->minimumLandmarkDistanceDoubleSpinBox->setEnabled(widgetsEnabled);
  d->startStopButton->setEnabled(widgetsEnabled);
  if (!d->LandmarkDetectionNode)
  {
    return;
  }

  bool wasBlocking = false;

  vtkMRMLTransformNode* inputTransformNode = d->LandmarkDetectionNode->GetInputTransformNode();

  wasBlocking = d->stylusTipNodeSelector->blockSignals(true);
  d->stylusTipNodeSelector->setCurrentNode(inputTransformNode);
  d->stylusTipNodeSelector->blockSignals(wasBlocking);

  wasBlocking = d->referenceNodeSelector->blockSignals(true);
  d->referenceNodeSelector->setCurrentNode(d->LandmarkDetectionNode->GetOutputCoordinateTransformNode());
  d->referenceNodeSelector->blockSignals(wasBlocking);

  wasBlocking = d->useMarkupsReferenceCheckBox->blockSignals(true);
  d->useMarkupsReferenceCheckBox->setChecked(d->LandmarkDetectionNode->GetUseMarkupsCoordinatesForOutput());
  d->useMarkupsReferenceCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->landmarksNodeSelector->blockSignals(true);
  d->landmarksNodeSelector->setCurrentNode(d->LandmarkDetectionNode->GetOutputMarkupsNode());
  d->landmarksNodeSelector->blockSignals(wasBlocking);

  wasBlocking = d->acquisitionRateDoubleSpinBox->blockSignals(true);
  d->acquisitionRateDoubleSpinBox->setValue(d->LandmarkDetectionNode->GetAcquisitionRateHz());
  d->acquisitionRateDoubleSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->filterWindowDoubleSpinBox->blockSignals(true);
  d->filterWindowDoubleSpinBox->setValue(d->LandmarkDetectionNode->GetFilterWindowTimeSec());
  d->filterWindowDoubleSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->detectionTimeDoubleSpinBox->blockSignals(true);
  d->detectionTimeDoubleSpinBox->setValue(d->LandmarkDetectionNode->GetDetectionTimeSec());
  d->detectionTimeDoubleSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->shaftMinimumDisplacementDoubleSpinBox->blockSignals(true);
  d->shaftMinimumDisplacementDoubleSpinBox->setValue(d->LandmarkDetectionNode->GetStylusShaftMinimumDisplacementThresholdMm());
  d->shaftMinimumDisplacementDoubleSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->tipMaximumDisplacementDoubleSpinBox->blockSignals(true);
  d->tipMaximumDisplacementDoubleSpinBox->setValue(d->LandmarkDetectionNode->GetStylusTipMaximumDisplacementThresholdMm());
  d->tipMaximumDisplacementDoubleSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->minimumLandmarkDistanceDoubleSpinBox->blockSignals(true);
  d->minimumLandmarkDistanceDoubleSpinBox->setValue(d->LandmarkDetectionNode->GetMinimumDistanceBetweenLandmarksMm());
  d->minimumLandmarkDistanceDoubleSpinBox->blockSignals(wasBlocking);

  bool landmarkDetectionInProgress = d->LandmarkDetectionNode->GetLandmarkDetectionInProgress();
  if (landmarkDetectionInProgress)
  {
    d->startStopButton->setText("Stop");
  }
  else
  {
    d->startStopButton->setText("Start");
  }

  d->stylusTipNodeSelector->setEnabled(!landmarkDetectionInProgress);
  d->referenceNodeSelector->setEnabled(!landmarkDetectionInProgress && !d->useMarkupsReferenceCheckBox->isChecked());
  d->useMarkupsReferenceCheckBox->setEnabled(!landmarkDetectionInProgress);
  d->landmarksNodeSelector->setEnabled(!landmarkDetectionInProgress);
  d->acquisitionRateDoubleSpinBox->setEnabled(!landmarkDetectionInProgress);
  d->filterWindowDoubleSpinBox->setEnabled(!landmarkDetectionInProgress);
  d->detectionTimeDoubleSpinBox->setEnabled(!landmarkDetectionInProgress);
  d->shaftMinimumDisplacementDoubleSpinBox->setEnabled(!landmarkDetectionInProgress);
  d->tipMaximumDisplacementDoubleSpinBox->setEnabled(!landmarkDetectionInProgress);
  d->minimumLandmarkDistanceDoubleSpinBox->setEnabled(!landmarkDetectionInProgress);
  d->startStopButton->setEnabled(inputTransformNode != nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModuleWidget::updateMRMLFromWidget()
{
  Q_D(qSlicerLandmarkDetectionModuleWidget);
  if (!d->LandmarkDetectionNode)
  {
    return;
  }

  d->LandmarkDetectionNode->SetAndObserveInputTransformNode(vtkMRMLTransformNode::SafeDownCast(d->stylusTipNodeSelector->currentNode()));
  d->LandmarkDetectionNode->SetAndObserveOutputCoordinateTransformNode(vtkMRMLTransformNode::SafeDownCast(d->referenceNodeSelector->currentNode()));
  d->LandmarkDetectionNode->SetUseMarkupsCoordinatesForOutput(d->useMarkupsReferenceCheckBox->isChecked());
  d->LandmarkDetectionNode->SetAndObserveOutputMarkupsNode(vtkMRMLMarkupsFiducialNode::SafeDownCast(d->landmarksNodeSelector->currentNode()));
  d->LandmarkDetectionNode->SetAcquisitionRateHz(d->acquisitionRateDoubleSpinBox->value());
  d->LandmarkDetectionNode->SetFilterWindowTimeSec(d->filterWindowDoubleSpinBox->value());
  d->LandmarkDetectionNode->SetDetectionTimeSec(d->detectionTimeDoubleSpinBox->value());
  d->LandmarkDetectionNode->SetStylusShaftMinimumDisplacementThresholdMm(d->shaftMinimumDisplacementDoubleSpinBox->value());
  d->LandmarkDetectionNode->SetStylusTipMaximumDisplacementThresholdMm(d->tipMaximumDisplacementDoubleSpinBox->value());
  d->LandmarkDetectionNode->SetMinimumDistanceBetweenLandmarksMm(d->minimumLandmarkDistanceDoubleSpinBox->value());
}

//-----------------------------------------------------------------------------
void qSlicerLandmarkDetectionModuleWidget::startStopButonClicked()
{
  Q_D(qSlicerLandmarkDetectionModuleWidget);
  if (!d->LandmarkDetectionNode)
  {
    return;
  }

  bool landmarkDetectionInProgress = !d->LandmarkDetectionNode->GetLandmarkDetectionInProgress();
  d->LandmarkDetectionNode->SetLandmarkDetectionInProgress(landmarkDetectionInProgress);
}
