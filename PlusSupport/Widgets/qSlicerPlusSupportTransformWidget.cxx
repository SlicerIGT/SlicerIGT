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

  This file was originally developed by Adam Rankin, Robarts Research Institute

==============================================================================*/

// Transform Widgets includes
#include "qSlicerPlusSupportTransformWidget.h"
#include "ui_qSlicerPlusSupportTransformWidget.h"

// MRML includes
#include <qMRMLNodeComboBox.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkMatrix4x4.h>

// Qt includes
#include <QDebug>
#include <QPixmap>
#include <QScopedPointer>
#include <QTimer>

// STL includes
#include <chrono>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PlusSupport
namespace
{
  // --------------------------------------------------------------------------
  bool IsIdentity(vtkMatrix4x4* a)
  {
    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        double value = (i == j) ? 1.0 : 0.0;
        if (a->GetElement(i, j) != value)
        {
          return false;
        }
      }
    }

    return true;
  }
}

class qSlicerPlusSupportTransformWidgetPrivate : public Ui_qSlicerPlusSupportTransformWidget
{
  Q_DECLARE_PUBLIC(qSlicerPlusSupportTransformWidget);

protected:
  qSlicerPlusSupportTransformWidget* const q_ptr;

public:
  qSlicerPlusSupportTransformWidgetPrivate(qSlicerPlusSupportTransformWidget& object);
  virtual void setupUi(qSlicerPlusSupportTransformWidget*);

  unsigned int                          XSize;
  unsigned int                          YSize;

  qMRMLNodeComboBox*                    TransformSelector;
  vtkMRMLLinearTransformNode*           TransformNode;
  unsigned long                         TransformNodeObserverTag;

  QPixmap                               OkPixmap;
  QPixmap                               NotOkPixmap;
  QPixmap                               WarningPixmap;

  std::chrono::system_clock::time_point LastModifiedTime;

  QTimer                                TimeoutTimer;
};

// --------------------------------------------------------------------------
qSlicerPlusSupportTransformWidgetPrivate::qSlicerPlusSupportTransformWidgetPrivate(qSlicerPlusSupportTransformWidget& object)
  : q_ptr(&object)
  , TransformSelector(nullptr)
  , TransformNode(nullptr)
{
}

// --------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidgetPrivate::setupUi(qSlicerPlusSupportTransformWidget* widget)
{
  this->Ui_qSlicerPlusSupportTransformWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPlusSupportTransformWidget methods

//-----------------------------------------------------------------------------
qSlicerPlusSupportTransformWidget::qSlicerPlusSupportTransformWidget(QWidget* parentWidget, QSize pixmapSize)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerPlusSupportTransformWidgetPrivate(*this))
{
  Q_D(qSlicerPlusSupportTransformWidget);
  d->setupUi(this);

  this->setLabelSize(pixmapSize);

  connect(&d->TimeoutTimer, &QTimer::timeout, this, &qSlicerPlusSupportTransformWidget::onTimerTick);

  d->TimeoutTimer.start(10);
}

//-----------------------------------------------------------------------------
qSlicerPlusSupportTransformWidget::~qSlicerPlusSupportTransformWidget()
{
  Q_D(qSlicerPlusSupportTransformWidget);

  disconnect(&d->TimeoutTimer, &QTimer::timeout, this, &qSlicerPlusSupportTransformWidget::onTimerTick);
}

//----------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidget::setTransformSelector(qMRMLNodeComboBox* comboBox)
{
  Q_D(qSlicerPlusSupportTransformWidget);

  if (d->TransformSelector != nullptr)
  {
    disconnect(d->TransformSelector, static_cast<void (qMRMLNodeComboBox::*)(vtkMRMLNode*)>(&qMRMLNodeComboBox::currentNodeChanged), this, &qSlicerPlusSupportTransformWidget::onTransformNodeChanged);
  }

  d->TransformSelector = comboBox;

  connect(d->TransformSelector, static_cast<void (qMRMLNodeComboBox::*)(vtkMRMLNode*)>(&qMRMLNodeComboBox::currentNodeChanged), this, &qSlicerPlusSupportTransformWidget::onTransformNodeChanged);
}

//----------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidget::setLabelSize(QSize& newSize)
{
  Q_D(qSlicerPlusSupportTransformWidget);

  d->XSize = newSize.width();
  d->YSize = newSize.height();

  // Create pixmaps of requested size
  {
    QIcon icon(":/Icons/icons_Ok.png");
    d->OkPixmap = icon.pixmap(icon.actualSize(newSize));
  }

  {
    QIcon icon(":/Icons/icons_NotOk.png");
    d->NotOkPixmap = icon.pixmap(icon.actualSize(newSize));
  }

  {
    QIcon icon(":/Icons/icons_Warning.png");
    d->WarningPixmap = icon.pixmap(icon.actualSize(newSize));
  }

  this->updateLabel();
}

//----------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidget::onTransformNodeChanged(vtkMRMLNode* newNode)
{
  Q_D(qSlicerPlusSupportTransformWidget);

  if (d->TransformNode != nullptr)
  {
    d->TransformNode->RemoveObserver(d->TransformNodeObserverTag);
  }

  d->TransformNode = vtkMRMLLinearTransformNode::SafeDownCast(newNode);

  if (d->TransformNode != nullptr)
  {
    d->TransformNode->AddObserver(vtkMRMLTransformNode::TransformModifiedEvent, this, &qSlicerPlusSupportTransformWidget::onTransformNodeModified);
  }

  this->updateLabel();
}

//----------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidget::onTimerTick()
{
  Q_D(qSlicerPlusSupportTransformWidget);

  this->updateLabel();
}

//----------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidget::onTransformNodeModified(vtkObject* caller, long unsigned int eventId, void* callData)
{
  Q_D(qSlicerPlusSupportTransformWidget);

  d->LastModifiedTime = std::chrono::system_clock::now();
}

//----------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidget::updateLabel()
{
  Q_D(qSlicerPlusSupportTransformWidget);
  if (d->TransformNode != nullptr)
  {
    vtkNew<vtkMatrix4x4> mat;
    d->TransformNode->GetMatrixTransformToParent(mat);
    if (IsIdentity(mat))
    {
      d->label_TransformStatus->setPixmap(d->NotOkPixmap);
    }
    else
    {
      std::chrono::milliseconds difference = std::chrono::duration_cast<std::chrono::milliseconds>(d->LastModifiedTime - std::chrono::system_clock::now());
      if (difference.count() > 1000)
      {
        // Timeout
        d->label_TransformStatus->setPixmap(d->NotOkPixmap);
      }
      else if (difference.count() > 250) // Set 4ups as our minimum for warning
      {
        // Warning
        d->label_TransformStatus->setPixmap(d->WarningPixmap);
      }
      else
      {
        // Ok
        d->label_TransformStatus->setPixmap(d->OkPixmap);
      }
    }
  }
  else
  {
    d->label_TransformStatus->clear();
  }
}
