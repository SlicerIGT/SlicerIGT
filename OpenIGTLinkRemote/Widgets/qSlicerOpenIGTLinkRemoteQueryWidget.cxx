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
#include <QDebug>
#include <QList>
#include <QModelIndexList>
#include <QTableWidgetSelectionRange>
#include <QTimer>

// SlicerQt includes
#include "qSlicerOpenIGTLinkRemoteQueryWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteQueryWidget.h"

// Other includes
#include "vtkSlicerOpenIGTLinkIFLogic.h"

#include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLIGTLQueryNode.h"
#include "vtkMRMLImageMetaListNode.h"
#include "vtkMRMLLabelMetaListNode.h"
//#include "vtkMRMLPointMetaListNode.h" TODO: fix this by not relying on vtkMRMLPointMetaListNode
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLNode.h"

#include "vtkSmartPointer.h"

#include <map>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IGTLRemote
class qSlicerOpenIGTLinkRemoteQueryWidgetPrivate
  : public Ui_qSlicerOpenIGTLinkRemoteQueryWidget
{
  Q_DECLARE_PUBLIC(qSlicerOpenIGTLinkRemoteQueryWidget);

public:
  qSlicerOpenIGTLinkRemoteQueryWidgetPrivate(qSlicerOpenIGTLinkRemoteQueryWidget&);
  ~qSlicerOpenIGTLinkRemoteQueryWidgetPrivate();

public:
  QButtonGroup typeButtonGroup;

  void init();
  void updateButtonsState();
  void clearMetadata();
  void requestImage(std::string name);
  void addMetadataQueryNodeToScene();

  enum
  {
    TYPE_IMAGE,
    TYPE_LABEL,
    TYPE_POINT,
  };

  vtkWeakPointer<vtkMRMLIGTLConnectorNode> connectorNode;

  // We only have one query node. It means that this query can only have one metadata query
  // (get list of images, points, etc.) in progress.
  vtkSmartPointer<vtkMRMLIGTLQueryNode> metadataQueryNode;

  // List of query node IDs corresponding to in-progress data transfer (image data, etc.)
  std::vector<std::string> inProgressDataQueryNodeIds;

  std::map<std::string, std::string> imageDeviceNameToNodeNameMap;
  std::map<std::string, std::string> labelDeviceNameToNodeNameMap;

protected:
  qSlicerOpenIGTLinkRemoteQueryWidget* const q_ptr;
};

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteQueryWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::
qSlicerOpenIGTLinkRemoteQueryWidgetPrivate(qSlicerOpenIGTLinkRemoteQueryWidget& object)
: q_ptr(&object)
, metadataQueryNode(vtkSmartPointer<vtkMRMLIGTLQueryNode>::New())
{
  metadataQueryNode->SetName("OpenIGTLinkRemoteMetadataQueryNode");
  metadataQueryNode->SetSaveWithScene(false); // this is temporary data only
  metadataQueryNode->SetHideFromEditors(true);
}

//-----------------------------------------------------------------------------
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
  this->typeButtonGroup.addButton(this->typePointRadioButton, qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_POINT);

  // set up table
  this->remoteDataListTable->setColumnCount(5);
  this->remoteDataListTable->verticalHeader()->hide();
  this->remoteDataListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->remoteDataListTable->setSelectionMode(QAbstractItemView::MultiSelection);
  this->remoteDataListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

  QObject::connect(this->connectorNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setConnectorNode(vtkMRMLNode*)));
  QObject::connect(this->updateButton, SIGNAL(clicked()), q, SLOT(queryRemoteList()));
  QObject::connect(this->getSelectedItemButton, SIGNAL(clicked()), q, SLOT(querySelectedItem()));
  QObject::connect(&typeButtonGroup, SIGNAL(buttonClicked(int)), q, SLOT(onQueryTypeChanged(int)));
  QObject::connect(this->trackingSTTButton, SIGNAL(clicked()), q, SLOT(startTracking()));
  QObject::connect(this->trackingSTPButton, SIGNAL(clicked()), q, SLOT(stopTracking()));
  QObject::connect(this->remoteDataListTable, SIGNAL(itemSelectionChanged()), q, SLOT(onRemoteDataListSelectionChanged()));

  // set to default query type
  this->typeImageRadioButton->click();
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::updateButtonsState()
{
  this->trackingSTTButton->setEnabled(this->connectorNode!=NULL);
  this->trackingSTPButton->setEnabled(this->connectorNode!=NULL);
  this->updateButton->setEnabled(this->connectorNode!=NULL);
  this->getSelectedItemButton->setEnabled(this->connectorNode!=NULL && this->remoteDataListTable->selectedRanges().size()>0);
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::clearMetadata()
{
  this->remoteDataListTable->setRowCount(0);
  this->imageDeviceNameToNodeNameMap.clear();
  this->labelDeviceNameToNodeNameMap.clear();
  this->updateButtonsState();
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::addMetadataQueryNodeToScene()
{
  Q_Q(qSlicerOpenIGTLinkRemoteQueryWidget);
  if (!q->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid scene";
    return;
  }
  if (q->mrmlScene()->IsNodePresent(this->metadataQueryNode))
  {
    // already in the scene
    return;
  }
  q->mrmlScene()->AddNode(this->metadataQueryNode);
}

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteQueryWidget methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteQueryWidget::qSlicerOpenIGTLinkRemoteQueryWidget(QWidget* _parent)
: Superclass(_parent)
, d_ptr( new qSlicerOpenIGTLinkRemoteQueryWidgetPrivate(*this) )
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  d->init();
  qvtkConnect(d->metadataQueryNode, vtkMRMLIGTLQueryNode::ResponseEvent,
    this, SLOT(onMetadataQueryResponseReceived()));
}

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteQueryWidget::~qSlicerOpenIGTLinkRemoteQueryWidget()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  qvtkDisconnect(d->metadataQueryNode, vtkMRMLIGTLQueryNode::ResponseEvent,
    this, SLOT(onMetadataQueryResponseReceived()));
  if (d->connectorNode)
  {
    qvtkDisconnect(d->connectorNode, vtkMRMLIGTLConnectorNode::NewDeviceEvent,
      this, SLOT(onNewDeviceAdded(vtkObject*, void*)));
  }
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  if (this->mrmlScene() == newScene)
  {
    // no change
    return;
  }

  if (this->mrmlScene())
  {
    // Switching scene
    // Remove metadata query node from the old scene
    this->mrmlScene()->RemoveNode(d->metadataQueryNode);
    // Remove all data query nodes
    for (std::vector<std::string>::iterator remIter = d->inProgressDataQueryNodeIds.begin();
      remIter != d->inProgressDataQueryNodeIds.end(); remIter++)
    {
      vtkMRMLIGTLQueryNode* node = vtkMRMLIGTLQueryNode::SafeDownCast(this->mrmlScene()->GetNodeByID((*remIter).c_str()));
      if (node)
      {
        qvtkDisconnect(node, vtkMRMLIGTLQueryNode::ResponseEvent, this, SLOT(onDataQueryResponseReceived(vtkObject*, void*)));
        this->mrmlScene()->RemoveNode(node);
      }
    }
    d->inProgressDataQueryNodeIds.clear();
  }

  d->clearMetadata();

  d->connectorNodeSelector->setMRMLScene(newScene);

  this->Superclass::setMRMLScene(newScene);
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::setIFLogic(vtkSlicerOpenIGTLinkIFLogic* vtkNotUsed(ifLogic))
{
  vtkNotUsed(ifLogic);
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::setConnectorNode(vtkMRMLNode* node)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  vtkMRMLIGTLConnectorNode* cnode = vtkMRMLIGTLConnectorNode::SafeDownCast(node);
  if (d->connectorNode == cnode)
  {
    // no change
    if (d->connectorNode==NULL)
    {
      // We get here when the scene is closed
      d->clearMetadata();
      d->updateButtonsState();
      // when a scene is closed data query node IDs become outdated, make sure they are cleared
      // (deleteCompletedDataQueryNodes does not touch valid, in-progress query nodes)
      deleteCompletedDataQueryNodes();
    }
    return;
  }

  qvtkReconnect(d->connectorNode, cnode, vtkMRMLIGTLConnectorNode::NewDeviceEvent,
    this, SLOT(onNewDeviceAdded(vtkObject*, void*)));
  d->connectorNode = cnode;

  d->clearMetadata();
  d->updateButtonsState();
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::queryRemoteList()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  if (!d->connectorNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid connector node";
    return;
  }

  if (d->typeButtonGroup.checkedId() == qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_IMAGE)
  {
    d->metadataQueryNode->SetIGTLName("IMGMETA");
  }
  else if (d->typeButtonGroup.checkedId() == qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_LABEL)
  {
    d->metadataQueryNode->SetIGTLName("LBMETA");
  }
  else if (d->typeButtonGroup.checkedId() == qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_POINT)
  {
    d->metadataQueryNode->SetIGTLName("POINT");
  }
  d->metadataQueryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  d->metadataQueryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
  d->addMetadataQueryNodeToScene();
  d->connectorNode->PushQuery((vtkMRMLIGTLQueryNode*)d->metadataQueryNode);
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::startTracking()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  qDebug(Q_FUNC_INFO);
  if (!d->connectorNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid connector node";
    return;
  }
  d->metadataQueryNode->SetIGTLName("TDATA");
  d->metadataQueryNode->SetIGTLDeviceName("");
  d->metadataQueryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  d->metadataQueryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_START);
  d->addMetadataQueryNodeToScene();
  d->connectorNode->PushQuery((vtkMRMLIGTLQueryNode*)d->metadataQueryNode);
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::stopTracking()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  qDebug(Q_FUNC_INFO);
  if (!d->connectorNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid connector node";
    return;
  }
  d->metadataQueryNode->SetIGTLName("TDATA");
  d->metadataQueryNode->SetIGTLDeviceName("");
  d->metadataQueryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  d->metadataQueryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_STOP);
  d->addMetadataQueryNodeToScene();
  d->connectorNode->PushQuery((vtkMRMLIGTLQueryNode*)d->metadataQueryNode);
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::onRemoteDataListSelectionChanged()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  d->updateButtonsState();
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::querySelectedItem()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  QList<QTableWidgetSelectionRange> selectRange(d->remoteDataListTable->selectedRanges());
  for (int selectionIndex=0; selectionIndex < selectRange.size(); selectionIndex++)
  {
    int topRowIndex = selectRange.at(selectionIndex).topRow();
    // Get the item identifier from the table
    std::string dataId( d->remoteDataListTable->item(topRowIndex, 0)->text().toLatin1() );
    switch (d->typeButtonGroup.checkedId()) 
    {
    case qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_IMAGE:
    case qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_LABEL:
      this->getImage( dataId );
      break;
    case qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_POINT:
      this->getPointList( dataId );
      break;
    default:
      qCritical() << Q_FUNC_INFO << " failed: unknown item type selected";
    }
  }
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::onNewDeviceAdded(vtkObject* vtkNotUsed(connectorNode), void* deviceNode)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  // change the node name to match the full descriptive name from the ImageMetaList or LabelMetaList
  vtkMRMLScalarVolumeNode* imageNode = vtkMRMLScalarVolumeNode::SafeDownCast((vtkObject*)deviceNode);
  vtkMRMLLabelMapVolumeNode* labelNode = vtkMRMLLabelMapVolumeNode::SafeDownCast((vtkObject*)deviceNode);
  std::string name;
  if (imageNode)
  {  
    if (d->imageDeviceNameToNodeNameMap.find(imageNode->GetName())!=d->imageDeviceNameToNodeNameMap.end())
    {
      name = d->imageDeviceNameToNodeNameMap[imageNode->GetName()];
    }
  }
  else if (labelNode)
  {
    if (d->labelDeviceNameToNodeNameMap.find(labelNode->GetName())!=d->labelDeviceNameToNodeNameMap.end())
    {
      name = d->labelDeviceNameToNodeNameMap[labelNode->GetName()];
    }
  }
  else
  {
    // node metadata not found
    return;
  }

  // Rename the node (give a unique name if the node is retrieved already)
  if (imageNode->GetScene())
  {
    name = imageNode->GetScene()->GetUniqueNameByString(name.c_str());
  }
  imageNode->SetName(name.c_str());
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::onMetadataQueryResponseReceived()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);

  vtkMRMLNode* qNode = d->metadataQueryNode->GetResponseDataNode();
  vtkMRMLImageMetaListNode* imgQueryNode = imgQueryNode = vtkMRMLImageMetaListNode::SafeDownCast(qNode); 
  vtkMRMLLabelMetaListNode* lbQueryNode = vtkMRMLLabelMetaListNode::SafeDownCast(qNode);
  //  vtkMRMLPointMetaListNode* ptQueryNode = vtkMRMLPointMetaListNode::SafeDownCast(qNode); TODO: fix this by not relying on vtkMRMLPointMetaListNode

  if (imgQueryNode)
  {
    d->imageDeviceNameToNodeNameMap.clear();
    int numImages = imgQueryNode->GetNumberOfImageMetaElement();
    d->remoteDataListTable->setRowCount(numImages);
    for (int i = 0; i < numImages; i++)
    {
      vtkMRMLImageMetaListNode::ImageMetaElement element;

      imgQueryNode->GetImageMetaElement(i, &element);

      time_t timer = (time_t) element.TimeStamp;
      struct tm *tst = localtime(&timer);
      std::stringstream timess;
      if (tst)
      {
        timess << tst->tm_year + 1900 << "-" << tst->tm_mon + 1 << "-" << tst->tm_mday << " "
          << tst->tm_hour << ":" << tst->tm_min << ":" << tst->tm_sec;
      }
      else
      {
        // this can be null if element.TimeStamp is invalid
        qWarning() << Q_FUNC_INFO << ": Received invalid timestamp in ImageMeta message";
        timess << "(N/A)";
      }

      QTableWidgetItem *deviceItem = new QTableWidgetItem(element.DeviceName.c_str());
      QTableWidgetItem *nameItem = new QTableWidgetItem(element.Name.c_str());
      QTableWidgetItem *patientIdItem = new QTableWidgetItem(element.PatientID.c_str());
      QTableWidgetItem *patientNameItem = new QTableWidgetItem(element.PatientName.c_str());
      QTableWidgetItem *modalityItem = new QTableWidgetItem(element.Modality.c_str());
      QTableWidgetItem *timeItem = new QTableWidgetItem(timess.str().c_str());

      d->remoteDataListTable->setItem(i, 0, deviceItem);
      d->remoteDataListTable->setItem(i, 1, nameItem);
      d->remoteDataListTable->setItem(i, 2, patientIdItem);
      d->remoteDataListTable->setItem(i, 3, patientNameItem);
      d->remoteDataListTable->setItem(i, 4, modalityItem);
      d->remoteDataListTable->setItem(i, 5, timeItem);

      d->imageDeviceNameToNodeNameMap[element.DeviceName] = element.Name;
    }
  }
  else if (lbQueryNode)
  {
    d->labelDeviceNameToNodeNameMap.clear();
    size_t numLabels = lbQueryNode->GetNumberOfLabelMetaElement();
    d->remoteDataListTable->setRowCount(numLabels);
    for (size_t i = 0; i < numLabels; i++)
    {
      vtkMRMLLabelMetaListNode::LabelMetaElement element;

      lbQueryNode->GetLabelMetaElement(i, &element);

      QTableWidgetItem *deviceItem = new QTableWidgetItem(element.DeviceName.c_str());
      QTableWidgetItem *nameItem = new QTableWidgetItem(element.Name.c_str());
      QTableWidgetItem *ownerItem = new QTableWidgetItem(element.Owner.c_str());

      d->remoteDataListTable->setItem(i, 0, deviceItem);
      d->remoteDataListTable->setItem(i, 1, nameItem);
      d->remoteDataListTable->setItem(i, 2, ownerItem);

      d->labelDeviceNameToNodeNameMap[element.DeviceName] = element.Name;
    }
  }
  /* TODO: fix this by not relying on vtkMRMLPointMetaListNode
  else if(ptQueryNode)
  {
  std::vector<std::string> ptGroupIds;
  ptQueryNode->GetPointGroupNames(ptGroupIds);
  d->remoteDataListTable->setRowCount(ptGroupIds.size());
  for (unsigned int i = 0; i < ptGroupIds.size(); i++)
  d->remoteDataListTable->setItem(i, 0, new QTableWidgetItem(ptGroupIds[i].c_str()));
  }
  */
  d->updateButtonsState();
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::getImage(std::string name)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  if (!this->mrmlScene())
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid scene node";
    return;
  }

  vtkSmartPointer<vtkMRMLIGTLQueryNode> node = vtkSmartPointer<vtkMRMLIGTLQueryNode>::New();
  node->SetSaveWithScene(false); // this is temporary data only
  node->SetHideFromEditors(true);
  node->SetIGTLName("IMAGE");
  node->SetIGTLDeviceName(name.c_str());
  node->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
  node->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
  this->mrmlScene()->AddNode(node);
  d->inProgressDataQueryNodeIds.push_back(node->GetID());

  // Request callback so the QueryNode can be removed.
  qvtkConnect(node, vtkMRMLIGTLQueryNode::ResponseEvent,
    this, SLOT(onDataQueryResponseReceived(vtkObject*, void*)));

  d->connectorNode->PushQuery(node);
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::onDataQueryResponseReceived(vtkObject* qNode, void* vtkNotUsed(nonode))
{
  // Request deletion of completed data query nodes (next time the application becomes idle)
  // We must not delete the completed data query node immediately here, because we may delete the query node
  // that invoked this callback function (and deleting a VTK object that is currently invoking an event may cause crash).
  QTimer::singleShot(0, this, SLOT(deleteCompletedDataQueryNodes()));
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::deleteCompletedDataQueryNodes()
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
  {
    d->inProgressDataQueryNodeIds.clear();
    return;
  }
  // Delete all the query nodes that are successfully completed
  for (std::vector<std::string>::iterator remIter = d->inProgressDataQueryNodeIds.begin();
    remIter != d->inProgressDataQueryNodeIds.end(); )
  {
    vtkMRMLIGTLQueryNode* node = vtkMRMLIGTLQueryNode::SafeDownCast(scene->GetNodeByID((*remIter).c_str()));
    if (node && (node != d->metadataQueryNode) &&
      (node->GetQueryStatus() == vtkMRMLIGTLQueryNode::STATUS_SUCCESS) )
    {
      qvtkDisconnect(node, vtkMRMLIGTLQueryNode::ResponseEvent,
        this, SLOT(onDataQueryResponseReceived(vtkObject*, void*)));
      scene->RemoveNode(node);
      remIter = d->inProgressDataQueryNodeIds.erase(remIter);
    }
    else if (!node)
    {
      // the node has been deleted from the scene already, remove from our list, too
      remIter = d->inProgressDataQueryNodeIds.erase(remIter);
    }
    else
    {
      remIter++;
    }
  }
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::getPointList(std::string id)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  /* TODO: fix this by not relying on vtkMRMLPointMetaListNode
  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid scene node";
    return;
  }

  typedef vtkMRMLPointMetaListNode::PointMetaElement ElemType;
  vtkMRMLNode* qNode = d->metadataQueryNode->GetResponseDataNode();
  vtkMRMLPointMetaListNode* ptQueryNode = vtkMRMLPointMetaListNode::SafeDownCast(qNode);

  if(!ptQueryNode)
  {
  std::cerr << "qSlicerOpenIGTLinkRemoteQueryWidget: Query result for " << id
  << " is no longer available!" << std::endl;
  return;
  }
  std::vector<ElemType>::iterator elemIter;
  std::vector<ElemType> elements;
  ptQueryNode->GetPointGroup(id, elements);
  if (elements.size() == 0)
  return;

  // annotation parent setup
  vtkMRMLAnnotationHierarchyNode* parent;
  {
  parent = vtkMRMLAnnotationHierarchyNode::New();
  parent->HideFromEditorsOff();
  parent->SetName(this->mrmlScene()->GetUniqueNameByString(id.c_str()));
  this->mrmlScene()->AddNode(parent);
  }

  for (elemIter = elements.begin(); elemIter != elements.end(); elemIter++)
  {
  vtkMRMLAnnotationHierarchyNode* fiduNode = vtkMRMLAnnotationHierarchyNode::New();
  {
  fiduNode->SetParentNodeID(parent->GetID());
  fiduNode->AllowMultipleChildrenOff();
  }

  vtkMRMLAnnotationFiducialNode* fidu = vtkMRMLAnnotationFiducialNode::New();
  {
  fidu->SetDisableModifiedEvent(1);
  fidu->SetName( this->mrmlScene()->GetUniqueNameByString(elemIter->Name.c_str()) );
  float* pos = elemIter->Position;
  fidu->SetFiducialCoordinates((double)pos[0], (double)pos[1], (double)pos[2]);
  fidu->Initialize(this->mrmlScene());
  fidu->SetDisableModifiedEvent(0);
  fiduNode->SetDisplayableNodeID(fidu->GetID());
  }
  this->mrmlScene()->InsertAfterNode(fidu, fiduNode);
  fidu->Delete();
  fiduNode->Delete();
  }
  parent->Delete();
  */
}

//------------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteQueryWidget::onQueryTypeChanged(int id)
{
  Q_D(qSlicerOpenIGTLinkRemoteQueryWidget);
  d->remoteDataListTable->clearContents();
  QStringList list;
  switch(id)
  {
  case qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_IMAGE:
    list << QObject::tr("Image ID") << QObject::tr("Image Name")
      << QObject::tr("Patient ID") << QObject::tr("Patient Name")
      << QObject::tr("Modality") << QObject::tr("Time");
    break;
  case qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_LABEL:
    list << QObject::tr("Image ID") << QObject::tr("Image Name")
      << QObject::tr("Owner Image") << QObject::tr("")
      << QObject::tr("");
    break;
  case qSlicerOpenIGTLinkRemoteQueryWidgetPrivate::TYPE_POINT:
    list << QObject::tr("Group ID") << QObject::tr("") 
      << QObject::tr("") << QObject::tr("")
      << QObject::tr("");
    break;
  }
  d->remoteDataListTable->setHorizontalHeaderLabels(list);
}
