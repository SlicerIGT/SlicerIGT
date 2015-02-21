
// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerOpenIGTLinkRemoteCommandWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteCommandWidget.h"

// Other includes
#include "vtkSlicerOpenIGTLinkRemoteLogic.h"
#include "vtkMRMLIGTLQueryNode.h"

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

  vtkWeakPointer<vtkMRMLIGTLQueryNode> commandQueryNode;
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

  connect( d->SendCommandButton, SIGNAL( clicked() ), this, SLOT( OnSendCommandClicked() ) );
}


// ==================================================================================
// qSlicerOpenIGTLinkRemoteCommandWidget methods

// Constructor
qSlicerOpenIGTLinkRemoteCommandWidget::qSlicerOpenIGTLinkRemoteCommandWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkRemoteCommandWidgetPrivate( *this ) )
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);

  d->commandQueryNode = NULL;
  this->setup();
}

//------------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteCommandWidget::~qSlicerOpenIGTLinkRemoteCommandWidget()
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
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
  d->logic()->SendCommandXML( d->commandQueryNode, connectorNode->GetID(), d->CommandTextEdit->toPlainText().toStdString().c_str());
  
  //d->CommandTextEdit->setPlainText( "" );
  onQueryResponseReceived();
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
  int status = d->logic()->GetCommandResponse( d->commandQueryNode, message, parameters );  

  QString responseGroupBoxTitle="Response received ("+QString(d->commandQueryNode->GetIGTLDeviceName())+")";
  d->responseGroupBox->setTitle(responseGroupBoxTitle);
  std::string displayedText=message;
  if (!parameters.empty())
  {
    displayedText+=" ("+parameters+")";
  }    
  if ( status == vtkSlicerOpenIGTLinkRemoteLogic::COMMAND_SUCCESS )
  {
    d->ResponseTextEdit->setPlainText( QString( displayedText.c_str() ) );
  }
  else if ( status == vtkSlicerOpenIGTLinkRemoteLogic::COMMAND_WAITING )
  {
    d->ResponseTextEdit->setPlainText( "Waiting for response..." );
  }
  else 
  {
    d->ResponseTextEdit->setPlainText( "Command failed.\n"+QString( displayedText.c_str() ) );
  }
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteCommandWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerOpenIGTLinkRemoteCommandWidget);
  
  if (this->mrmlScene() && this->mrmlScene() != newScene)
  {
    qvtkDisconnect(d->commandQueryNode, vtkMRMLIGTLQueryNode::ResponseEvent, this, SLOT(onQueryResponseReceived()));
    d->logic()->DeleteCommandQueryNode(d->commandQueryNode);
    d->commandQueryNode = NULL;
  }
  if (newScene)
  {
    d->commandQueryNode = d->logic()->CreateCommandQueryNode();
    qvtkConnect(d->commandQueryNode, vtkMRMLIGTLQueryNode::ResponseEvent, this, SLOT(onQueryResponseReceived()));
  }
  
  if ( this->CommandLogic->GetMRMLScene() != newScene )
  {
    qWarning( "Incosistent MRML scene in OpenIGTLinkRemote logic" );
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
