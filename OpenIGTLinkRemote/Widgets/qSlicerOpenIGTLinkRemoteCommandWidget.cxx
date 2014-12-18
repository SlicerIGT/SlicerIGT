
// Qt includes
#include <QDebug>
#include <QTimer>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerOpenIGTLinkRemoteCommandWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteCommandWidget.h"

// Other includes
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
  return vtkSlicerOpenIGTLinkRemoteLogic::SafeDownCast( q->CommandLogic );
}


void qSlicerOpenIGTLinkRemoteCommandWidget::setup()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  d->setupUi(this);

  connect( d->SendCommandButton, SIGNAL( clicked() ), this, SLOT( OnSendCommandClicked() ) );
  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );
}


// qSlicerOpenIGTLinkRemoteCommandWidget methods

// Constructor
qSlicerOpenIGTLinkRemoteCommandWidget
::qSlicerOpenIGTLinkRemoteCommandWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( *this ) )
{
  qSlicerAbstractCoreModule* remoteModule = qSlicerApplication::application()->moduleManager()->module( "OpenIGTLinkRemote" );
  if ( remoteModule != NULL )
  {
    this->CommandLogic = vtkSlicerOpenIGTLinkRemoteLogic::SafeDownCast( remoteModule->logic() );
    this->CommandLogic->Register( this );
  }
  else
  {
    this->CommandLogic = NULL;
    qWarning( "OpenIGTLinkRemote module logic not found!" );
  }
  
  this->Timer = new QTimer( this );
  this->LastCommandId = 0;
  this->setup();
}


qSlicerOpenIGTLinkRemoteCommandWidget
::~qSlicerOpenIGTLinkRemoteCommandWidget()
{
  this->Timer->stop();
  if ( this->CommandLogic != NULL )
  {
    this->CommandLogic->UnRegister( this );
    this->CommandLogic = NULL;
  }
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
    d->ReplyTextEdit->setPlainText( "Please type command in the Command field!" );
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

  vtkMRMLNode* node = d->ConnectorComboBox->currentNode();
  if ( node == NULL )
  {
    d->ReplyTextEdit->setPlainText( "Connector node not selected!" );
    this->LastCommandId = 0;
    return;
  }
  
  std::string message;
  std::string parameters;
  int status = d->logic()->GetCommandReply( this->LastCommandId, message, parameters );

  QString replyGroupBoxTitle="Reply received (id: "+QString::number(this->LastCommandId)+")";
  d->replyGroupBox->setTitle(replyGroupBoxTitle);
    
  if ( status == vtkSlicerOpenIGTLinkRemoteLogic::REPLY_WAITING )
  {
    d->ReplyTextEdit->setPlainText( "Waiting for reply..." );
  }
  else
  {
    std::string displayedText=message;
    if (!parameters.empty())
    {
      displayedText+=" ("+parameters+")";
    }
    d->ReplyTextEdit->setPlainText( QString( displayedText.c_str() ) );
  }
  
  if ( status != vtkSlicerOpenIGTLinkRemoteLogic::REPLY_WAITING )
  {
    d->logic()->DiscardCommand( this->LastCommandId, node->GetID() );
    this->Timer->stop();
    this->LastCommandId = 0;
  }
}


void qSlicerOpenIGTLinkRemoteCommandWidget
::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  
  
  if ( this->CommandLogic->GetMRMLScene() != newScene )
  {
    qWarning( "Incosistent MRML scene in OpenIGTLinkRemote logic" );
  }
  
  
  this->Superclass::setMRMLScene(newScene);
}


void qSlicerOpenIGTLinkRemoteCommandWidget
::setCommandLogic(vtkSlicerOpenIGTLinkRemoteLogic* newCommandLogic)
{
  if ( this->CommandLogic != newCommandLogic )
  {
    this->CommandLogic = newCommandLogic;
  }
}


void qSlicerOpenIGTLinkRemoteCommandWidget
::setIFLogic(vtkSlicerOpenIGTLinkIFLogic* ifLogic)
{
  this->CommandLogic->SetIFLogic(ifLogic);
}
