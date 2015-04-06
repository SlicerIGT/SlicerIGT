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
#include "qSlicerMarkupsToModelFooBarWidget.h"
#include "ui_qSlicerMarkupsToModelFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_MarkupsToModel
class qSlicerMarkupsToModelFooBarWidgetPrivate
  : public Ui_qSlicerMarkupsToModelFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsToModelFooBarWidget);
protected:
  qSlicerMarkupsToModelFooBarWidget* const q_ptr;

public:
  qSlicerMarkupsToModelFooBarWidgetPrivate(
    qSlicerMarkupsToModelFooBarWidget& object);
  virtual void setupUi(qSlicerMarkupsToModelFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerMarkupsToModelFooBarWidgetPrivate
::qSlicerMarkupsToModelFooBarWidgetPrivate(
  qSlicerMarkupsToModelFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerMarkupsToModelFooBarWidgetPrivate
::setupUi(qSlicerMarkupsToModelFooBarWidget* widget)
{
  this->Ui_qSlicerMarkupsToModelFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsToModelFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelFooBarWidget
::qSlicerMarkupsToModelFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerMarkupsToModelFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerMarkupsToModelFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsToModelFooBarWidget
::~qSlicerMarkupsToModelFooBarWidget()
{
}
