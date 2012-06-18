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
#include "qSlicerRealTimeImagingModuleWidget.h"
#include "ui_qSlicerRealTimeImagingModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RealTimeImaging
class qSlicerRealTimeImagingModuleWidgetPrivate: public Ui_qSlicerRealTimeImagingModule
{
public:
  qSlicerRealTimeImagingModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerRealTimeImagingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerRealTimeImagingModuleWidgetPrivate::qSlicerRealTimeImagingModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerRealTimeImagingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerRealTimeImagingModuleWidget::qSlicerRealTimeImagingModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerRealTimeImagingModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerRealTimeImagingModuleWidget::~qSlicerRealTimeImagingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerRealTimeImagingModuleWidget::setup()
{
  Q_D(qSlicerRealTimeImagingModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

