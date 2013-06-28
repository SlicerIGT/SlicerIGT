
// Qt includes
#include <QDebug>
#include <QTimer>

// SlicerQt includes
#include "qSlicerOpenIGTLinkRemoteCommandWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteCommandWidget.h"

#include "vtkSlicerOpenIGTLinkRemoteLogic.h"

#include "vtkMRMLNode.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerOpenIGTLinkRemoteCommandWidgetPrivate: public Ui_qSlicerOpenIGTLinkRemoteCommandWidget
{
Q_DECLARE_PUBLIC( qSlicerOpenIGTLinkRemoteCommandWidget );

protected:
  qSlicerOpenIGTLinkRemoteCommandWidget* const q_ptr;

public:
  qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( qSlicerOpenIGTLinkRemoteCommandWidget& object );
  
  vtkSlicerOpenIGTLinkRemoteLogic * logic();
};



//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteCommandWidgetPrivate methods



qSlicerOpenIGTLinkRemoteCommandWidgetPrivate
::qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( qSlicerOpenIGTLinkRemoteCommandWidget& object )
  : q_ptr( &object )
{
}


vtkSlicerOpenIGTLinkRemoteLogic * qSlicerOpenIGTLinkRemoteCommandWidgetPrivate
::logic()
{
  Q_Q( qSlicerOpenIGTLinkRemoteCommandWidget );
  return vtkSlicerOpenIGTLinkRemoteLogic::SafeDownCast( q->logic() );
}

void qSlicerOpenIGTLinkRemoteCommandWidget::setup()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  d->setupUi(this);

  std::cout <<"Hello world" << std::endl;
  connect( d->SendCommandButton, SIGNAL( clicked() ), this, SLOT( OnSendCommandClicked() ) );
  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );
}


//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteCommandWidget methods

// Constructor
qSlicerOpenIGTLinkRemoteCommandWidget
::qSlicerOpenIGTLinkRemoteCommandWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( *this ) )
{
  this->Timer = new QTimer( this );
  this->LastCommandId = 0;
  this->setup();
}

qSlicerOpenIGTLinkRemoteCommandWidget::~qSlicerOpenIGTLinkRemoteCommandWidget()
{
  this->Timer->stop();
}

void qSlicerOpenIGTLinkRemoteCommandWidget
::OnSendCommandClicked()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  
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



void qSlicerOpenIGTLinkRemoteCommandWidget
::OnTimeout()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  
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
