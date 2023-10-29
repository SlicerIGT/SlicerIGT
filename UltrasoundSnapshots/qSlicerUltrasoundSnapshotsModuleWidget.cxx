
// Qt includes
#include <QDebug>
#include <QMessageBox>

// SlicerQt includes
#include "qSlicerUltrasoundSnapshotsModuleWidget.h"
#include "ui_qSlicerUltrasoundSnapshotsModule.h"
#include <qSlicerApplication.h>

#include "vtkSlicerUltrasoundSnapshotsLogic.h"

#include "vtkMRMLScene.h"



//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerUltrasoundSnapshotsModuleWidgetPrivate: public Ui_qSlicerUltrasoundSnapshotsModule
{
  Q_DECLARE_PUBLIC( qSlicerUltrasoundSnapshotsModuleWidget );
protected:
  qSlicerUltrasoundSnapshotsModuleWidget* const q_ptr;
public:
  qSlicerUltrasoundSnapshotsModuleWidgetPrivate( qSlicerUltrasoundSnapshotsModuleWidget& object );
  vtkSlicerUltrasoundSnapshotsLogic* logic() const;
};


//-----------------------------------------------------------------------------
// qSlicerUltrasoundSnapshotsModuleWidgetPrivate methods


//-----------------------------------------------------------------------------
qSlicerUltrasoundSnapshotsModuleWidgetPrivate::qSlicerUltrasoundSnapshotsModuleWidgetPrivate( qSlicerUltrasoundSnapshotsModuleWidget& object )
 : q_ptr( &object )
{
}


vtkSlicerUltrasoundSnapshotsLogic*
qSlicerUltrasoundSnapshotsModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerUltrasoundSnapshotsModuleWidget );
  return vtkSlicerUltrasoundSnapshotsLogic::SafeDownCast( q->logic() );
}




//-----------------------------------------------------------------------------
// qSlicerUltrasoundSnapshotsModuleWidget methods



qSlicerUltrasoundSnapshotsModuleWidget
::qSlicerUltrasoundSnapshotsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerUltrasoundSnapshotsModuleWidgetPrivate( *this ) )
{
}



qSlicerUltrasoundSnapshotsModuleWidget
::~qSlicerUltrasoundSnapshotsModuleWidget()
{
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::OnInputSelectionChanged( vtkMRMLNode* currentNode )
{
  Q_D(qSlicerUltrasoundSnapshotsModuleWidget);
  
  if ( currentNode == NULL )
  {
    return;
  }
  
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast( currentNode );
  if ( volumeNode == NULL )
  {
    return;
  }
  
  d->logic()->SetInputVolumeNode( volumeNode );
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::OnAddSnapshotClicked()
{
  Q_D(qSlicerUltrasoundSnapshotsModuleWidget);
  
  vtkMRMLNode* cnode = d->UltrasoundImageComboBox->currentNode();
  vtkMRMLScalarVolumeNode* vnode = NULL;
  if ( cnode != NULL )
  {
    vnode = vtkMRMLScalarVolumeNode::SafeDownCast( cnode );
  }
  
  if ( vnode != NULL )
  {
    d->logic()->AddSnapshot( vnode, d->WindowLevelCheckBox->isChecked() );
  }
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::OnClearSnapshotsClicked()
{
  Q_D(qSlicerUltrasoundSnapshotsModuleWidget);
  
  QMessageBox *confirmBox =  new QMessageBox();
  confirmBox->setText(tr("This action will delete all snapshots."));
  confirmBox->setInformativeText(tr("Continue with this action?"));
  confirmBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  int confirmChoice = confirmBox->exec();
  
  switch (confirmChoice) {
   case QMessageBox::Yes:
       d->logic()->ClearSnapshots();
       break;
   case QMessageBox::No:
       break;
   default:
       break;
  }
  
  delete confirmBox;
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::enter()
{
  Q_D( qSlicerUltrasoundSnapshotsModuleWidget );
  
  d->UltrasoundImageComboBox->setCurrentNode( d->logic()->GetInputVolumeNode() );
  
  this->Superclass::enter();
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::setup()
{
  Q_D(qSlicerUltrasoundSnapshotsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->UltrasoundImageComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( OnInputSelectionChanged( vtkMRMLNode* ) ) );
  connect( d->AddSnapshotButton, SIGNAL( clicked() ), this, SLOT( OnAddSnapshotClicked() ) );
  connect( d->ClearSnapshotsButton, SIGNAL( clicked() ), this, SLOT( OnClearSnapshotsClicked() ) );
}

