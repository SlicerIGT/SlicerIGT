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
#include <QDebug>
#include <QtGui>

// SlicerQt includes
#include "qSlicerCreateModelsModuleWidget.h"
#include "ui_qSlicerCreateModelsModule.h"

#include "vtkSlicerCreateModelsLogic.h"



//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCreateModelsModuleWidgetPrivate: public Ui_qSlicerCreateModelsModule
{
  Q_DECLARE_PUBLIC( qSlicerCreateModelsModuleWidget );
protected:
  qSlicerCreateModelsModuleWidget* const q_ptr;
public:
  qSlicerCreateModelsModuleWidgetPrivate( qSlicerCreateModelsModuleWidget& object );
  vtkSlicerCreateModelsLogic* logic() const;
};


//-----------------------------------------------------------------------------
// qSlicerCreateModelsModuleWidgetPrivate methods


qSlicerCreateModelsModuleWidgetPrivate
::qSlicerCreateModelsModuleWidgetPrivate( qSlicerCreateModelsModuleWidget& object )
 : q_ptr( &object )
{
}


vtkSlicerCreateModelsLogic*
qSlicerCreateModelsModuleWidgetPrivate
::logic() const
{
  Q_Q( const qSlicerCreateModelsModuleWidget );
  return vtkSlicerCreateModelsLogic::SafeDownCast( q->logic() );
}



//-----------------------------------------------------------------------------
// qSlicerCreateModelsModuleWidget methods


//-----------------------------------------------------------------------------
qSlicerCreateModelsModuleWidget::qSlicerCreateModelsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerCreateModelsModuleWidgetPrivate( *this ) )
{
}



//-----------------------------------------------------------------------------
qSlicerCreateModelsModuleWidget::~qSlicerCreateModelsModuleWidget()
{
}



void
qSlicerCreateModelsModuleWidget
::OnCreateNeedleClicked()
{
  Q_D(qSlicerCreateModelsModuleWidget);
  
  d->logic()->CreateNeedle( d->NeedleLength->value(), d->NeedleRadius->value(), d->NeedleTipRadius->value(), d->markersCheckBox->isChecked() );
}



void
qSlicerCreateModelsModuleWidget
::OnCreateCubeClicked()
{
  Q_D(qSlicerCreateModelsModuleWidget);
  
  d->logic()->CreateCube( d->CubeRLength->value(), d->CubeALength->value(), d->CubeSLength->value() );
}



void
qSlicerCreateModelsModuleWidget
::OnCreateCylinderClicked()
{
  Q_D(qSlicerCreateModelsModuleWidget);
  
  d->logic()->CreateCylinder( d->CylinderHeight->value(), d->CylinderRadius->value() );
}


void
qSlicerCreateModelsModuleWidget
::OnCreateSphereClicked()
{
  Q_D(qSlicerCreateModelsModuleWidget);
  
  d->logic()->CreateSphere( d->SphereRadius->value() );
}



void
qSlicerCreateModelsModuleWidget
::OnCreateCoordinateClicked()
{
  Q_D( qSlicerCreateModelsModuleWidget );
  
  d->logic()->CreateCoordinate( d->CAxisLengthSpinBox->value(), d->CAxisThicknessSpinBox->value() );
}



void qSlicerCreateModelsModuleWidget::setup()
{
  Q_D(qSlicerCreateModelsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->CreateNeedleButton, SIGNAL( clicked() ), this, SLOT( OnCreateNeedleClicked() ) );
  connect( d->CreateCubeButton, SIGNAL( clicked() ), this, SLOT( OnCreateCubeClicked() ) );
  connect( d->CreateCylinderButton, SIGNAL( clicked() ), this, SLOT( OnCreateCylinderClicked() ) );
  connect( d->CreateSphereButton, SIGNAL( clicked() ), this, SLOT( OnCreateSphereClicked() ) );
  connect( d->CreateCoordinateButton, SIGNAL( clicked() ), this, SLOT( OnCreateCoordinateClicked() ) );
}

