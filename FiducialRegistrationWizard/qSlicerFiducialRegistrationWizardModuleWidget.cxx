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
#include <QMenu>

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
  if (d->UpdateButton->checkState() == Qt::Checked)
  {
    // If update button is untoggled then make it unchecked, too
    d->UpdateButton->setCheckState(Qt::Unchecked);
  }
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  d->logic()->UpdateCalibration( fiducialRegistrationWizardNode );
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onUpdateButtonCheckboxToggled(bool checked)
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast(d->ModuleNodeComboBox->currentNode());
  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }

  if ( checked )
  {
    fiducialRegistrationWizardNode->SetUpdateModeToAuto();
  }
  else
  {
    fiducialRegistrationWizardNode->SetUpdateModeToManual();
  }
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
      if ( fromMarkupsNode->GetNumberOfControlPoints() < toMarkupsNode->GetNumberOfControlPoints() )
      {
        correspondingFiducialString << "corresponding to ";
        correspondingFiducialString << toMarkupsNode->GetNthFiducialLabel( fromMarkupsNode->GetNumberOfControlPoints() );
      }
      d->ToMarkupsWidget->highlightNthFiducial( fromMarkupsNode->GetNumberOfControlPoints() );
    }

    // Adding to "To" list
    if ( strcmp( activeMarkupsNode->GetID(), toMarkupsNode->GetID() ) == 0 )
    {
      if ( toMarkupsNode->GetNumberOfControlPoints() < fromMarkupsNode->GetNumberOfControlPoints() )
      {
        correspondingFiducialString << "corresponding to ";
        correspondingFiducialString << fromMarkupsNode->GetNthFiducialLabel( toMarkupsNode->GetNumberOfControlPoints() );
      }
      d->FromMarkupsWidget->highlightNthFiducial( toMarkupsNode->GetNumberOfControlPoints() );
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

  // Setup PointMatching ComboBox options
  d->PointMatchingComboBox->addItem( tr( vtkMRMLFiducialRegistrationWizardNode::PointMatchingAsString( vtkMRMLFiducialRegistrationWizardNode::POINT_MATCHING_MANUAL ).c_str() ) );
  d->PointMatchingComboBox->setItemData( 0, tr( "Point indices in the \"From\" list match those in the \"To\" list." ), Qt::ToolTipRole );
  d->PointMatchingComboBox->addItem( tr( vtkMRMLFiducialRegistrationWizardNode::PointMatchingAsString( vtkMRMLFiducialRegistrationWizardNode::POINT_MATCHING_AUTOMATIC ).c_str() ) );
  d->PointMatchingComboBox->setItemData( 1, tr( "EXPERIMENTAL. Point pairing between the two lists will be computed automatically. This feature is intended only for rigid and similarity transforms." ), Qt::ToolTipRole );

  // Setup Update button menu
  QMenu* updateMenu = new QMenu(tr("Update options"), this);
  // Install event filter to override menu position to show it on the right side of the button.
  // This is necessary because the update button is very wide and the
  // menu arrow is on the right side. With the default QMenu the menu would appear
  // on the left side, which would be very inconvenient because the mouse would need
  // to be moved a lot to click the manual/auto option.
  updateMenu->installEventFilter(this);
  updateMenu->setObjectName("UpdateOptions");
  connect( d->ModuleNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onParameterNodeSelected()) );

  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->FromMarkupsWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ToMarkupsWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );
  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->TransformPreviewWidget, SLOT(setMRMLScene(vtkMRMLScene*)) );

  // from/to markups widgets need to have scene set before this module widget,
  // otherwise they will run into errors
  d->FromMarkupsWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  d->ToMarkupsWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  // Make connections to update the mrml from the widget
  connect( d->PointMatchingComboBox, SIGNAL( currentIndexChanged(int)), this, SLOT(updateMRMLFromGUI()) );
  connect( d->ProbeTransformFromComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromGUI()) );
  connect( d->ProbeTransformToComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromGUI()) );
  connect( d->OutputTransformComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromGUI()) );
  connect( d->RigidRadioButton, SIGNAL(toggled(bool)), this, SLOT(updateMRMLFromGUI()) );
  connect( d->SimilarityRadioButton, SIGNAL(toggled(bool)), this, SLOT(updateMRMLFromGUI()) );
  connect( d->WarpingRadioButton, SIGNAL(toggled(bool)), this, SLOT(updateMRMLFromGUI()) );

  connect( d->FromMarkupsWidget, SIGNAL(markupsFiducialNodeChanged()), this, SLOT(updateMRMLFromGUI()) );
  connect( d->FromMarkupsWidget, SIGNAL(updateFinished()), this, SLOT(PostProcessFromMarkupsWidget()) );
  connect( d->ToMarkupsWidget, SIGNAL(markupsFiducialNodeChanged()), this, SLOT(updateMRMLFromGUI()) );
  connect( d->ToMarkupsWidget, SIGNAL(updateFinished()), this, SLOT(PostProcessToMarkupsWidget()) );

  // These connections will do work (after being updated from the node)
  connect( d->RecordFromButton, SIGNAL(clicked()), this, SLOT(onRecordFromButtonClicked()) );
  connect( d->RecordToButton, SIGNAL(clicked()), this, SLOT(onRecordToButtonClicked()) );
  connect( d->UpdateButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonClicked()) );
  connect( d->UpdateButton, SIGNAL(checkBoxToggled(bool)), this, SLOT(onUpdateButtonCheckboxToggled(bool)));
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::enter()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);
  this->Superclass::enter();

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }


  // Create a module MRML node if there is none in the scene.
  // For convenience, select a default module.
  if ( d->ModuleNodeComboBox->currentNode() == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLFiducialRegistrationWizardNode");
    if (node == NULL)
    {
      node = this->mrmlScene()->AddNewNodeByClass("vtkMRMLFiducialRegistrationWizardNode");
    }

    if ( node == NULL )
    {
      qCritical( "Failed to create module node vtkMRMLFiducialRegistrationWizardNode" );
      return;
    }

    d->ModuleNodeComboBox->setCurrentNode( node );
  }
 
  // Need to update the GUI so that it observes whichever parameter node is selected
  this->onParameterNodeSelected();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::updateMRMLFromGUI()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  if ( fiducialRegistrationWizardNode == NULL )
  {
    return;
  }

  bool allWasBlocked = this->qvtkBlockAll( true );

  std::string pointMatchingAsString = d->PointMatchingComboBox->currentText().toStdString();
  int pointMatchingAsEnum = vtkMRMLFiducialRegistrationWizardNode::PointMatchingFromString( pointMatchingAsString );
  fiducialRegistrationWizardNode->SetPointMatching( pointMatchingAsEnum );

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
  this->updateGUIFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::updateGUIFromMRML()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );

  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );

  if ( fiducialRegistrationWizardNode == NULL )
  {
    d->PointMatchingComboBox->setEnabled(false);
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
  bool wasPointMatchingComboBoxBlocked = d->PointMatchingComboBox->blockSignals(true);
  bool wasProbeTransformFromComboBoxBlocked = d->ProbeTransformFromComboBox->blockSignals(true);
  bool wasProbeTransformToComboBoxBlocked = d->ProbeTransformToComboBox->blockSignals(true);
  bool wasOutputTransformComboBoxBlocked = d->OutputTransformComboBox->blockSignals(true);
  bool wasRigidRadioButtonBlocked = d->RigidRadioButton->blockSignals(true);
  bool wasSimilarityRadioButtonBlocked = d->SimilarityRadioButton->blockSignals(true);
  bool wasWarpingRadioButtonBlocked = d->WarpingRadioButton->blockSignals(true);
  bool wasFromMarkupsWidgetBlocked = d->FromMarkupsWidget->blockSignals(true);
  bool wasToMarkupsWidgetBlocked = d->ToMarkupsWidget->blockSignals(true);

  int pointMatchingIndex = d->PointMatchingComboBox->findText( QString( vtkMRMLFiducialRegistrationWizardNode::PointMatchingAsString( fiducialRegistrationWizardNode->GetPointMatching() ).c_str() ) );
  if ( pointMatchingIndex < 0 )
  {
    pointMatchingIndex = 0;
  }
  d->PointMatchingComboBox->setCurrentIndex( pointMatchingIndex );

  d->ProbeTransformFromComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetProbeTransformFromNode() );
  d->ProbeTransformToComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetProbeTransformToNode() );
  d->OutputTransformComboBox->setCurrentNode( fiducialRegistrationWizardNode->GetOutputTransformNode() );
  d->TransformPreviewWidget->setCurrentNode( fiducialRegistrationWizardNode->GetOutputTransformNode() );

  d->FromMarkupsWidget->setCurrentNode( fiducialRegistrationWizardNode->GetFromFiducialListNode() );
  d->ToMarkupsWidget->setCurrentNode( fiducialRegistrationWizardNode->GetToFiducialListNode() );

  if ( fiducialRegistrationWizardNode->GetRegistrationMode() == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_RIGID )
  {
    d->RigidRadioButton->setChecked( true );
  }
  else if ( fiducialRegistrationWizardNode->GetRegistrationMode() == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_SIMILARITY )
  {
    d->SimilarityRadioButton->setChecked( true );
  }
  else if ( fiducialRegistrationWizardNode->GetRegistrationMode() == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_WARPING )
  {
    d->WarpingRadioButton->setChecked( true );
  }

  if ( fiducialRegistrationWizardNode->GetUpdateMode() == vtkMRMLFiducialRegistrationWizardNode::UPDATE_MODE_AUTOMATIC )
  {
    bool wasBlocked = d->UpdateButton->blockSignals(true);
    d->UpdateButton->setText(tr("Auto-update"));
    d->UpdateButton->setCheckable(true);
    d->UpdateButton->setChecked(true);
    d->UpdateButton->blockSignals(wasBlocked);
  }
  else
  {
    bool wasBlocked = d->UpdateButton->blockSignals(true);
    d->UpdateButton->setText(tr("Update"));
    d->UpdateButton->setCheckable(false);
    d->UpdateButton->blockSignals(wasBlocked);
  }

  // Restore signals
  d->PointMatchingComboBox->blockSignals(wasPointMatchingComboBoxBlocked);
  d->ProbeTransformFromComboBox->blockSignals(wasProbeTransformFromComboBoxBlocked);
  d->ProbeTransformToComboBox->blockSignals(wasProbeTransformToComboBoxBlocked);
  d->OutputTransformComboBox->blockSignals(wasOutputTransformComboBoxBlocked);
  d->RigidRadioButton->blockSignals(wasRigidRadioButtonBlocked);
  d->SimilarityRadioButton->blockSignals(wasSimilarityRadioButtonBlocked);
  d->WarpingRadioButton->blockSignals(wasWarpingRadioButtonBlocked);
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
  d->PointMatchingComboBox->setEnabled(true);
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
  if ( markupsNode != NULL && markupsNode->GetNumberOfControlPoints()>0 )
  {
    d->FromMarkupsWidget->highlightNthFiducial( markupsNode->GetNumberOfControlPoints()-1 );
  }
}

//------------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::PostProcessToMarkupsWidget()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  // Node is added or removed from the markups list, make sure the last item is visible
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( d->ToMarkupsWidget->currentNode() );
  if ( markupsNode != NULL && markupsNode->GetNumberOfControlPoints()>0 )
  {
    d->ToMarkupsWidget->highlightNthFiducial( markupsNode->GetNumberOfControlPoints()-1 );
  }
}

//-----------------------------------------------------------------------------
void qSlicerFiducialRegistrationWizardModuleWidget::onParameterNodeSelected()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  vtkMRMLFiducialRegistrationWizardNode* pNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( d->ModuleNodeComboBox->currentNode() );
  qvtkReconnect(d->FiducialRegistrationWizardNode, pNode, vtkCommand::ModifiedEvent, this, SLOT(updateGUIFromMRML()));
  d->FiducialRegistrationWizardNode = pNode;
  this->updateGUIFromMRML();
}
