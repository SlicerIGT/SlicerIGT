
// Qt includes
#include <QDebug>
#include <QTimer>

// SlicerQt includes
#include "qSlicerOpenIGTLinkRemoteModuleWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteModuleWidget.h"

#include "vtkSlicerOpenIGTLinkRemoteLogic.h"

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerOpenIGTLinkRemoteModuleWidgetPrivate: public Ui_qSlicerOpenIGTLinkRemoteModuleWidget
{
Q_DECLARE_PUBLIC( qSlicerOpenIGTLinkRemoteModuleWidget );

protected:
  qSlicerOpenIGTLinkRemoteModuleWidget* const q_ptr;

public:
  qSlicerOpenIGTLinkRemoteModuleWidgetPrivate( qSlicerOpenIGTLinkRemoteModuleWidget& object );

};



//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteModuleWidgetPrivate methods



qSlicerOpenIGTLinkRemoteModuleWidgetPrivate
::qSlicerOpenIGTLinkRemoteModuleWidgetPrivate( qSlicerOpenIGTLinkRemoteModuleWidget& object )
  : q_ptr( &object )
{
}

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
}


//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);

  d->queryWidget->setMRMLScene(newScene);
  d->commandWidget->setMRMLScene(newScene);
  
  this->Superclass::setMRMLScene(newScene);
}


void qSlicerOpenIGTLinkRemoteModuleWidget::setIFLogic(vtkSlicerOpenIGTLinkIFLogic *ifLogic)
{
  Q_D(qSlicerOpenIGTLinkRemoteModuleWidget);

  d->queryWidget->setIFLogic(ifLogic);
  d->commandWidget->setIFLogic(ifLogic);
}
