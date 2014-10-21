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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerToolWatchdogFooBarWidget.h"
#include "ui_qSlicerToolWatchdogFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ToolWatchdog
class qSlicerToolWatchdogFooBarWidgetPrivate
  : public Ui_qSlicerToolWatchdogFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerToolWatchdogFooBarWidget);
protected:
  qSlicerToolWatchdogFooBarWidget* const q_ptr;

public:
  qSlicerToolWatchdogFooBarWidgetPrivate(
    qSlicerToolWatchdogFooBarWidget& object);
  virtual void setupUi(qSlicerToolWatchdogFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerToolWatchdogFooBarWidgetPrivate
::qSlicerToolWatchdogFooBarWidgetPrivate(
  qSlicerToolWatchdogFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerToolWatchdogFooBarWidgetPrivate
::setupUi(qSlicerToolWatchdogFooBarWidget* widget)
{
  this->Ui_qSlicerToolWatchdogFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerToolWatchdogFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerToolWatchdogFooBarWidget
::qSlicerToolWatchdogFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerToolWatchdogFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerToolWatchdogFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerToolWatchdogFooBarWidget
::~qSlicerToolWatchdogFooBarWidget()
{
}
