
// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerOpenIGTLinkRemoteCommandWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteCommandWidget.h"

// Other includes
#include "vtkSlicerOpenIGTLinkCommand.h"
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

  vtkSmartPointer<vtkSlicerOpenIGTLinkCommand> command;
};



//-----------------------------------------------------------------------------
// Private class

//------------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteCommandWidgetPrivate::qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( qSlicerOpenIGTLinkRemoteCommandWidget& object )
  : q_ptr( &object )
{
}

//------------------------------------------------------------------------------
vtkSlicerOpenIGTLinkRemoteLogic * qSlicerOpenIGTLinkRemoteCommandWidgetPrivate::logic()
{
  Q_Q( qSlicerOpenIGTLinkRemoteCommandWidget );
  return vtkSlicerOpenIGTLinkRemoteLogic::SafeDownCast( q->CommandLogic );
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::setup()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  d->setupUi(this);

  d->ShowFullResponseCheckBox->setChecked(false);

  connect( d->SendCommandButton, SIGNAL( clicked() ), this, SLOT( OnSendCommandClicked() ) );
  qvtkConnect(d->command, vtkSlicerOpenIGTLinkCommand::CommandCompletedEvent, this, SLOT(onQueryResponseReceived()));
}


// ==================================================================================
// qSlicerOpenIGTLinkRemoteCommandWidget methods

// Constructor
qSlicerOpenIGTLinkRemoteCommandWidget::qSlicerOpenIGTLinkRemoteCommandWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( *this ) )
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);

  d->command = vtkSmartPointer<vtkSlicerOpenIGTLinkCommand>::New();
  this->setup();
}

//------------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteCommandWidget::~qSlicerOpenIGTLinkRemoteCommandWidget()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  qvtkDisconnect(d->command, vtkSlicerOpenIGTLinkCommand::CommandCompletedEvent, this, SLOT(onQueryResponseReceived()));
  this->setMRMLScene(NULL);
  if ( this->CommandLogic != NULL )
  {
    this->CommandLogic->UnRegister(NULL);
    this->CommandLogic = NULL;
  }
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::OnSendCommandClicked()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);

  // Cancel previous command if it was already in progress
  if (d->command->GetStatus() == vtkSlicerOpenIGTLinkCommand::CommandWaiting)
  {
    qDebug("qSlicerOpenIGTLinkRemoteCommandWidgetPrivate::sendCommand: previous command was already in progress, cancel it now");
    d->logic()->CancelCommand( d->command);
  }

  vtkMRMLNode* connectorNode = d->ConnectorComboBox->currentNode();
  if ( connectorNode == NULL )
  {
    d->ResponseTextEdit->setPlainText( "Connector node not selected!" );
    return;
  }
  
  std::string commandString = d->CommandTextEdit->toPlainText().toStdString();
  if ( commandString.size() < 1 )
  {
    d->ResponseTextEdit->setPlainText( "Please type command XML in the Command field!" );
    return;
  }
  
  // Logic sends command message.
  if (d->command->SetCommandText(d->CommandTextEdit->toPlainText().toStdString().c_str()))
  {
    d->logic()->SendCommand( d->command, connectorNode->GetID());
    onQueryResponseReceived();
  }
  else
  {
    d->ResponseTextEdit->setPlainText( "Command cannot be sent: XML parsing failed" );
  }
  
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::onQueryResponseReceived()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  
  vtkMRMLNode* node = d->ConnectorComboBox->currentNode();
  if ( node == NULL )
  {
    d->ResponseTextEdit->setPlainText( "Connector node not selected!" );
    return;
  }
  
  std::string message;
  std::string parameters;
  int status = d->command->GetStatus();

  QString responseGroupBoxTitle="Response received ("+QString(d->command->GetID())+")";
  d->responseGroupBox->setTitle(responseGroupBoxTitle);
  std::string displayedText = d->command->GetResponseMessage() ? d->command->GetResponseMessage() : "";
  if (d->command->GetResponseXML() == NULL)
  {
    displayedText = d->command->GetResponseText() ? d->command->GetResponseText() : "";
  }
  if (status == vtkSlicerOpenIGTLinkCommand::CommandSuccess)
  {
    d->ResponseTextEdit->setPlainText(QString(displayedText.c_str()));
  }
  else if (status == vtkSlicerOpenIGTLinkCommand::CommandWaiting)
  {
    d->ResponseTextEdit->setPlainText("Waiting for response...");
  }
  else if (status == vtkSlicerOpenIGTLinkCommand::CommandExpired)
  {
    d->ResponseTextEdit->setPlainText("Command timed out");
  }
  else 
  {
    d->ResponseTextEdit->setPlainText("Command failed.\n"+QString(displayedText.c_str()));
  }
  std::string fullResponseText = d->command->GetResponseText() ? d->command->GetResponseText() : "";
  d->FullResponseTextEdit->setPlainText(QString(fullResponseText.c_str()));
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);

  // The scene set in the widget should match the scene set in the logic,
  // log a warning if it is not so.
  // During scene opening or closing it is normal to have one of the scene pointers
  // still/already NULL while the other is non-NULL.
  if ( this->CommandLogic->GetMRMLScene() != newScene
    && newScene != NULL && this->CommandLogic->GetMRMLScene() != NULL)
    {
    qWarning( "Inconsistent MRML scene in OpenIGTLinkRemote logic" );
    }
  
  this->Superclass::setMRMLScene(newScene);
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::setCommandLogic(vtkMRMLAbstractLogic* newCommandLogic)
{
  if ( newCommandLogic == NULL )
  {
    qWarning( "Trying to set NULL as logic" );
    return;
  }
  
  this->CommandLogic = vtkSlicerOpenIGTLinkRemoteLogic::SafeDownCast( newCommandLogic );
  if ( this->CommandLogic != NULL )
  {
    this->CommandLogic->Register(NULL);
  }
  else
  {
    qWarning( "Logic is not an OpenIGTLinkRemoteLogic type!" );
  }
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::setIFLogic(vtkSlicerOpenIGTLinkIFLogic* ifLogic)
{
  this->CommandLogic->SetIFLogic(ifLogic);
}
