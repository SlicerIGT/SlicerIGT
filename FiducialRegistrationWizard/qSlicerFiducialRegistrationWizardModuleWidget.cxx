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

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QtGui>
#include <QColor>

// SlicerQt includes
#include "qSlicerFiducialRegistrationWizardModuleWidget.h"
#include "qSlicerSimpleMarkupsWidget.h"
#include "qSlicerTransformPreviewWidget.h"
#include "ui_qSlicerFiducialRegistrationWizardModule.h"

#include "vtkSlicerFiducialRegistrationWizardLogic.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_FiducialRegistrationWizard
class qSlicerFiducialRegistrationWizardModuleWidgetPrivate: public Ui_qSlicerFiducialRegistrationWizardModule
{
  Q_DECLARE_PUBLIC( qSlicerFiducialRegistrationWizardModuleWidget ); 
  
protected:
  qSlicerFiducialRegistrationWizardModuleWidget* const q_ptr;
public:
  qSlicerFiducialRegistrationWizardModuleWidgetPrivate( qSlicerFiducialRegistrationWizardModuleWidget& object );
  vtkSlicerFiducialRegistrationWizardLogic* logic() const;

  vtkWeakPointer<vtkMRMLFiducialRegistrationWizardNode> FiducialRegistrationWizardNode;

  // Embedded widgets (in addition to widgets in the .ui file)
  qSlicerTransformPreviewWidget* TransformPreviewWidget;
};

//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidgetPrivate methods

//------------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModuleWidgetPrivate::qSlicerFiducialRegistrationWizardModuleWidgetPrivate( qSlicerFiducialRegistrationWizardModuleWidget& object ) : q_ptr( &object )
{
}

//------------------------------------------------------------------------------
vtkSlicerFiducialRegistrationWizardLogic* qSlicerFiducialRegistrationWizardModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerFiducialRegistrationWizardModuleWidget );
  return vtkSlicerFiducialRegistrationWizardLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidget methods

//------------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModuleWidget
::qSlicerFiducialRegistrationWizardModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerFiducialRegistrationWizardModuleWidgetPrivate( *this ) )
{
}

//------------------------------------------------------------------------------
qSlicerFiducialRegistrationWizardModuleWidget
::~qSlicerFiducialRegistrationWizardModuleWidget()
{
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onRecordFromButtonClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLLinearTransformNode* probeFromTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->ProbeTransformFromComboBox->currentNode() );
  vtkMRMLMarkupsFiducialNode* fromMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->currentNode() );
  d->logic()->AddFiducial( probeFromTransformNode, fromMarkupsNode);
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onRecordToButtonClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );  
  vtkMRMLLinearTransformNode* probeToTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->ProbeTransformToComboBox->currentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->currentNode() );
  d->logic()->AddFiducial( probeToTransformNode, toMarkupsNode);
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onUpdateButtonClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  d->logic()->UpdateCalibration( fiducialRegistrationWizardNode );
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onUpdateButtonToggled(bool checked)
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  UpdateFromMRMLNode(); // this makes sure the button is kept depressed while in auto-update mode
}

//------------------------------------------------------------------------------
std::string qSlicerFiducialRegistrationWizardModuleWidget::GetCorrespondingFiducialString()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkSlicerMarkupsLogic* markupsLogic = d->logic()->GetMarkupsLogic();
  if (markupsLogic==NULL)
  {
    qCritical("qSlicerFiducialRegistrationWizardModuleWidget::GetCorrespondingFiducialString failed: markups logic is invalid");
    return "";
  }

  std::stringstream correspondingFiducialString;
  correspondingFiducialString << "Place fiducial (";

  vtkMRMLMarkupsNode* activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast( this->mrmlScene()->GetNodeByID( markupsLogic->GetActiveListID() ) );

  vtkMRMLMarkupsFiducialNode* fromMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->currentNode() );
  vtkMRMLMarkupsFiducialNode* toMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->currentNode() );

  if ( fromMarkupsNode != NULL && toMarkupsNode != NULL )
  {
    // Adding to "From" list
    if ( strcmp( activeMarkupsNode->GetID(), fromMarkupsNode->GetID() ) == 0 )
    {
      if ( fromMarkupsNode->GetNumberOfFiducials() < toMarkupsNode->GetNumberOfFiducials() )
      {
        correspondingFiducialString << "corresponding to ";
        correspondingFiducialString << toMarkupsNode->GetNthFiducialLabel( fromMarkupsNode->GetNumberOfFiducials() );
      }
      d->ToMarkupsWidget->highlightNthFiducial( fromMarkupsNode->GetNumberOfFiducials() );
    }

    // Adding to "To" list
    if ( strcmp( activeMarkupsNode->GetID(), toMarkupsNode->GetID() ) == 0 )
    {
      if ( toMarkupsNode->GetNumberOfFiducials() < fromMarkupsNode->GetNumberOfFiducials() )
      {
        correspondingFiducialString << "corresponding to ";
        correspondingFiducialString << fromMarkupsNode->GetNthFiducialLabel( toMarkupsNode->GetNumberOfFiducials() );
      }
      d->FromMarkupsWidget->highlightNthFiducial( toMarkupsNode->GetNumberOfFiducials() );
    }

  }

  correspondingFiducialString << ")";
  return correspondingFiducialString.str();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::setup()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);

  d->setupUi(this);
  this->Superclass::setup();

  // Embed widgets here
  d->FromMarkupsWidget->setNodeBaseName( "From" );
  QColor fromMarkupsDefaultColor;
  fromMarkupsDefaultColor.setRgbF(1, 0, 0);
  d->FromMarkupsWidget->setDefaultNodeColor( fromMarkupsDefaultColor );

  d->ToMarkupsWidget->setNodeBaseName( "To" );
  QColor toMarkupsDefaultColor;
  toMarkupsDefaultColor.setRgbF(0, 0, 1);
  d->ToMarkupsWidget->setDefaultNodeColor(toMarkupsDefaultColor);

  d->TransformPreviewWidget = new qSlicerTransformPreviewWidget;
  d->PreviewGroupBox->layout()->addWidget( d->TransformPreviewWidget );

  // Setup Update button menu
  QMenu* updateMenu = new QMenu(tr("Update options"), this);
  // Install event filter to override menu position to show it on the right side of the button.
  // This is necessary because the update button is very wide and the
  // menu arrow is on the right side. With the default QMenu the menu would appear
  // on the left side, which would be very inconvenient because the mouse would need
  // to be moved a lot to click the manual/auto option.
  updateMenu->installEventFilter(this);
  updateMenu->setObjectName("UpdateOptions");
  QActionGroup* updateActions = new QActionGroup(d->UpdateButton);
  updateActions->setExclusive(true);
  updateMenu->addAction(d->ActionAutoUpdate);
  updateActions->addAction(d->ActionAutoUpdate);
  QObject::connect(d->ActionAutoUpdate, SIGNAL(triggered()), this, SLOT(onAutoUpdateSelected()));
  updateMenu->addAction(d->ActionManualUpdate);
  updateActions->addAction(d->ActionManualUpdate);
  QObject::connect(d->ActionManualUpdate, SIGNAL(triggered()), this, SLOT(onManualUpdateSelected()));
  d->UpdateButton->setMenu(updateMenu);

  connect( d->ModuleNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onFiducialRegistrationWizardNodeSelectionChanged()) );

  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->FromMarkupsWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ToMarkupsWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->TransformPreviewWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  // Make connections to update the mrml from the widget
  connect( d->ProbeTransformFromComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->ProbeTransformToComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->OutputTransformComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->RigidRadioButton, SIGNAL(toggled(bool)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->SimilarityRadioButton, SIGNAL(toggled(bool)), this, SLOT(UpdateToMRMLNode()) );
  connect( d->WarpingRadioButton, SIGNAL(toggled(bool)), this, SLOT(UpdateToMRMLNode()) );

  connect( d->FromMarkupsWidget, SIGNAL(markupsFiducialNodeChanged()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->FromMarkupsWidget, SIGNAL(updateFinished()), this, SLOT(PostProcessFromMarkupsWidget()) );
  connect( d->ToMarkupsWidget, SIGNAL(markupsFiducialNodeChanged()), this, SLOT(UpdateToMRMLNode()) );
  connect( d->ToMarkupsWidget, SIGNAL(updateFinished()), this, SLOT(PostProcessToMarkupsWidget()) );

  // These connections will do work (after being updated from the node)
  connect( d->RecordFromButton, SIGNAL(clicked()), this, SLOT(onRecordFromButtonClicked()) );
  connect( d->RecordToButton, SIGNAL(clicked()), this, SLOT(onRecordToButtonClicked()) );
  connect( d->UpdateButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonClicked()) );
  connect( d->UpdateButton, SIGNAL(toggled(bool)), this, SLOT(onUpdateButtonToggled(bool)) );

  this->UpdateFromMRMLNode();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::enter()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);
  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }  

  // Create a module MRML node if there is none in the scene.
  vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLFiducialRegistrationWizardNode");
  if ( node == NULL )
  {
    vtkSmartPointer< vtkMRMLFiducialRegistrationWizardNode > newNode = vtkSmartPointer< vtkMRMLFiducialRegistrationWizardNode >::New();
    this->mrmlScene()->AddNode( newNode );
  }

  node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLFiducialRegistrationWizardNode" );
  if ( node == NULL )
  {
    qCritical( "Failed to create module node vtkMRMLFiducialRegistrationWizardNode" );
    return;
  }

  // For convenience, select a default module.
  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    d->ModuleNodeComboBox->setCurrentNode( node );
  }
  else
  {
    this->UpdateFromMRMLNode();
  }

  this->Superclass::enter();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::UpdateToMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }

  bool allWasBlocked = this->qvtkBlockAll( true );

  fiducialRegistrationWizardNode->SetProbeTransformFromNodeId(d->ProbeTransformFromComboBox->currentNode()?d->ProbeTransformFromComboBox->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetProbeTransformToNodeId(d->ProbeTransformToComboBox->currentNode()?d->ProbeTransformToComboBox->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetOutputTransformNodeId(d->OutputTransformComboBox->currentNode()?d->OutputTransformComboBox->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetAndObserveFromFiducialListNodeId(d->FromMarkupsWidget->currentNode()?d->FromMarkupsWidget->currentNode()->GetID():NULL);
  fiducialRegistrationWizardNode->SetAndObserveToFiducialListNodeId(d->ToMarkupsWidget->currentNode()?d->ToMarkupsWidget->currentNode()->GetID():NULL);

  if (d->RigidRadioButton->isChecked())
  {
    fiducialRegistrationWizardNode->SetRegistrationModeToRigid();
  }
  else if (d->SimilarityRadioButton->isChecked())
  {
    fiducialRegistrationWizardNode->SetRegistrationModeToSimilarity();
  }
  else if (d->WarpingRadioButton->isChecked())
  {
    fiducialRegistrationWizardNode->SetRegistrationModeToWarping();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "Failed to set registration mode, GUI is in invalid state";
  }

  this->qvtkBlockAll(allWasBlocked);

  // The modified event was blocked... Now allow it to happen
  d->ModuleNodeComboBox->currentNode()->Modified();
  this->UpdateFromMRMLNode();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::UpdateFromMRMLNode()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    d->ProbeTransformFromComboBox->setEnabled(false);
    d->ProbeTransformToComboBox->setEnabled(false);
    d->RecordFromButton->setEnabled(false);
    d->RecordToButton->setEnabled(false);
    d->OutputTransformComboBox->setEnabled(false);
    d->RigidRadioButton->setEnabled(false);
    d->SimilarityRadioButton->setEnabled(false);
    d->WarpingRadioButton->setEnabled(false);
    d->FromMarkupsWidget->setEnabled(false);
    d->ToMarkupsWidget->setEnabled(false);
    d->UpdateButton->setEnabled(false);
    d->StatusLabel->setText( "No Fiducial Registration Wizard module node selected." );
    return;
  }

  // Disconnect to prevent signals form triggering events
  bool wasProbeTransformFromComboBoxBlocked = d->ProbeTransformFromComboBox->blockSignals(true);
  bool wasProbeTransformToComboBoxBlocked = d->ProbeTransformToComboBox->blockSignals(true);
  bool wasOutputTransformComboBoxBlocked = d->OutputTransformComboBox->blockSignals(true);
  bool wasRigidRadioButtonBlocked = d->RigidRadioButton->blockSignals(true);
  bool wasSimilarityRadioButtonBlocked = d->SimilarityRadioButton->blockSignals(true);
  bool wasWarpingRadioButtonBlocked = d->WarpingRadioButton->blockSignals(true);
  bool wasActionAutoUpdateBlocked = d->ActionAutoUpdate->blockSignals(true);
  bool wasActionManualUpdateBlocked = d->ActionManualUpdate->blockSignals(true);
  bool wasFromMarkupsWidgetBlocked = d->FromMarkupsWidget->blockSignals(true);
  bool wasToMarkupsWidgetBlocked = d->ToMarkupsWidget->blockSignals(true);

  d->ProbeTransformFromComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetProbeTransformFromNode() );
  d->ProbeTransformToComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetProbeTransformToNode() );
  d->OutputTransformComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetOutputTransformNode() );
  d->TransformPreviewWidget->setCurrentNode( fiducialRegistrationWizardNode->GetOutputTransformNode() );

  d->FromMarkupsWidget->setCurrentNode( fiducialRegistrationWizardNode->GetFromFiducialListNode() );
  d->ToMarkupsWidget->setCurrentNode( fiducialRegistrationWizardNode->GetToFiducialListNode() );

  if ( fiducialRegistrationWizardNode->GetRegistrationMode().compare( "Rigid" ) == 0 )
  {
    d->RigidRadioButton->setChecked( Qt::Checked );
  }
  else if ( fiducialRegistrationWizardNode->GetRegistrationMode().compare( "Similarity" ) == 0 )
  {
    d->SimilarityRadioButton->setChecked( Qt::Checked );
  }
  else if ( fiducialRegistrationWizardNode->GetRegistrationMode().compare( "Warping" ) == 0 )
  {
    d->WarpingRadioButton->setChecked( Qt::Checked );
  }

  if ( fiducialRegistrationWizardNode->GetUpdateMode().compare( "Automatic" ) == 0 )
  {
    d->UpdateButton->setText(tr("Auto-update"));
    d->UpdateButton->setCheckable(true);
    d->UpdateButton->setChecked(true);
    d->ActionAutoUpdate->setChecked( Qt::Checked );
    d->ActionManualUpdate->setChecked( Qt::Unchecked );
  }
  else
  {
    d->UpdateButton->setText(tr("Update"));
    d->UpdateButton->setCheckable(false);
    d->ActionAutoUpdate->setChecked( Qt::Unchecked );
    d->ActionManualUpdate->setChecked( Qt::Checked );
  }

  // Restore signals
  d->ProbeTransformFromComboBox->blockSignals(wasProbeTransformFromComboBoxBlocked);
  d->ProbeTransformToComboBox->blockSignals(wasProbeTransformToComboBoxBlocked);
  d->OutputTransformComboBox->blockSignals(wasOutputTransformComboBoxBlocked);
  d->RigidRadioButton->blockSignals(wasRigidRadioButtonBlocked);
  d->SimilarityRadioButton->blockSignals(wasSimilarityRadioButtonBlocked);
  d->WarpingRadioButton->blockSignals(wasWarpingRadioButtonBlocked);
  d->ActionAutoUpdate->blockSignals(wasActionAutoUpdateBlocked);
  d->ActionManualUpdate->blockSignals(wasActionManualUpdateBlocked);
  d->FromMarkupsWidget->blockSignals(wasFromMarkupsWidgetBlocked);
  d->ToMarkupsWidget->blockSignals(wasToMarkupsWidgetBlocked);

  // From fiducials section
  d->FromMarkupsWidget->setEnabled(true);

  // To fiducials section
  d->ToMarkupsWidget->setEnabled(true);

  // Probe transform section
  d->ProbeTransformFromComboBox->setEnabled(true);
  d->ProbeTransformToComboBox->setEnabled(true);
  d->RecordFromButton->setEnabled(d->ProbeTransformFromComboBox->currentNode()!=NULL);
  d->RecordToButton->setEnabled(d->ProbeTransformToComboBox->currentNode()!=NULL);

  // Results section
  d->OutputTransformComboBox->setEnabled(true);
  d->RigidRadioButton->setEnabled(true);
  d->SimilarityRadioButton->setEnabled(true);
  d->WarpingRadioButton->setEnabled(true);
  d->UpdateButton->setEnabled(true);

  std::stringstream statusString;
  statusString << "Status: ";
  statusString << d->logic()->GetOutputMessage( d->ModuleNodeComboBox->currentNode()->GetID() );
  d->StatusLabel->setText( QString::fromStdString( statusString.str() ) );
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::PostProcessFromMarkupsWidget()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  // Node is added or removed from the markups list, make sure the last item is visible
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->FromMarkupsWidget->currentNode() );
  if ( markupsNode != NULL && markupsNode->GetNumberOfFiducials()>0 )
  {
    d->FromMarkupsWidget->highlightNthFiducial( markupsNode->GetNumberOfFiducials()-1 );
  }
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::PostProcessToMarkupsWidget()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  // Node is added or removed from the markups list, make sure the last item is visible
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->currentNode() );
  if ( markupsNode != NULL && markupsNode->GetNumberOfFiducials()>0 )
  {
    d->ToMarkupsWidget->highlightNthFiducial( markupsNode->GetNumberOfFiducials()-1 );
  }
}

//-----------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onFiducialRegistrationWizardNodeSelectionChanged()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* selectedFiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  qvtkReconnect(d->FiducialRegistrationWizardNode, selectedFiducialRegistrationWizardNode, vtkCommand::ModifiedEvent, this, SLOT(UpdateFromMRMLNode()));
  d->FiducialRegistrationWizardNode = selectedFiducialRegistrationWizardNode;
  this->UpdateFromMRMLNode();
}

//-----------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onAutoUpdateSelected()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }
  fiducialRegistrationWizardNode->SetUpdateMode("Automatic");
}

//-----------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onManualUpdateSelected()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }
  fiducialRegistrationWizardNode->SetUpdateMode("Manual");
}

//-----------------------------------------------------------------------------
bool qSlicerFiducialRegistrationWizardModuleWidget::eventFilter(QObject * obj, QEvent *event)
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  if (event->type() == QEvent::Show && d->UpdateButton != NULL && obj == d->UpdateButton->menu())
  {
    // Show UpdateButton's menu aligned to the right side of the button
    QMenu* menu = d->UpdateButton->menu();
    QPoint menuPos = menu->pos();
    menu->move(menuPos.x()+d->UpdateButton->geometry().width()-menu->geometry().width(), menuPos.y());
    return true;
  }
  return false;
}