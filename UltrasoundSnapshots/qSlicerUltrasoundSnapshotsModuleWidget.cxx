
// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerUltrasoundSnapshotsModuleWidget.h"
#include "ui_qSlicerUltrasoundSnapshotsModule.h"

#include "vtkSlicerUltrasoundSnapshotsLogic.h"



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
    d->logic()->AddSnapshot( vnode );
  }
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::OnClearSnapshotsClicked()
{
  
}



void
qSlicerUltrasoundSnapshotsModuleWidget
::setup()
{
  Q_D(qSlicerUltrasoundSnapshotsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->AddSnapshotButton, SIGNAL( clicked() ), this, SLOT( OnAddSnapshotClicked() ) );
  connect( d->ClearSnapshotsButton, SIGNAL( clicked() ), this, SLOT( OnClearSnapshotsClicked() ) );
}

