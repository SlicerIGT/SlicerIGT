
// Qt includes
#include <QDebug>
#include <QTimer>

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



// Constructor
qSlicerOpenIGTLinkRemoteModuleWidget
::qSlicerOpenIGTLinkRemoteModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkRemoteModuleWidgetPrivate( *this ) )
{
  this->Timer = new QTimer( this );
  this->LastCommandId = 0;
}



qSlicerOpenIGTLinkRemoteModuleWidget::~qSlicerOpenIGTLinkRemoteModuleWidget()
{
  this->Timer->stop();
}



void qSlicerOpenIGTLinkRemoteModuleWidget::setup()
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->SendCommandButton, SIGNAL( clicked() ), this, SLOT( OnSendCommandClicked() ) );
  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );
}


void qSlicerOpenIGTLinkRemoteModuleWidget
::OnSendCommandClicked()
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);
  
  vtkMRMLNode* node = d->ConnectorComboBox->currentNode();
  if ( node == NULL )
  {
    d->ReplyTextEdit->setPlainText( "Connector node not selected!" );
    this->LastCommandId = 0;
    return;
  }
  
  std::string commandString = d->CommandTextEdit->toPlainText().toStdString();
  if ( commandString.size() < 1 )
  {
    d->ReplyTextEdit->setPlainText( "Please type command in the Command filed!" );
    this->LastCommandId = 0;
    return;
  }
  
  // Logic sends command message.
  this->LastCommandId = d->logic()->SendCommand( d->CommandTextEdit->toPlainText().toStdString(), node->GetID() );
  
  d->CommandTextEdit->setPlainText( "" );
  d->ReplyTextEdit->setPlainText( "" );
  
  this->Timer->start( 100 );
}



void qSlicerOpenIGTLinkRemoteModuleWidget
::OnTimeout()
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);
  
  if ( this->LastCommandId == 0 )
  {
    return;
  }
  
  std::string message;
  int status = d->logic()->GetCommandReply( this->LastCommandId, message );
  
  if ( status == d->logic()->REPLY_WAITING )
  {
    d->ReplyTextEdit->setPlainText( "Waiting for reply..." );
  }
  else
  {
    d->ReplyTextEdit->setPlainText( QString( message.c_str() ) );
  }
  
  if ( status != d->logic()->REPLY_WAITING )
  {
    d->logic()->DiscardCommand( this->LastCommandId );
  }
}
