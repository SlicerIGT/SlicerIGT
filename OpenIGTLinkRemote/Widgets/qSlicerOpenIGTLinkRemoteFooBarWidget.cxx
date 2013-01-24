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
#include "qSlicerOpenIGTLinkRemoteFooBarWidget.h"
#include "ui_qSlicerOpenIGTLinkRemoteFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_OpenIGTLinkRemote
class qSlicerOpenIGTLinkRemoteFooBarWidgetPrivate
  : public Ui_qSlicerOpenIGTLinkRemoteFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerOpenIGTLinkRemoteFooBarWidget);
protected:
  qSlicerOpenIGTLinkRemoteFooBarWidget* const q_ptr;

public:
  qSlicerOpenIGTLinkRemoteFooBarWidgetPrivate(
    qSlicerOpenIGTLinkRemoteFooBarWidget& object);
  virtual void setupUi(qSlicerOpenIGTLinkRemoteFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteFooBarWidgetPrivate
::qSlicerOpenIGTLinkRemoteFooBarWidgetPrivate(
  qSlicerOpenIGTLinkRemoteFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerOpenIGTLinkRemoteFooBarWidgetPrivate
::setupUi(qSlicerOpenIGTLinkRemoteFooBarWidget* widget)
{
  this->Ui_qSlicerOpenIGTLinkRemoteFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkRemoteFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteFooBarWidget
::qSlicerOpenIGTLinkRemoteFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerOpenIGTLinkRemoteFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerOpenIGTLinkRemoteFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkRemoteFooBarWidget
::~qSlicerOpenIGTLinkRemoteFooBarWidget()
{
}
