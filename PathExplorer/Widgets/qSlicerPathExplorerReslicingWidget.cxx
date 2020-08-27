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

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898

  ==============================================================================*/

// PathExplorer Widgets includes
#include "qSlicerPathExplorerReslicingWidget.h"
#include "ui_qSlicerPathExplorerReslicingWidget.h"

#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsLineNode.h>
#include <vtkMRMLSliceNode.h>

#include "ctkPopupWidget.h"

// VTK includes
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

class qSlicerPathExplorerReslicingWidget;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathExplorer
class Q_SLICER_MODULE_PATHEXPLORER_WIDGETS_EXPORT qSlicerPathExplorerReslicingWidgetPrivate
  : public Ui_qSlicerPathExplorerReslicingWidget
{
  Q_DECLARE_PUBLIC(qSlicerPathExplorerReslicingWidget);

 public:
  qSlicerPathExplorerReslicingWidgetPrivate(
    qSlicerPathExplorerReslicingWidget& object);
  virtual ~qSlicerPathExplorerReslicingWidgetPrivate();
  virtual void setupUi(qSlicerPathExplorerReslicingWidget*);

  int loadAttributesFromViewer();
  void saveAttributesToViewer();
  void updateWidget();

 protected:
  qSlicerPathExplorerReslicingWidget * const   q_ptr;
  vtkMRMLSliceNode*                             SliceNode;
  vtkMRMLMarkupsLineNode*                       ReslicingLineNode;
  std::string                                   DrivingRulerNodeID;
  std::string                                   DrivingRulerNodeName;
  double                                        ResliceAngle;
  double                                        ReslicePosition;
  bool                                          ReslicePerpendicular;
};

//-----------------------------------------------------------------------------
qSlicerPathExplorerReslicingWidgetPrivate
::qSlicerPathExplorerReslicingWidgetPrivate(
  qSlicerPathExplorerReslicingWidget& object)
  : q_ptr(&object)
{
  this->DrivingRulerNodeID.assign("");
  this->DrivingRulerNodeName.assign("");
  this->SliceNode = nullptr;
  this->ReslicingLineNode = nullptr;
  this->ResliceAngle = 0.0;
  this->ReslicePosition = 0.0;
  this->ReslicePerpendicular = true;
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerReslicingWidgetPrivate
::~qSlicerPathExplorerReslicingWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidgetPrivate
::setupUi(qSlicerPathExplorerReslicingWidget* widget)
{
  this->Ui_qSlicerPathExplorerReslicingWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
int qSlicerPathExplorerReslicingWidgetPrivate
::loadAttributesFromViewer()
{
  if (!this->ReslicingLineNode)
    {
    return 0;
    }

  const char* drivingID = this->SliceNode->GetAttribute("PathExplorer.DrivingPathID");
  if (!drivingID)
    {
    return 0;
    }
  this->DrivingRulerNodeID.assign(drivingID);

  const char* drivingName = this->SliceNode->GetAttribute("PathExplorer.DrivingPathName");
  if (!drivingName)
    {
    return 0;
    }
  this->DrivingRulerNodeName.assign(drivingName);

  std::stringstream itemPosAttrStr;
  itemPosAttrStr << "PathExplorer." << this->ReslicingLineNode->GetName() << "_" << this->SliceNode->GetName() << "_Position";
  const char* posStr = this->SliceNode->GetAttribute(itemPosAttrStr.str().c_str());
  this->ReslicePosition = posStr ?
    atof(posStr) :
    0.0;

  std::stringstream itemAngleAttrStr;
  itemAngleAttrStr << "PathExplorer." << this->ReslicingLineNode->GetName() << "_" << this->SliceNode->GetName() << "_Angle";
  const char* angleStr = this->SliceNode->GetAttribute(itemAngleAttrStr.str().c_str());
  this->ResliceAngle = angleStr ?
    atof(angleStr) :
    0.0;

  std::stringstream itemPerpAttrStr;
  itemPerpAttrStr << "PathExplorer." << this->ReslicingLineNode->GetName() << "_" << this->SliceNode->GetName() << "_Perpendicular";
  const char* perpStr = this->SliceNode->GetAttribute(itemPerpAttrStr.str().c_str());
  if (perpStr)
    {
    this->ReslicePerpendicular = strcmp(perpStr, "ON") == 0;
    }

  return 1;
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidgetPrivate
::saveAttributesToViewer()
{
  if (!this->ReslicingLineNode)
    {
    return;
    }

  std::stringstream posAttrStr;
  posAttrStr << "PathExplorer." << this->ReslicingLineNode->GetName() << "_" << this->SliceNode->GetName() << "_Position";
  std::stringstream posValStr;
  posValStr << this->ReslicePosition;
  this->SliceNode->SetAttribute(posAttrStr.str().c_str(), posValStr.str().c_str());

  std::stringstream angleAttrStr;
  angleAttrStr << "PathExplorer." << this->ReslicingLineNode->GetName() << "_" << this->SliceNode->GetName() << "_Angle";
  std::stringstream angleValStr;
  angleValStr << this->ResliceAngle;
  this->SliceNode->SetAttribute(angleAttrStr.str().c_str(), angleValStr.str().c_str());

  std::stringstream perpAttrStr;
  perpAttrStr << "PathExplorer." << this->ReslicingLineNode->GetName() << "_" << this->SliceNode->GetName() << "_Perpendicular";
  this->SliceNode->SetAttribute(perpAttrStr.str().c_str(), this->ReslicePerpendicularRadioButton->isChecked() ?
                                "ON" :
                                "OFF");
}
//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidgetPrivate
::updateWidget()
{
  if (!this->SliceNode || !this->ReslicingLineNode)
    {
    return;
    }

  vtkMRMLMarkupsDisplayNode* lineDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(this->ReslicingLineNode->GetDisplayNode());

  // block all signals while updating
  bool sliderOldState = this->ResliceSlider->blockSignals(true);
  bool perpendicularOldState = this->ReslicePerpendicularRadioButton->blockSignals(true);
  bool inPlaneOldState = this->ResliceInPlaneRadioButton->blockSignals(true);

  // Get updated values
  int sliderMinimum = 0;
  int sliderMaximum = this->ReslicePerpendicular ? 100 : 180;
  double sliderValue = this->ReslicePerpendicular ? this->ReslicePosition : this->ResliceAngle;
  const char* distanceLabel = this->ReslicePerpendicular ? "Distance: " : "Angle: ";
  const char* unitsLabel = this->ReslicePerpendicular ? "mm" : "deg";

  // Update reslice button
  int enabled = 0;
  if (!this->DrivingRulerNodeID.empty() && this->DrivingRulerNodeID.compare(this->ReslicingLineNode->GetID()) == 0)
    {
    enabled = 1;
    this->ResliceButton->setText(this->DrivingRulerNodeName.c_str());
    }
  this->ResliceButton->setChecked(enabled);
  this->ResliceSlider->setEnabled(enabled);
  this->ReslicePerpendicularRadioButton->setEnabled(enabled);
  this->ResliceInPlaneRadioButton->setEnabled(enabled);

  // Update slider
  this->ResliceSlider->setMinimum(sliderMinimum);
  this->ResliceSlider->setMaximum(sliderMaximum);
  this->ResliceSlider->setValue(sliderValue);

  // Update labels
  this->DistanceAngleLabel->setText(distanceLabel);
  this->DistanceAngleUnitLabel->setText(unitsLabel);

  QString decimalValue = QString::number(this->ResliceAngle);
  if (this->ReslicePerpendicular)
    {
    double distanceValue = this->ReslicingLineNode->GetLineLengthWorld() * this->ReslicePosition / 100;
    decimalValue.setNum(distanceValue, 'f', 2);
    }
  this->ResliceValueLabel->setText(decimalValue);

  // Update Perpendicular / InPlane
  this->ReslicePerpendicularRadioButton->setChecked(this->ReslicePerpendicular);
  this->ResliceInPlaneRadioButton->setChecked(!this->ReslicePerpendicular);

  // Update slice intersection visibility
  if (lineDisplayNode && this->ResliceButton->isChecked())
    {
    lineDisplayNode->SetVisibility2D(this->ReslicePerpendicular);
    }

  // Restore signals
  this->ResliceSlider->blockSignals(sliderOldState);
  this->ReslicePerpendicularRadioButton->blockSignals(perpendicularOldState);
  this->ResliceInPlaneRadioButton->blockSignals(inPlaneOldState);
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerReslicingWidget
::qSlicerPathExplorerReslicingWidget(QWidget *parentWidget)
  : Superclass (parentWidget)
  , d_ptr(new qSlicerPathExplorerReslicingWidgetPrivate(*this))
{
  Q_D(qSlicerPathExplorerReslicingWidget);
  d->setupUi(this);

  this->setSliceNode(nullptr);

  connect(d->ResliceButton, SIGNAL(toggled(bool)),
          this, SLOT(onResliceToggled(bool)));
  connect(d->ResliceSlider, SIGNAL(valueChanged(int)),
          this, SLOT(onResliceValueChanged(int)));
  connect(d->ReslicePerpendicularRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onPerpendicularToggled(bool)));

  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
	  this, SLOT(onMRMLSceneChanged(vtkMRMLScene*)));
}

//-----------------------------------------------------------------------------
qSlicerPathExplorerReslicingWidget
::~qSlicerPathExplorerReslicingWidget()
{
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::setReslicingRulerNode(vtkMRMLMarkupsLineNode* ruler)
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  // Disable everything except button
  this->setEnabled(1);

  if (!d->SliceNode)
    {
    return;
    }

  if (d->ReslicingLineNode)
    {
    // If previous trajectory, save values as attributes before changing it
    d->saveAttributesToViewer();
    }

  // Load previous values of new trajectory
  d->ReslicingLineNode = ruler;
  if (d->loadAttributesFromViewer())
    {
    d->updateWidget();
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::onMRMLSceneChanged(vtkMRMLScene* newScene)
{
  if (!newScene)
    {
    return;
    }

  this->qvtkReconnect(this->mrmlScene(),
		      newScene, vtkMRMLScene::NodeAboutToBeRemovedEvent,
		      this, SLOT(onMRMLNodeRemoved(vtkObject*, void*)));
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::onMRMLNodeRemoved(vtkObject*, void* callData)
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  if (!d->ReslicingLineNode)
    {
    return;
    }

  if (callData == d->ReslicingLineNode)
    {
    d->ReslicingLineNode = NULL;
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::onRulerModified()
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  if (!d->ReslicingLineNode)
    {
    return;
    }

  this->resliceWithRuler(d->ReslicingLineNode,
			 d->SliceNode,
			 d->ReslicePerpendicular,
			 d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::onResliceToggled(bool buttonStatus)
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  if (!d->SliceNode || !d->ReslicingLineNode)
    {
    return;
    }

  if (buttonStatus)
    {
    // Reslice
    d->ResliceSlider->setEnabled(1);
    d->ReslicePerpendicularRadioButton->setEnabled(1);
    d->ResliceInPlaneRadioButton->setEnabled(1);

    d->DrivingRulerNodeID.assign(d->ReslicingLineNode->GetID());
    d->DrivingRulerNodeName.assign(d->ReslicingLineNode->GetName());
    d->SliceNode->SetAttribute("PathExplorer.DrivingPathID",d->ReslicingLineNode->GetID());
    d->SliceNode->SetAttribute("PathExplorer.DrivingPathName",d->ReslicingLineNode->GetName());
    d->updateWidget();

    this->resliceWithRuler(d->ReslicingLineNode,
                           d->SliceNode,
                           d->ReslicePerpendicular,
                           d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);

    this->qvtkConnect(d->ReslicingLineNode, vtkCommand::ModifiedEvent,
		      this, SLOT(onRulerModified()));
    }
  else
    {
    // Reslice
    d->ResliceSlider->setEnabled(0);
    d->ReslicePerpendicularRadioButton->setEnabled(0);
    d->ResliceInPlaneRadioButton->setEnabled(0);

    this->qvtkDisconnect(d->ReslicingLineNode, vtkCommand::ModifiedEvent,
			 this, SLOT(onRulerModified()));
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::onPerpendicularToggled(bool status)
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  if (!d->ReslicingLineNode)
    {
    return;
    }

  d->ReslicePerpendicular = status;
  d->updateWidget();

  this->resliceWithRuler(d->ReslicingLineNode,
                         d->SliceNode,
                         d->ReslicePerpendicular,
                         d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::onResliceValueChanged(int resliceValue)
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  if (!d->ReslicingLineNode)
    {
    return;
    }

  if (d->ReslicePerpendicular)
    {
    d->ReslicePosition = resliceValue;
    QString decimalValue;
    double distanceValue = d->ReslicingLineNode->GetLineLengthWorld() * d->ReslicePosition / 100;
    decimalValue = decimalValue.setNum(distanceValue, 'f', 2);
    d->ResliceValueLabel->setText(decimalValue);
    }
  else
    {
    d->ResliceAngle = resliceValue;
    d->ResliceValueLabel->setNum(d->ResliceAngle);
    }

  if (d->ResliceButton->isChecked())
    {
    this->resliceWithRuler(d->ReslicingLineNode,
                           d->SliceNode,
                           d->ReslicePerpendicular,
                           d->ReslicePerpendicular ? d->ReslicePosition : d->ResliceAngle);
    }
}

//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget
::resliceWithRuler(vtkMRMLMarkupsLineNode* ruler,
                   vtkMRMLSliceNode* viewer,
                   bool perpendicular,
                   double resliceValue)
{
  if (!ruler || !viewer)
    {
    return;
    }

  // Get ruler points
  double point1[4] = {0,0,0,0};
  double point2[4] = {0,0,0,0};
  ruler->GetNthControlPointPositionWorld(0, point1);
  ruler->GetNthControlPointPositionWorld(1, point2);

  // Compute vectors
  double t[3];
  double n[3];
  double pos[3];
  if (perpendicular)
    {
    // Ruler vector is normal vector
    n[0] = point2[0] - point1[0];
    n[1] = point2[1] - point1[1];
    n[2] = point2[2] - point1[2];

    // Reslice at chosen position
    pos[0] = point1[0] + n[0] * resliceValue / 100;
    pos[1] = point1[1] + n[1] * resliceValue / 100;
    pos[2] = point1[2] + n[2] * resliceValue / 100;

    // Normalize
    double nlen = vtkMath::Normalize(n);
    n[0] /= nlen;
    n[1] /= nlen;
    n[2] /= nlen;

    // angle in radian
    vtkMath::Perpendiculars(n, t, NULL, 0);
    }
  else
    {
    // Ruler vector is transverse vector
    t[0] = point2[0] - point1[0];
    t[1] = point2[1] - point1[1];
    t[2] = point2[2] - point1[2];

    // Reslice at target position
    pos[0] = point2[0];
    pos[1] = point2[1];
    pos[2] = point2[2];

    // Normalize
    double tlen = vtkMath::Normalize(t);
    t[0] /= tlen;
    t[1] /= tlen;
    t[2] /= tlen;

    // angle in radian
    vtkMath::Perpendiculars(t, n, NULL, resliceValue*vtkMath::Pi()/180);
    }

  double nx = n[0];
  double ny = n[1];
  double nz = n[2];
  double tx = t[0];
  double ty = t[1];
  double tz = t[2];
  double px = pos[0];
  double py = pos[1];
  double pz = pos[2];

  viewer->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
}


//-----------------------------------------------------------------------------
void qSlicerPathExplorerReslicingWidget::setSliceNode(vtkMRMLSliceNode* sliceNode)
{
  Q_D(qSlicerPathExplorerReslicingWidget);

  this->setEnabled(sliceNode!=nullptr);
  d->SliceNode = sliceNode;
  
  std::string name;
  double color[3] = { 0.5, 0.5, 0.5 };
  if (sliceNode)
  {
    name = sliceNode->GetName();
    sliceNode->GetLayoutColor(color);
  }
  
  d->ResliceButton->setText(QString::fromStdString(name));
  
  // Set color  
  std::stringstream buttonBgColor;
  buttonBgColor << "QPushButton { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba("
    << color[0] * 255 << ","
    << color[1] * 255 << ","
    << color[2] * 255 << ","
    << "128), stop: 1 rgba("
    << color[0] * 255 << ","
    << color[1] * 255 << ","
    << color[2] * 255 << ","
    << "128)); width:100%; border:1px solid black;} "
    << ":checked { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba("
    << color[0] * 255 << ","
    << color[1] * 255 << ","
    << color[2] * 255 << ","
    << "255), stop: 1 rgba("
    << color[0] * 255 << ","
    << color[1] * 255 << ","
    << color[2] * 255 << ","
    << "255)); width:100%; border:1px solid black;} ";

  d->ResliceButton->setAutoFillBackground(true);
  d->ResliceButton->setStyleSheet(QString::fromStdString(buttonBgColor.str()));
}
