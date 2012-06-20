/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes

// SlicerQt includes
#include "qSlicerIGTLRemoteModuleWidget.h"
#include "ui_qSlicerIGTLRemoteModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IGTLRemote
class qSlicerIGTLRemoteModuleWidgetPrivate: public Ui_qSlicerIGTLRemoteModule
{
public:
  qSlicerIGTLRemoteModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidgetPrivate::qSlicerIGTLRemoteModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerIGTLRemoteModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidget::qSlicerIGTLRemoteModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIGTLRemoteModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerIGTLRemoteModuleWidget::~qSlicerIGTLRemoteModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIGTLRemoteModuleWidget::setup()
{
  Q_D(qSlicerIGTLRemoteModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

