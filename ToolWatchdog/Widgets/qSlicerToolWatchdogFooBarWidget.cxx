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
#include "qSlicerWatchdogFooBarWidget.h"
#include "ui_qSlicerWatchdogFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Watchdog
class qSlicerWatchdogFooBarWidgetPrivate
  : public Ui_qSlicerWatchdogFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerWatchdogFooBarWidget);
protected:
  qSlicerWatchdogFooBarWidget* const q_ptr;

public:
  qSlicerWatchdogFooBarWidgetPrivate(
    qSlicerWatchdogFooBarWidget& object);
  virtual void setupUi(qSlicerWatchdogFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerWatchdogFooBarWidgetPrivate
::qSlicerWatchdogFooBarWidgetPrivate(
  qSlicerWatchdogFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerWatchdogFooBarWidgetPrivate
::setupUi(qSlicerWatchdogFooBarWidget* widget)
{
  this->Ui_qSlicerWatchdogFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWatchdogFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerWatchdogFooBarWidget
::qSlicerWatchdogFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerWatchdogFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerWatchdogFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerWatchdogFooBarWidget
::~qSlicerWatchdogFooBarWidget()
{
}
