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
#include <QDebug>
#include <QMenu>
#include <QtGui>
#include <QTimer>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qMRMLWatchdogToolBar.h"

// qMRML includes
#include "qSlicerToolBarManagerWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLWatchdogNode.h"
#include "vtkSlicerWatchdogLogic.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>


// --------------------------------------------------------------------------
// qSlicerToolBarManagerWidget methods

// --------------------------------------------------------------------------
qSlicerToolBarManagerWidget::qSlicerToolBarManagerWidget(QWidget* _parent)
: Superclass(_parent), WatchdogSound(NULL)
{
  this->WatchdogToolBarHash=NULL;
  this->Timer = new QTimer( this );
  this->StatusRefreshTimeSec=0.2;
  this->LastSoundElapsedTime=2.0;
}

//---------------------------------------------------------------------------
qSlicerToolBarManagerWidget::~qSlicerToolBarManagerWidget()
{
  this->Timer->stop();
  assert(this->Superclass::mrmlScene() != 0);
  this->qvtkDisconnect(this->Superclass::mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT(RemoveToolBar(vtkObject*, vtkObject*)));
  this->qvtkDisconnect(this->Superclass::mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT(AddToolBar(vtkObject*, vtkObject*)));

  disconnect( this->Timer, SIGNAL( timeout() ), this, SLOT( onTimerEvent() ) );
  this->qvtkDisconnect(this->WatchdogLogic, vtkSlicerWatchdogLogic::WatchdogLogicUpdatedEvent, this, SLOT(onUpdateToolBars()));

}

QHash<QString, qMRMLWatchdogToolBar *> * qSlicerToolBarManagerWidget::GetToolBarHash()
{
  return this->WatchdogToolBarHash;
}

// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::setSound(std::string watchdogModuleShareDirectory)
{
  if(this->WatchdogSound==NULL)
  {
    this->WatchdogSound=new QSound( QDir::toNativeSeparators( QString::fromStdString( watchdogModuleShareDirectory+"/alarmWatchdog.wav" ) ) );
    this->LastSoundElapsedTime=2.0;
  }
}

void qSlicerToolBarManagerWidget::setStatusRefreshTimeSec(double statusRefreshTimeSec)
{
  if(statusRefreshTimeSec != 0)
  {
    this->StatusRefreshTimeSec=statusRefreshTimeSec;
  }
}

// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  this->Superclass::setMRMLScene( newScene );

  this->qvtkConnect(newScene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(RemoveToolBar(vtkObject*, vtkObject*)));
  this->qvtkConnect(newScene, vtkMRMLScene::NodeAddedEvent, this, SLOT(AddToolBar(vtkObject*, vtkObject*)));

}

// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::setLogic(vtkSlicerWatchdogLogic* watchdogLogic)
{
  this->WatchdogLogic=watchdogLogic;
  this->setSound(watchdogLogic->GetModuleShareDirectory());
  this->setStatusRefreshTimeSec(watchdogLogic->GetStatusRefreshTimeSec());
  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( onTimerEvent() ) );
  this->qvtkConnect(watchdogLogic, vtkSlicerWatchdogLogic::WatchdogLogicUpdatedEvent, this, SLOT(onUpdateToolBars()));
}

// --------------------------------------------------------------------------
void  qSlicerToolBarManagerWidget::onTimerEvent()
{
  this->WatchdogLogic->UpdateWatchdogNodes();
}

// --------------------------------------------------------------------------
void  qSlicerToolBarManagerWidget::onUpdateToolBars()
{
  if(this->WatchdogToolBarHash==NULL)
  {
    return;
  }
  this->LastSoundElapsedTime = this->LastSoundElapsedTime+this->StatusRefreshTimeSec;

  vtkCollection * watchdogNodes  = this->Superclass::mrmlScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
  vtkSmartPointer<vtkCollectionIterator> watchdogNodeIt = vtkSmartPointer<vtkCollectionIterator>::New();
  watchdogNodeIt->SetCollection( watchdogNodes );

  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
    if(this->WatchdogToolBarHash->contains(QString(watchdogNode->GetID())) && this->WatchdogToolBarHash->value(QString(watchdogNode->GetID()))->isVisible())
    {
      std::list<WatchedTool>* toolsVectorPtr = watchdogNode->GetToolNodes();
      int numberTools = toolsVectorPtr->size();
      //qDebug() << "update toolbars watchnode list number of tools " <<numberTools;

      if ( toolsVectorPtr == NULL /*|| numberTools!= d->ToolsTableWidget->rowCount()*/)
      {
        return;
      }
      int row=0;
      for (std::list<WatchedTool>::iterator itTool = toolsVectorPtr->begin() ; itTool != toolsVectorPtr->end(); ++itTool)
      {
        if((*itTool).tool==NULL)
        {
          return;
        }
        this->WatchdogToolBarHash->value(QString(watchdogNode->GetID()))->SetNodeStatus(row,(*itTool).status);

        if(this->LastSoundElapsedTime>=2.0)
          if((*itTool).playSound && !(*itTool).status)
          {
            this->LastSoundElapsedTime=0;
            if(this->WatchdogSound->isFinished())
            {
              this->WatchdogSound->setLoops(1);
              this->WatchdogSound->play();
            }
          }
          //if((*itTool).sound && (*itTool).status)
          //{
          //  this->BreachSound->stop();
          //}
        row++;
      }
    }
  }
  watchdogNodes->Delete();

}

// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::RemoveToolBar(vtkObject*, vtkObject* nodeToBeRemoved)
{
  if(nodeToBeRemoved==NULL && this->WatchdogToolBarHash==NULL)
  {
    return;
  }

  vtkMRMLWatchdogNode* watchdogNodeToBeRemoved = vtkMRMLWatchdogNode::SafeDownCast( nodeToBeRemoved );
  if(watchdogNodeToBeRemoved==NULL)
  {
    return;
  }

  qMRMLWatchdogToolBar *watchdogToolBar = this->WatchdogToolBarHash->value(watchdogNodeToBeRemoved->GetID());

  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  window->removeToolBar(watchdogToolBar);
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //watchdogToolBar->toggleViewAction()->name()
      //toolBarMenuActions.remove(watchdogToolBar->toggleViewAction());
      toolBarMenu->removeAction(watchdogToolBar->toggleViewAction());
      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      break;
    }
  }

  this->WatchdogToolBarHash->remove(watchdogNodeToBeRemoved->GetID());
  if(this->WatchdogToolBarHash->size()==0)
  {
    this->Timer->stop();
    //qCritical("STOPPED timer!!");
  }
}

// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::InitializeToolBar(vtkMRMLWatchdogNode* watchdogNodeAdded )
{
  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLWatchdogToolBar *watchdogToolBar = new qMRMLWatchdogToolBar (window);
  watchdogToolBar->SetWatchdogToolBarName(watchdogNodeAdded->GetName());
  this->WatchdogToolBarHash->insert(QString(watchdogNodeAdded->GetID()), watchdogToolBar);
  window->addToolBar(watchdogToolBar);
  watchdogToolBar->setWindowTitle(QApplication::translate("qSlicerAppMainWindow", watchdogNodeAdded->GetName(), 0, QApplication::UnicodeUTF8));
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolBar->toggleViewAction());
      break;
    }
  }
}

// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::AddToolBar(vtkObject*, vtkObject* nodeAdded)
{
  assert(this->Superclass::mrmlScene() != 0);
  if(nodeAdded==NULL && this->WatchdogToolBarHash==NULL)
  {
    return;
  }

  if(this->WatchdogToolBarHash==NULL)
  {
    this->WatchdogToolBarHash = new QHash<QString, qMRMLWatchdogToolBar *>;
  }

  vtkMRMLWatchdogNode* watchdogNodeAdded = vtkMRMLWatchdogNode::SafeDownCast( nodeAdded );
  if(watchdogNodeAdded==NULL)
  {
    return;
  }

  if( !this->Timer->isActive() )
  {
    this->Timer->start(1000.0*this->StatusRefreshTimeSec );
    //qCritical("STARTED timer!!");
  }
  if(!this->WatchdogToolBarHash->contains(watchdogNodeAdded->GetID()))
  {
    this->InitializeToolBar(watchdogNodeAdded);
    std::list<WatchedTool>* toolsVectorPtr = watchdogNodeAdded->GetToolNodes();
    int numberTools = toolsVectorPtr->size();
    //qDebug() << "toolbar manager initialize watchdogNodeAdded tools " <<numberTools<< "in the toolbar";
    if ( toolsVectorPtr == NULL /*|| numberTools!= d->ToolsTableWidget->rowCount()*/)
    {
      return;
    }
    int row=0;
    for (std::list<WatchedTool>::iterator itTool = toolsVectorPtr->begin() ; itTool != toolsVectorPtr->end(); ++itTool)
    {
      if((*itTool).id=="")
      {
        return;
      }
      this->WatchdogToolBarHash->value(QString(watchdogNodeAdded->GetID()))->SetToolNodeAddedLabel((*itTool).label.c_str());
      row++;
    }
  }
}






