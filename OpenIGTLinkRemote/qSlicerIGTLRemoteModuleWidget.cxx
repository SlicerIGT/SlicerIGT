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

// SlicerQt includes
#include "qSlicerIGTLRemoteModuleWidget.h"
#include "ui_qSlicerIGTLRemoteModule.h"

#include "qSlicerApplication.h"

#include <QList>
#include <QTableWidgetSelectionRange>


#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLIGTLQueryNode.h"
#include "vtkMRMLImageMetaListNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IGTLRemote
class qSlicerIGTLRemoteModuleWidgetPrivate: public Ui_qSlicerIGTLRemoteModule
{
public:
  qSlicerIGTLRemoteModuleWidgetPrivate();

public:
  QButtonGroup typeButtonGroup;
  
  enum {
    TYPE_IMAGE,
    TYPE_LABEL,
  };

  vtkMRMLIGTLConnectorNode * connectorNode;

  // TODO: we only have one query node.. it may cause some issue,
  // when there are multiple connectors using the single query Node.
  vtkMRMLIGTLQueryNode * queryNode;
  
};

//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidgetPrivate::qSlicerIGTLRemoteModuleWidgetPrivate()
{
  this->connectorNode = NULL;
  this->queryNode = NULL;
}


//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidget::qSlicerIGTLRemoteModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIGTLRemoteModuleWidgetPrivate )
{
  
}

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidget::~qSlicerIGTLRemoteModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setup()
{
  Q_D(qSlicerIGTLRemoteModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  d->typeButtonGroup.addButton(d->typeImageRadioButton, qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_IMAGE);
  d->typeButtonGroup.addButton(d->typeLabelRadioButton, qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_LABEL);
  //d->typeButtonGroup.addButton(d->typeAllRadioButton, qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_ALL);
  d->typeImageRadioButton->setChecked(true);

  QObject::connect(d->connectorNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(setConnectorNode(vtkMRMLNode*)));
  QObject::connect(d->updateButton, SIGNAL(clicked()),
                   this, SLOT(queryRemoteList()));
  QObject::connect(d->getSelectedItemButton, SIGNAL(clicked()),
                   this, SLOT(querySelectedItem()));

  QStringList list;
  list << tr("Image ID") << tr("Patient ID") << tr("Patient Name") << tr("Modality") << tr("Time");
  d->remoteDataListTable->setRowCount(15);
  d->remoteDataListTable->setColumnCount(5);
  d->remoteDataListTable->setHorizontalHeaderLabels(list);
  d->remoteDataListTable->verticalHeader()->hide();
  d->remoteDataListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  d->remoteDataListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  d->remoteDataListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

}


//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

  vtkMRMLScene* oldScene = this->mrmlScene();

  this->Superclass::setMRMLScene(newScene);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  
  if (oldScene != newScene)
    {
    if (d->connectorNodeSelector)
      {
      d->connectorNodeSelector->setMRMLScene(newScene);
      }
    }

  newScene->InitTraversal();

}


//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setConnectorNode(vtkMRMLNode* node)
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

  vtkMRMLIGTLConnectorNode* cnode =
    vtkMRMLIGTLConnectorNode::SafeDownCast(node);
  if (cnode)
    {
    d->connectorNode = cnode;
    }
}


//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::queryRemoteList()
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

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
  if (d->typeButtonGroup.checkedId() == qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_IMAGE)
    {
    d->queryNode->SetIGTLName("IMGMETA");
    }
  else if (d->typeButtonGroup.checkedId() == qSlicerIGTLRemoteModuleWidgetPrivate::TYPE_LABEL)
    {
    d->queryNode->SetIGTLName("LBMETA");
    }
  d->queryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  d->queryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
  d->connectorNode->PushQuery((vtkMRMLIGTLQueryNode*)d->queryNode);
}


//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::querySelectedItem()
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

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
      //this->ImageQueryNodeList.push_back(node);
      scene->AddNode(node);
      qvtkConnect(node, vtkMRMLIGTLQueryNode::ResponseEvent,
                  this, SLOT(onQueryResponseReceived()));
      d->connectorNode->PushQuery(node);
      }
    }

}


//------------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::onQueryResponseReceived()
{
  Q_D(qSlicerIGTLRemoteModuleWidget);

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
