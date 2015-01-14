/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QMenu>
#include <QInputDialog>
#include <QToolButton>
#include <QLabel>
#include <QList>

// qMRML includes
#include "qMRMLWatchdogToolBar.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLNodeFactory.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qMRMLWatchdogToolBarPrivate
{
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLWatchdogToolBar);
protected:
  qMRMLWatchdogToolBar* const q_ptr;
public:
  qMRMLWatchdogToolBarPrivate(qMRMLWatchdogToolBar& object);
  void init();
  void setMRMLScene(vtkMRMLScene* newScene);
  QList<QAction*>*                         ActionsListPtr;
  //QAction*                         SceneViewAction;
  //qMRMLSceneViewMenu*              SceneViewMenu;

  // TODO In LayoutManager, use GetActive/IsActive flag ...
  vtkWeakPointer<vtkMRMLViewNode>  ActiveMRMLThreeDViewNode;
  vtkSmartPointer<vtkMRMLScene>    MRMLScene;

public slots:
  void OnMRMLSceneStartBatchProcessing();
  void OnMRMLSceneEndBatchProcessing();
  void updateWidgetFromMRML();
  void createSceneView();
};

//--------------------------------------------------------------------------
// qMRMLWatchdogToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLWatchdogToolBarPrivate::qMRMLWatchdogToolBarPrivate(qMRMLWatchdogToolBar& object)
  : q_ptr(&object)
{
  this->ActionsListPtr = NULL;
  //this->SceneViewAction = 0;
  //this->SceneViewMenu = 0;
}

// --------------------------------------------------------------------------
void qMRMLWatchdogToolBarPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLWatchdogToolBar);
  // Enable buttons
  q->setEnabled(this->MRMLScene != 0);
  //this->LabelsList->setEnabled(this->ActiveMRMLThreeDViewNode != 0);
}

//---------------------------------------------------------------------------
void qMRMLWatchdogToolBarPrivate::init()
{
  Q_Q(qMRMLWatchdogToolBar);

  // Screenshot button
  if(this->ActionsListPtr == NULL)
  {
    this->ActionsListPtr = new QList<QAction*>;
    QLabel* transformLabel = new QLabel(q);
    transformLabel->setToolTip(q->tr("Each square indicates the state of the tools watched by the Watchdog module"));
    transformLabel->setText("Tools watched:");
    //this->LabelsList->setIcon(QIcon(":/Icons/Watchdog.png"));
    this->ActionsListPtr->push_back(q->addWidget(transformLabel));
    //QObject::connect(this->LabelsListPtr, SIGNAL(triggered()),
    //                 q, SIGNAL(screenshotButtonClicked()));
    
  }
  //// Scene View buttons
  //this->SceneViewAction = new QAction(q);
  //this->SceneViewAction->setIcon(QIcon(":/Icons/ViewCamera.png"));
  //this->SceneViewAction->setText(q->tr("Scene view"));
  //this->SceneViewAction->setToolTip(q->tr("Watchdog and name a scene view."));
  //QObject::connect(this->SceneViewAction, SIGNAL(triggered()),
  //                 q, SIGNAL(sceneViewButtonClicked()));
  //q->addAction(this->SceneViewAction);

  //// Scene view menu
  //QToolButton* sceneViewMenuButton = new QToolButton(q);
  //sceneViewMenuButton->setText(q->tr("Restore view"));
  //sceneViewMenuButton->setIcon(QIcon(":/Icons/ViewCameraSelect.png"));
  //sceneViewMenuButton->setToolTip(QObject::tr("Restore or delete saved scene views."));
  //this->SceneViewMenu = new qMRMLSceneViewMenu(sceneViewMenuButton);
  //sceneViewMenuButton->setMenu(this->SceneViewMenu);
  //sceneViewMenuButton->setPopupMode(QToolButton::InstantPopup);
  ////QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
  ////                 this->SceneViewMenu, SLOT(setMRMLScene(vtkMRMLScene*)));
  //q->addWidget(sceneViewMenuButton);
  //QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
  //                sceneViewMenuButton,
  //                SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
}
// --------------------------------------------------------------------------
void qMRMLWatchdogToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  if (newScene == this->MRMLScene)
    {
    return;
    }
/*
  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent,
                      this, SLOT(OnMRMLSceneStartBatchProcessing()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent,
                      this, SLOT(OnMRMLSceneEndBatchProcessing()));

*/
  this->MRMLScene = newScene;

  //this->SceneViewMenu->setMRMLScene(newScene);

  // Update UI
  this->updateWidgetFromMRML();
}


// --------------------------------------------------------------------------
void qMRMLWatchdogToolBarPrivate::OnMRMLSceneStartBatchProcessing()
{
  Q_Q(qMRMLWatchdogToolBar);
  q->setEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLWatchdogToolBarPrivate::OnMRMLSceneEndBatchProcessing()
{
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLWatchdogToolBarPrivate::createSceneView()
{
  Q_Q(qMRMLWatchdogToolBar);

  // Ask user for a name
  bool ok = false;
  QString sceneViewName = QInputDialog::getText(q, QObject::tr("SceneView Name"),
                                                QObject::tr("SceneView Name:"), QLineEdit::Normal,
                                                "View", &ok);
  if (!ok || sceneViewName.isEmpty())
    {
    return;
    }

  // Create scene view
  qMRMLNodeFactory nodeFactory;
  nodeFactory.setMRMLScene(this->MRMLScene);
  nodeFactory.setBaseName("vtkMRMLSceneViewNode", sceneViewName);
  vtkMRMLNode * newNode = nodeFactory.createNode("vtkMRMLSceneViewNode");
  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(newNode);
  newSceneViewNode->StoreScene();
}

// --------------------------------------------------------------------------
// qMRMLWatchdogToolBar methods

// --------------------------------------------------------------------------
qMRMLWatchdogToolBar::qMRMLWatchdogToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qMRMLWatchdogToolBarPrivate(*this))
{
  Q_D(qMRMLWatchdogToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLWatchdogToolBar::qMRMLWatchdogToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLWatchdogToolBarPrivate(*this))
{
  Q_D(qMRMLWatchdogToolBar);
  d->init();
}

void qMRMLWatchdogToolBar
::SetFirstlabel(char * watchDogNodeName)
{
  Q_D(qMRMLWatchdogToolBar);
  QString tooltip("Each square indicates the state of the tools watched by the Watchdog module: ");
  tooltip.append(watchDogNodeName);
  QLabel* firstLabel = (QLabel*)this->widgetForAction(d->ActionsListPtr->at(0));
  firstLabel->setToolTip( tooltip );
  firstLabel->setText(QString(watchDogNodeName));
}

void qMRMLWatchdogToolBar
::ToolNodeAdded(const char * label)
{
  Q_D(qMRMLWatchdogToolBar);

  QLabel* toolLabel = new QLabel(this);
  toolLabel->setToolTip(this->tr("Tool in row %1").arg(d->ActionsListPtr->size()));
  toolLabel->setText(QString(label).left(6));

  toolLabel->setAlignment(Qt::AlignCenter);
  toolLabel->setStyleSheet("QLabel { background-color: green; min-width: 2em; max-height: 2em;}");
  d->ActionsListPtr->push_back(this->addWidget(toolLabel));
  //QObject::connect(this->LabelsListPtr, SIGNAL(triggered()),
  //                 this, SIGNAL(screenshotButtonClicked()));
  
}
void qMRMLWatchdogToolBar
::SwapToolNodes(int toolA, int toolB )
{
  Q_D(qMRMLWatchdogToolBar);

  //d->ActionsListPtr->swap(toolA+1,toolB+1);
  QLabel* toolLabelA = (QLabel*)this->widgetForAction(d->ActionsListPtr->at(toolA+1));
  QLabel* toolLabelB = (QLabel*)this->widgetForAction(d->ActionsListPtr->at(toolB+1));
  QString TempLabel = toolLabelA->text();
  toolLabelA->setText(toolLabelB->text());
  toolLabelB->setText(TempLabel);
  //this->widgetForAction(d->ActionsListPtr->at(toolA+1))->setIconText(this->widgetForAction(d->ActionsListPtr->at(toolB+1))->iconText());
  //this->widgetForAction(d->ActionsListPtr->at(toolB+1))->SetIconText(toolLabel);

}




void qMRMLWatchdogToolBar
::ToolNodeDeleted()
{
  Q_D(qMRMLWatchdogToolBar);
  this->removeAction(d->ActionsListPtr->back());
  d->ActionsListPtr->pop_back();
}


void qMRMLWatchdogToolBar
::DeleteToolNode(int row)
{
  Q_D(qMRMLWatchdogToolBar);
  this->removeAction(d->ActionsListPtr->at(row+1));
  d->ActionsListPtr->removeAt(row+1);
}



void qMRMLWatchdogToolBar
::SetNodeStatus(int row, bool status )
{
  Q_D(qMRMLWatchdogToolBar);
  if(d->ActionsListPtr!= NULL)
  {
    if(d->ActionsListPtr->size()>1&&row+1<d->ActionsListPtr->size())
    {
      if(status)
      {
        this->widgetForAction(d->ActionsListPtr->at(row+1))->setStyleSheet("QLabel { background-color: rgb(45,224,90); min-width: 2em; max-height: 2em;}");
      }
      else
      {
        this->widgetForAction(d->ActionsListPtr->at(row+1))->setStyleSheet("QLabel { background-color: red; min-width: 2em; max-height: 2em;}");
      }
    }
  }
  return;
  
  //QObject::connect(this->LabelsListPtr, SIGNAL(triggered()),
  //                 this, SIGNAL(screenshotButtonClicked()));
  //this->addWidget(d->LabelsListPtr->back());
}

void qMRMLWatchdogToolBar
::SetNodeLabel(int row,const char * toolLabel)
{
  Q_D(qMRMLWatchdogToolBar);
  if(d->ActionsListPtr!= NULL)
  {
    if(d->ActionsListPtr->size()>1&&row+1<d->ActionsListPtr->size())
    {
      QLabel* label= (QLabel*)(this->widgetForAction(d->ActionsListPtr->at(row+1)));
      label->setText(QString(toolLabel));
    }
  }
  return;
}


//---------------------------------------------------------------------------
qMRMLWatchdogToolBar::~qMRMLWatchdogToolBar()
{
}

// --------------------------------------------------------------------------
void qMRMLWatchdogToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLWatchdogToolBar);
  d->setMRMLScene(scene);
}

// --------------------------------------------------------------------------
void qMRMLWatchdogToolBar::setActiveMRMLThreeDViewNode(
  vtkMRMLViewNode * newActiveMRMLThreeDViewNode)
{
  Q_D(qMRMLWatchdogToolBar);
  d->ActiveMRMLThreeDViewNode = newActiveMRMLThreeDViewNode;
  d->updateWidgetFromMRML();
}
