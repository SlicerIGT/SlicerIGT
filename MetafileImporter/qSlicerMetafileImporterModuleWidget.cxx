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
#include <QFileDialog>
#include <QProgressDialog>
#include <QtGui>

// SlicerQt includes
#include "qSlicerMetafileImporterModuleWidget.h"
#include "ui_qSlicerMetafileImporterModuleWidget.h"
#include "vtkSlicerMetafileImporterLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMetafileImporterModuleWidgetPrivate: public Ui_qSlicerMetafileImporterModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerMetafileImporterModuleWidget );

protected:
  qSlicerMetafileImporterModuleWidget* const q_ptr;
public:
  qSlicerMetafileImporterModuleWidgetPrivate( qSlicerMetafileImporterModuleWidget& object );
  ~qSlicerMetafileImporterModuleWidgetPrivate();

  vtkSlicerMetafileImporterLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerMetafileImporterModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMetafileImporterModuleWidgetPrivate::qSlicerMetafileImporterModuleWidgetPrivate( qSlicerMetafileImporterModuleWidget& object ) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------

qSlicerMetafileImporterModuleWidgetPrivate::~qSlicerMetafileImporterModuleWidgetPrivate()
{
}


vtkSlicerMetafileImporterLogic* qSlicerMetafileImporterModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerMetafileImporterModuleWidget );
  return vtkSlicerMetafileImporterLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerMetafileImporterModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMetafileImporterModuleWidget::qSlicerMetafileImporterModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerMetafileImporterModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerMetafileImporterModuleWidget::~qSlicerMetafileImporterModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMetafileImporterModuleWidget::setup()
{
  Q_D(qSlicerMetafileImporterModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect( d->ImportButton, SIGNAL( clicked() ), this, SLOT( onImportButtonClicked() ) );
}


//-----------------------------------------------------------------------------
void qSlicerMetafileImporterModuleWidget
::onImportButtonClicked()
{
  Q_D( qSlicerMetafileImporterModuleWidget );

  QString fileName = QFileDialog::getOpenFileName( this, tr("Open metafile"), "", tr("MHA Files (*.mha *.mhd *.nrrd)") );

  if ( fileName.isEmpty() == false )
  {
    QProgressDialog dialog;
    dialog.setModal( true );
    dialog.setLabelText( "Please wait while reading sequence metafile..." );
    dialog.show();
    dialog.setValue( 25 );

    d->logic()->ReadSequenceFile( fileName.toStdString() );

    dialog.close();
  }

}

