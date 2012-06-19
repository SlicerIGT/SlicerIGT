
// Qt includes
#include <QButtonGroup>

#include "qSlicerReslicePropertyWidget.h"
#include "ui_qSlicerReslicePropertyWidget.h"

// MRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDViewControllerWidget.h>

// MRML includes
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"

// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>


class qSlicerReslicePropertyWidgetPrivate;
class vtkMRMLNode;
class vtkObject;


//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_OpenIGTLinkIF
class qSlicerReslicePropertyWidgetPrivate : public Ui_qSlicerReslicePropertyWidget
{
  Q_DECLARE_PUBLIC(qSlicerReslicePropertyWidget);
protected:
  qSlicerReslicePropertyWidget* const q_ptr;
public:
  qSlicerReslicePropertyWidgetPrivate(qSlicerReslicePropertyWidget& object);
  void init();

  QButtonGroup methodButtonGrouop;
  QButtonGroup orientationButtonGroup;

};

//------------------------------------------------------------------------------
qSlicerReslicePropertyWidgetPrivate::qSlicerReslicePropertyWidgetPrivate(qSlicerReslicePropertyWidget& object)
  : q_ptr(&object)
{
  //this->IGTLConnectorNode = 0;
}

//------------------------------------------------------------------------------
void qSlicerReslicePropertyWidgetPrivate::init()
{
  Q_Q(qSlicerReslicePropertyWidget);
  this->setupUi(q);

  //QObject::connect(this->ConnectorNameEdit, SIGNAL(editingFinished()),
  //                 q, SLOT(updateIGTLConnectorNode()));
  //QObject::connect(this->ConnectorStateCheckBox, SIGNAL(toggled(bool)),
  //                 q, SLOT(startCurrentIGTLConnector(bool)));
  //QObject::connect(this->ConnectorHostNameEdit, SIGNAL(editingFinished()),
  //                 q, SLOT(updateIGTLConnectorNode()));
  //QObject::connect(this->ConnectorPortEdit, SIGNAL(editingFinished()),
  //                 q, SLOT(updateIGTLConnectorNode()));
  //QObject::connect(&this->ConnectorTypeButtonGroup, SIGNAL(buttonClicked(int)),
  //                 q, SLOT(updateIGTLConnectorNode()));
  //
  
  this->methodButtonGrouop.addButton(this->positionRadioButton);
  this->methodButtonGrouop.addButton(this->orientationRadioButton);
  this->orientationButtonGroup.addButton(this->inPlaneRadioButton);
  this->orientationButtonGroup.addButton(this->inPlane90RadioButton);
  this->orientationButtonGroup.addButton(this->transverseRadioButton);

}

//------------------------------------------------------------------------------
qSlicerReslicePropertyWidget::qSlicerReslicePropertyWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerReslicePropertyWidgetPrivate(*this))
{
  Q_D(qSlicerReslicePropertyWidget);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerReslicePropertyWidget::~qSlicerReslicePropertyWidget()
{
}

//------------------------------------------------------------------------------
void qSlicerReslicePropertyWidget::onMRMLNodeModified()
{
  Q_D(qSlicerReslicePropertyWidget);
  //if (!d->IGTLConnectorNode)
  //  {
  //  return;
  //  }
  //d->ConnectorNameEdit->setText(d->IGTLConnectorNode->GetName());
  //d->ConnectorHostNameEdit->setText(d->IGTLConnectorNode->GetServerHostname());
  //d->ConnectorPortEdit->setText(QString("%1").arg(d->IGTLConnectorNode->GetServerPort()));
  //int type = d->IGTLConnectorNode->GetType();
  //d->ConnectorNotDefinedRadioButton->setChecked(type == vtkMRMLIGTLConnectorNode::TYPE_NOT_DEFINED);
  //d->ConnectorServerRadioButton->setChecked(type == vtkMRMLIGTLConnectorNode::TYPE_SERVER);
  //d->ConnectorClientRadioButton->setChecked(type == vtkMRMLIGTLConnectorNode::TYPE_CLIENT);
  //
  //setStateEnabled(d, type != vtkMRMLIGTLConnectorNode::TYPE_NOT_DEFINED);
  //
  //bool deactivated = d->IGTLConnectorNode->GetState() == vtkMRMLIGTLConnectorNode::STATE_OFF;
  //if (deactivated)
  //  {
  //  setNameEnabled(d, true);
  //  setTypeEnabled(d, true);
  //  setHostnameEnabled(d, type == vtkMRMLIGTLConnectorNode::TYPE_CLIENT);
  //  setPortEnabled(d, type != vtkMRMLIGTLConnectorNode::TYPE_NOT_DEFINED);
  //  }
  //else
  //  {
  //  setNameEnabled(d, false);
  //  setTypeEnabled(d, false);
  //  setHostnameEnabled(d, false);
  //  setPortEnabled(d, false);
  //  }
  //d->ConnectorStateCheckBox->setChecked(!deactivated);
}

////------------------------------------------------------------------------------
//void qSlicerReslicePropertyWidget::startCurrentIGTLConnector(bool value)
//{
//  Q_D(qSlicerReslicePropertyWidget);
//  Q_ASSERT(d->IGTLConnectorNode);
//  if (value)
//    {
//    d->IGTLConnectorNode->Start();
//    }
//  else
//    {
//    d->IGTLConnectorNode->Stop();
//    }
//}
//
////------------------------------------------------------------------------------
//void qSlicerReslicePropertyWidget::updateIGTLConnectorNode()
//{
//  Q_D(qSlicerReslicePropertyWidget);
//
//  d->IGTLConnectorNode->DisableModifiedEventOn();
//
//  d->IGTLConnectorNode->SetName(d->ConnectorNameEdit->text().toLatin1());
//  d->IGTLConnectorNode->SetType(d->ConnectorTypeButtonGroup.checkedId());
//  d->IGTLConnectorNode->SetServerHostname(d->ConnectorHostNameEdit->text().toStdString());
//  d->IGTLConnectorNode->SetServerPort(d->ConnectorPortEdit->text().toInt());
//
//  d->IGTLConnectorNode->DisableModifiedEventOff();
//  d->IGTLConnectorNode->InvokePendingModifiedEvent();
//}
