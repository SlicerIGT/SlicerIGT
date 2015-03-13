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
#include <QInputDialog>
#include <QToolButton>
#include <QLabel>
#include <QList>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qMRMLWatchdogToolBar.h"


// qMRML includes
#include "qSlicerToolBarManagerWidget.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLScene.h>
#include "vtkMRMLWatchdogNode.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>




// --------------------------------------------------------------------------
// qSlicerToolBarManagerWidget methods

// --------------------------------------------------------------------------
qSlicerToolBarManagerWidget::qSlicerToolBarManagerWidget(QWidget* _parent)
  : Superclass(_parent)
{
  this->WatchdogToolbarHash=NULL;
}

//---------------------------------------------------------------------------
qSlicerToolBarManagerWidget::~qSlicerToolBarManagerWidget()
{
  //assert(this->Superclass::mrmlScene() != 0);
  //this->qvtkDisconnect(this->Superclass::mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT(RemoveToolbar(vtkObject*, vtkObject*)));
  //this->qvtkDisconnect(this->Superclass::mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT(AddToolbar(vtkObject*, vtkObject*)));
}


QHash<QString, qMRMLWatchdogToolBar *> * qSlicerToolBarManagerWidget::GetToolbarHash()
{
  return this->WatchdogToolbarHash;
}


// --------------------------------------------------------------------------
void qSlicerToolBarManagerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  qCritical()<<"HOASSS";
  this->Superclass::setMRMLScene( newScene );
  //InitializeToolbarHash();
  
  //if (newScene == this->Superclass::mrmlScene())
  //  {
  //  return;
  //  }

  //this->qvtkConnect(newScene, vtkMRMLScene::EndImportEvent, this, SLOT(InitializeToolbarHash()));
  this->qvtkConnect(newScene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(RemoveToolbar(vtkObject*, vtkObject*)));
  this->qvtkConnect(newScene, vtkMRMLScene::NodeAddedEvent, this, SLOT(AddToolbar(vtkObject*, vtkObject*)));

//  this->qvtkReconnect(this->Superclass::mrmlScene(), newScene, vtkMRMLScene::EndImportEvent,
//                      this, SLOT(InitializeToolbarHash()));
////
//  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent,
//                      this, SLOT(OnMRMLSceneEndBatchProcessing()));
//
//*/
//  this->MRMLScene = newScene;
//
//  //this->SceneViewMenu->setMRMLScene(newScene);
//
//  // Update UI
//  this->updateWidgetFromMRML();
}

void qSlicerToolBarManagerWidget::InitializeToolbar(vtkMRMLWatchdogNode* watchdogNodeAdded )
{
  qDebug() << "Initialize toolBAR ";
  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  qMRMLWatchdogToolBar *watchdogToolbar = new qMRMLWatchdogToolBar (window);
  watchdogToolbar->SetFirstlabel(watchdogNodeAdded->GetName());
  this->WatchdogToolbarHash->insert(QString(watchdogNodeAdded->GetID()), watchdogToolbar);
  window->addToolBar(watchdogToolbar);
  watchdogToolbar->setWindowTitle(QApplication::translate("qSlicerAppMainWindow", watchdogNodeAdded->GetName(), 0, QApplication::UnicodeUTF8));
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolbar->toggleViewAction());
      connect(watchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );
      break;
    }
  }
}


//void qSlicerToolBarManagerWidget::InitializeToolbarHash()
//{
//  //Q_D(qSlicerWatchdogModuleWidget);
//  qDebug() << "Initialize toolBAR HASH";
//
//  if(this->WatchdogToolbarHash==NULL)
//  {
//    this->WatchdogToolbarHash = new QHash<QString, qMRMLWatchdogToolBar *>;
//  }
//  assert(this->Superclass::mrmlScene() != 0);
//  vtkCollection* watchdogNodes = this->Superclass::mrmlScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
//  vtkCollectionIterator* watchdogNodeIt = vtkCollectionIterator::New();
//  watchdogNodeIt->SetCollection( watchdogNodes );
//  int hasTools=0;
//  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
//  {
//    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
//    if ( watchdogNode != NULL)
//    {
//      if(!this->WatchdogToolbarHash->contains(watchdogNode->GetID()))
//      {
//        this->InitializeToolbar(watchdogNode);
//        //watchdogNode->GetNumberOfTools()
//        //  this->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->ToolNodeAdded(currentToolNode->GetName());
//        std::list<WatchedTool>* toolsVectorPtr = watchdogNode->GetToolNodes();
//        int numberTools = toolsVectorPtr->size();
//        qDebug() << "enter initialize watchdognode tools " <<numberTools<< "in the toolbar";
//        if ( toolsVectorPtr == NULL /*|| numberTools!= d->ToolsTableWidget->rowCount()*/)
//        {
//          return;
//        }
//        int row=0;
//        for (std::list<WatchedTool>::iterator itTool = toolsVectorPtr->begin() ; itTool != toolsVectorPtr->end(); ++itTool)
//        {
//          if((*itTool).tool==NULL)
//          {
//            return;
//          }
//          this->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->ToolNodeAdded((*itTool).label.c_str());
//          row++;
//        }
//      }
//    }
//  }
//  watchdogNodeIt->Delete();
//  watchdogNodes->Delete();
//}

void  qSlicerToolBarManagerWidget::onUpdateToolbars()
{
  //qDebug() << "update toolbars";
  if(this->WatchdogToolbarHash==NULL)
  {
    return;
  }

  vtkCollection* watchdogNodes = this->Superclass::mrmlScene()->GetNodesByClass( "vtkMRMLWatchdogNode" );
  vtkCollectionIterator* watchdogNodeIt = vtkCollectionIterator::New();
  watchdogNodeIt->SetCollection( watchdogNodes );

  for ( watchdogNodeIt->InitTraversal(); ! watchdogNodeIt->IsDoneWithTraversal(); watchdogNodeIt->GoToNextItem() )
  {
    vtkMRMLWatchdogNode* watchdogNode = vtkMRMLWatchdogNode::SafeDownCast( watchdogNodeIt->GetCurrentObject() );
    if(this->WatchdogToolbarHash->contains(QString(watchdogNode->GetID())) && this->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->isVisible())
    {
      //d->logic()->UpdateToolStatus( watchdogNode, (unsigned long) ElapsedTimeSec );
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
        this->WatchdogToolbarHash->value(QString(watchdogNode->GetID()))->SetNodeStatus(row,(*itTool).status);
        row++;
      }
    }
  }
    watchdogNodeIt->Delete();
    watchdogNodes->Delete();
}




void qSlicerToolBarManagerWidget::RemoveToolbar(vtkObject*, vtkObject* nodeToBeRemoved)
{
  //vtkWarningMacro("DELETE WATCHDOG NODE : "<< this->GetName());
  if(nodeToBeRemoved==NULL && this->WatchdogToolbarHash==NULL)
  {
    return;
  }

  vtkMRMLWatchdogNode* watchdogNodeToBeRemoved = vtkMRMLWatchdogNode::SafeDownCast( nodeToBeRemoved );
  if(watchdogNodeToBeRemoved==NULL)
  {
    return;
  }

  qMRMLWatchdogToolBar *watchdogToolbar = this->WatchdogToolbarHash->value(watchdogNodeToBeRemoved->GetID());

  QMainWindow* window = qSlicerApplication::application()->mainWindow();
  window->removeToolBar(watchdogToolbar);
  foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
  {
    if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
    {
      QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
      //watchdogToolbar->toggleViewAction()->name()
      //toolBarMenuActions.remove(watchdogToolbar->toggleViewAction());
      toolBarMenu->removeAction(watchdogToolbar->toggleViewAction());

      //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
      //toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),watchdogToolbar->toggleViewAction());
      break;
    }
  }
}


void qSlicerToolBarManagerWidget::AddToolbar(vtkObject*, vtkObject* nodeAdded)
{
  //vtkWarningMacro("DELETE WATCHDOG NODE : "<< this->GetName());
  assert(this->Superclass::mrmlScene() != 0);
  if(nodeAdded==NULL && this->WatchdogToolbarHash==NULL)
  {
    return;
  }

  qDebug() << "Initialize toolBAR HASH";
  if(this->WatchdogToolbarHash==NULL)
  {
    this->WatchdogToolbarHash = new QHash<QString, qMRMLWatchdogToolBar *>;
  }

  vtkMRMLWatchdogNode* watchdogNodeAdded = vtkMRMLWatchdogNode::SafeDownCast( nodeAdded );
  if(watchdogNodeAdded==NULL)
  {
    return;
  }

  if ( watchdogNodeAdded != NULL)
  {
    if(!this->WatchdogToolbarHash->contains(watchdogNodeAdded->GetID()))
    {
      this->InitializeToolbar(watchdogNodeAdded);
      //watchdogNodeToBeAdded->GetNumberOfTools()
      //  this->WatchdogToolbarHash->value(QString(watchdogNodeToBeAdded->GetID()))->ToolNodeAdded(currentToolNode->GetName());
      std::list<WatchedTool>* toolsVectorPtr = watchdogNodeAdded->GetToolNodes();
      int numberTools = toolsVectorPtr->size();
      qDebug() << "toolbar manager initialize watchdogNodeAdded tools " <<numberTools<< "in the toolbar";
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
        this->WatchdogToolbarHash->value(QString(watchdogNodeAdded->GetID()))->ToolNodeAdded((*itTool).label.c_str());
        row++;
      }
    }
  }

}






