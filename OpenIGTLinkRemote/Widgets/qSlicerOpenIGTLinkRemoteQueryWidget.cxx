/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QList>
#include <QTableWidgetSelectionRange>

// SlicerQt includes
#include "qSlicerOpenIGTLinkRemoteQueryWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteQueryWidget.h"

// Other includes
#include "vtkSlicerOpenIGTLinkIFLogic.h"
#include "vtkSlicerOpenIGTLinkRemoteQueryLogic.h"

#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLIGTLQueryNode.h"
#include "vtkMRMLImageMetaListNode.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IGTLRemote
class qSlicerOpenIGTLinkRemoteQueryWidgetPrivate
  : public Ui_qSlicerOpenIGTLinkRemoteQueryWidget
{
  Q_DECLARE_PUBLIC(qSlicerOpenIGTLinkRemoteQueryWidget);

public:
  qSlicerOpenIGTLinkRemoteQueryWidgetPrivate(qSlicerOpenIGTLinkRemoteQueryWidget&);
  ~qSlicerOpenIGTLinkRemoteQueryWidgetPrivate();

  QButtonGroup typeButtonGroup;
  
  void init();

  enum {
    TYPE_IMAGE,
    TYPE_LABEL,
  };

  vtkMRMLIGTLConnectorNode * connectorNode;

  // TODO: we only have one query node.. it may cause some issue,
  // when there are multiple connectors using the single query Node.
  vtkMRMLIGTLQueryNode * queryNode;
  
protected:
  qSlicerOpenIGTLinkRemoteQueryWidget* const q_ptr;
};

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteQueryWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::
  qSlicerOpenIGTLinkRemoteQueryWidgetPrivate(qSlicerOpenIGTLinkRemoteQueryWidget& object)
  : q_ptr(&object)
{
  this->connectorNode = NULL;
  this->queryNode = NULL;
}

qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::~qSlicerOpenIGTLinkRemoteQueryWidgetPrivate()
{
}


//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::init()
{
  Q_Q(qSlicerOpenIGTLinkRemoteQueryWidget);
  this->setupUi(q);

  this->typeButtonGroup.addButton(this->typeImageRadioButton, qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_IMAGE);
  this->typeButtonGroup.addButton(this->typeLabelRadioButton, qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_LABEL);
  //this->typeButtonGroup.addButton(this->typeAllRadioButton, qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_ALL);
  this->typeImageRadioButton->setChecked(true);

  QObject::connect(this->connectorNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setConnectorNode(vtkMRMLNode*)));
  QObject::connect(this->updateButton, SIGNAL(clicked()),
                   q, SLOT(queryRemoteList()));
  QObject::connect(this->getSelectedItemButton, SIGNAL(clicked()),
                   q, SLOT(querySelectedItem()));

  QStringList list;
  list << QObject::tr("Image ID") << QObject::tr("Patient ID") 
       << QObject::tr("Patient Name") << QObject::tr("Modality")
       << QObject::tr("Time");
  this->remoteDataListTable->setRowCount(15);
  this->remoteDataListTable->setColumnCount(5);
  this->remoteDataListTable->setHorizontalHeaderLabels(list);
  this->remoteDataListTable->verticalHeader()->hide();
  this->remoteDataListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->remoteDataListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  this->remoteDataListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteQueryWidget methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteQueryWidget::qSlicerOpenIGTLinkRemoteQueryWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr( new qSlicerOpenIGTLinkRemoteQueryWidgetPrivate(*this) )
{
  this->QueryLogic = vtkSlicerOpenIGTLinkRemoteQueryLogic::New();
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  d->init();
}


//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteQueryWidget::~qSlicerOpenIGTLinkRemoteQueryWidget()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  if( d->queryNode != NULL )
  {
    d->queryNode->Delete();
  }
}


//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  d->connectorNodeSelector->setMRMLScene(newScene);
  this->QueryLogic->SetMRMLScene(newScene);

  this->Superclass::setMRMLScene(newScene);
}


void qSlicerOpenIGTLinkRemoteQueryWidget::setIFLogic(vtkSlicerOpenIGTLinkIFLogic *ifLogic)
{
  this->QueryLogic->SetIFLogic(ifLogic);
}


//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::setConnectorNode(vtkMRMLNode* node)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  vtkMRMLIGTLConnectorNode* cnode =
    vtkMRMLIGTLConnectorNode::SafeDownCast(node);
  if (cnode)
    {
    d->connectorNode = cnode;
    }
}


//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::queryRemoteList()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return;
    }
  if (!d->connectorNode)
    {
    return;
    }

  if (d->queryNode == NULL)
    {
    d->queryNode = vtkMRMLIGTLQueryNode::New();
    d->queryNode->SetNoNameQuery(1);
    //this->queryNode->SetIGTLName(igtl::ImageMetaMessage::GetDeviceType());
    scene->AddNode(d->queryNode);
    qvtkConnect(d->queryNode, vtkMRMLIGTLQueryNode::ResponseEvent,
                this, SLOT(onQueryResponseReceived()));
    }
  if (d->typeButtonGroup.checkedId() == qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_IMAGE)
    {
    d->queryNode->SetIGTLName("IMGMETA");
    }
  else if (d->typeButtonGroup.checkedId() == qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_LABEL)
    {
    d->queryNode->SetIGTLName("LBMETA");
    }
  d->queryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  d->queryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
  d->connectorNode->PushQuery((vtkMRMLIGTLQueryNode*)d->queryNode);
}


//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::querySelectedItem()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return;
    }
  if (!d->connectorNode)
    {
    return;
    }

  QList<QTableWidgetSelectionRange> selectRanges(d->remoteDataListTable->selectedRanges());
  for (int i =0; i != selectRanges.size(); ++i)
    {
    QTableWidgetSelectionRange range = selectRanges.at(i);
    int top = range.topRow();
    int bottom = range.bottomRow();
    for (int j = top; j <= bottom; ++j)
      {
      QTableWidgetItem *itemDeviceName = d->remoteDataListTable->item(j, 0);
      vtkMRMLIGTLQueryNode* node = vtkMRMLIGTLQueryNode::New();
      node->SetIGTLName("IMAGE");
      node->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
      node->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
      node->SetName(itemDeviceName->text().toAscii());
      //d->ImageQueryNodeList.push_back(node);
      scene->AddNode(node);
      qvtkConnect(node, vtkMRMLIGTLQueryNode::ResponseEvent,
                  this, SLOT(onQueryResponseReceived()));
      d->connectorNode->PushQuery(node);
      }
    }

}


//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::onQueryResponseReceived()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    return;
    }

  if (!d->queryNode)
    {
    return;
    }

  vtkMRMLImageMetaListNode* node 
    = vtkMRMLImageMetaListNode::SafeDownCast(scene->GetNodeByID(d->queryNode->GetResponseDataNodeID()));

  if (node)
    {
    int numImages = node->GetNumberOfImageMetaElement();
    d->remoteDataListTable->setRowCount(numImages);

    for (int i = 0; i < numImages; i ++)
      {
      vtkMRMLImageMetaListNode::ImageMetaElement element;

      node->GetImageMetaElement(i, &element);

      time_t timer = (time_t) element.TimeStamp;
      struct tm *tst = localtime(&timer);
      std::stringstream timess;
      timess << tst->tm_year+1900 << "-" << tst->tm_mon+1 << "-" << tst->tm_mday << " "
             << tst->tm_hour << ":" << tst->tm_min << ":" << tst->tm_sec;

      QTableWidgetItem *deviceItem = new QTableWidgetItem(element.DeviceName.c_str());
      QTableWidgetItem *idItem = new QTableWidgetItem(element.PatientID.c_str());
      QTableWidgetItem *nameItem = new QTableWidgetItem(element.PatientName.c_str());
      QTableWidgetItem *modalityItem = new QTableWidgetItem(element.Modality.c_str());
      QTableWidgetItem *timeItem = new QTableWidgetItem(timess.str().c_str());

      d->remoteDataListTable->setItem(i, 0, deviceItem);
      d->remoteDataListTable->setItem(i, 1, idItem);
      d->remoteDataListTable->setItem(i, 2, nameItem);
      d->remoteDataListTable->setItem(i, 3, modalityItem);
      d->remoteDataListTable->setItem(i, 4, timeItem);
      }
    }
}
