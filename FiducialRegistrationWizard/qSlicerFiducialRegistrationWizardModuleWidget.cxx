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

// SlicerQt includes
#include "qSlicerFiducialRegistrationWizardModuleWidget.h"
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
};



//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidgetPrivate methods


qSlicerFiducialRegistrationWizardModuleWidgetPrivate::qSlicerFiducialRegistrationWizardModuleWidgetPrivate( qSlicerFiducialRegistrationWizardModuleWidget& object ) : q_ptr( &object )
{
}


vtkSlicerFiducialRegistrationWizardLogic* qSlicerFiducialRegistrationWizardModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerFiducialRegistrationWizardModuleWidget );
  return vtkSlicerFiducialRegistrationWizardLogic::SafeDownCast( q->logic() );
}



//-----------------------------------------------------------------------------
// qSlicerFiducialRegistrationWizardModuleWidget methods



qSlicerFiducialRegistrationWizardModuleWidget
::qSlicerFiducialRegistrationWizardModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerFiducialRegistrationWizardModuleWidgetPrivate( *this ) )
{
}



qSlicerFiducialRegistrationWizardModuleWidget
::~qSlicerFiducialRegistrationWizardModuleWidget()
{
}


void qSlicerFiducialRegistrationWizardModuleWidget
::onProbeTransformNodeSelected()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  
  vtkMRMLNode* node = d->ProbeTransformComboBox->currentNode();
  vtkMRMLLinearTransformNode* tNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  
  if( tNode != NULL )
  {
    d->logic()->SetProbeTransformNode( tNode );
  }
}



void qSlicerFiducialRegistrationWizardModuleWidget
::onMarkupsFiducialNodeSelected()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  
  vtkMRMLNode* node = d->MarkupsFiducialComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode* mNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( node );
  
  if( mNode != NULL )
  {
    d->logic()->SetMarkupsFiducialNode( mNode );
  }
}



void qSlicerFiducialRegistrationWizardModuleWidget
::onRecordClicked()
{
  Q_D( qSlicerFiducialRegistrationWizardModuleWidget );
  
  d->logic()->AddFiducial( d->NameBaseLineEdit->text().toStdString() );
}



void qSlicerFiducialRegistrationWizardModuleWidget
::setup()
{
  Q_D(qSlicerFiducialRegistrationWizardModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  d->ProbeTransformComboBox->setNoneEnabled( true );
  d->MarkupsFiducialComboBox->setNoneEnabled( true );
  
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onProbeTransformNodeSelected() ) );
  connect( d->MarkupsFiducialComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsFiducialNodeSelected() ) );
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordClicked() ) );
}

