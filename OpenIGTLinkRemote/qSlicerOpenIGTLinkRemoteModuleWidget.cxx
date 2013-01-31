
// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerOpenIGTLinkRemoteModuleWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteModuleWidget.h"

#include "vtkSlicerOpenIGTLinkRemoteLogic.h"

#include "vtkMRMLNode.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerOpenIGTLinkRemoteModuleWidgetPrivate: public Ui_qSlicerOpenIGTLinkRemoteModuleWidget
{
Q_DECLARE_PUBLIC( qSlicerOpenIGTLinkRemoteModuleWidget );

protected:
  qSlicerOpenIGTLinkRemoteModuleWidget* const q_ptr;

public:
  qSlicerOpenIGTLinkRemoteModuleWidgetPrivate( qSlicerOpenIGTLinkRemoteModuleWidget& object );
  
  vtkSlicerOpenIGTLinkRemoteLogic * logic();
};



//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteModuleWidgetPrivate methods



qSlicerOpenIGTLinkRemoteModuleWidgetPrivate
::qSlicerOpenIGTLinkRemoteModuleWidgetPrivate( qSlicerOpenIGTLinkRemoteModuleWidget& object )
  : q_ptr( &object )
{
}


vtkSlicerOpenIGTLinkRemoteLogic * qSlicerOpenIGTLinkRemoteModuleWidgetPrivate
::logic()
{
  Q_Q( qSlicerOpenIGTLinkRemoteModuleWidget );
  return vtkSlicerOpenIGTLinkRemoteLogic::SafeDownCast( q->logic() );
}



//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteModuleWidget methods



qSlicerOpenIGTLinkRemoteModuleWidget::qSlicerOpenIGTLinkRemoteModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkRemoteModuleWidgetPrivate( *this ) )
{
}



qSlicerOpenIGTLinkRemoteModuleWidget::~qSlicerOpenIGTLinkRemoteModuleWidget()
{
}



void qSlicerOpenIGTLinkRemoteModuleWidget::setup()
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->SendCommandButton, SIGNAL( clicked() ), this, SLOT( OnSendCommandClicked() ) );
}


void qSlicerOpenIGTLinkRemoteModuleWidget
::OnSendCommandClicked()
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);
  
  vtkMRMLNode* node = d->ConnectorComboBox->currentNode();
  if ( node == NULL )
  {
    d->ReplyTextEdit->setPlainText( "Connector node not selected!" );
    return;
  }
  
  
  // Logic sends command message.
  d->logic()->SendCommand( d->CommandTextEdit->toPlainText().toStdString(), node->GetID() );
  
  d->CommandTextEdit->setPlainText( "" );
  d->ReplyTextEdit->setPlainText( "" );
}
