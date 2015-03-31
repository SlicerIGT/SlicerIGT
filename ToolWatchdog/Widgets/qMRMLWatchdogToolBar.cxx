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
//#include <QMenu>
#include <QInputDialog>
//#include <QToolButton>
#include <QLabel>
#include <QList>

// qMRML includes
#include "qMRMLWatchdogToolBar.h"
//#include "qMRMLSceneViewMenu.h"
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
  QList<QAction*>*                         ActionsListPtr;
};

//--------------------------------------------------------------------------
// qMRMLWatchdogToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLWatchdogToolBarPrivate::qMRMLWatchdogToolBarPrivate(qMRMLWatchdogToolBar& object)
  : q_ptr(&object)
{
  this->ActionsListPtr = NULL;
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
    transformLabel->setMargin(2);
    //this->LabelsList->setIcon(QIcon(":/Icons/Watchdog.png"));
    this->ActionsListPtr->push_back(q->addWidget(transformLabel));
    //QObject::connect(this->LabelsListPtr, SIGNAL(triggered()),
    //                 q, SIGNAL(screenshotButtonClicked()));
    
  }
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

//---------------------------------------------------------------------------
void qMRMLWatchdogToolBar
::SetWatchdogToolBarName(char * watchdogNodeName)
{
  Q_D(qMRMLWatchdogToolBar);
  QString tooltip("Each square indicates the state of the tools watched by the Watchdog module: ");
  tooltip.append(watchdogNodeName);
  QLabel* watchdogToolBarName = (QLabel*)this->widgetForAction(d->ActionsListPtr->at(0));
  watchdogToolBarName->setToolTip( tooltip );
  watchdogToolBarName->setText(QString(watchdogNodeName));
}

//---------------------------------------------------------------------------
void qMRMLWatchdogToolBar
::SetToolNodeAddedLabel(const char * toolNodeAddedLabel)
{
  Q_D(qMRMLWatchdogToolBar);

  QLabel* toolLabel = new QLabel(this);
  toolLabel->setToolTip(this->tr("Tool in row %1").arg(d->ActionsListPtr->size()));
  toolLabel->setText(toolNodeAddedLabel);

  toolLabel->setAlignment(Qt::AlignCenter);
  toolLabel->setStyleSheet("QLabel { background-color: green; min-width: 2em; max-height: 2em;}");
  toolLabel->setMargin(4);
  d->ActionsListPtr->push_back(this->addWidget(toolLabel));
}

//---------------------------------------------------------------------------
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
}

//---------------------------------------------------------------------------
void qMRMLWatchdogToolBar
::DeleteToolNode(int row)
{
  Q_D(qMRMLWatchdogToolBar);
  this->removeAction(d->ActionsListPtr->at(row+1));
  d->ActionsListPtr->removeAt(row+1);
}

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
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

