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

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PlusSupport
class qSlicerPlusSupportTransformWidgetPrivate
  : public Ui_qSlicerPlusSupportTransformWidget
{
  Q_DECLARE_PUBLIC(qSlicerPlusSupportTransformWidget);
protected:
  qSlicerPlusSupportTransformWidget* const q_ptr;

public:
  qSlicerPlusSupportTransformWidgetPrivate(
    qSlicerPlusSupportTransformWidget& object);
  virtual void setupUi(qSlicerPlusSupportTransformWidget*);
};

// --------------------------------------------------------------------------
qSlicerPlusSupportTransformWidgetPrivate
::qSlicerPlusSupportTransformWidgetPrivate(
  qSlicerPlusSupportTransformWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPlusSupportTransformWidgetPrivate
::setupUi(qSlicerPlusSupportTransformWidget* widget)
{
  this->Ui_qSlicerPlusSupportTransformWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPlusSupportTransformWidget methods

//-----------------------------------------------------------------------------
qSlicerPlusSupportTransformWidget
::qSlicerPlusSupportTransformWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPlusSupportTransformWidgetPrivate(*this) )
{
  Q_D(qSlicerPlusSupportTransformWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerPlusSupportTransformWidget
::~qSlicerPlusSupportTransformWidget()
{
}
