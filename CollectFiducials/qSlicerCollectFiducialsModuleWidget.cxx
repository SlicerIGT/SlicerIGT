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
#include "qSlicerCollectFiducialsModuleWidget.h"
#include "ui_qSlicerCollectFiducialsModule.h"

#include "vtkSlicerCollectFiducialsLogic.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"



//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CollectFiducials
class qSlicerCollectFiducialsModuleWidgetPrivate: public Ui_qSlicerCollectFiducialsModule
{
  Q_DECLARE_PUBLIC( qSlicerCollectFiducialsModuleWidget ); 
  
protected:
  qSlicerCollectFiducialsModuleWidget* const q_ptr;
public:
  qSlicerCollectFiducialsModuleWidgetPrivate( qSlicerCollectFiducialsModuleWidget& object );
  vtkSlicerCollectFiducialsLogic* logic() const;
};



//-----------------------------------------------------------------------------
// qSlicerCollectFiducialsModuleWidgetPrivate methods


qSlicerCollectFiducialsModuleWidgetPrivate::qSlicerCollectFiducialsModuleWidgetPrivate( qSlicerCollectFiducialsModuleWidget& object ) : q_ptr( &object )
{
}


vtkSlicerCollectFiducialsLogic* qSlicerCollectFiducialsModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerCollectFiducialsModuleWidget );
  return vtkSlicerCollectFiducialsLogic::SafeDownCast( q->logic() );
}



//-----------------------------------------------------------------------------
// qSlicerCollectFiducialsModuleWidget methods



qSlicerCollectFiducialsModuleWidget
::qSlicerCollectFiducialsModuleWidget( QWidget* _parent )
  : Superclass( _parent )
  , d_ptr( new qSlicerCollectFiducialsModuleWidgetPrivate( *this ) )
{
}



qSlicerCollectFiducialsModuleWidget
::~qSlicerCollectFiducialsModuleWidget()
{
}


void qSlicerCollectFiducialsModuleWidget
::onProbeTransformNodeSelected()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  
  vtkMRMLNode* node = d->ProbeTransformComboBox->currentNode();
  vtkMRMLLinearTransformNode* tNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  
  if( tNode != NULL )
  {
    d->logic()->SetProbeTransformNode( tNode );
  }
}



/**
 * This will be used when Annotation list can be specified.
 */
void qSlicerCollectFiducialsModuleWidget
::onFiducialListSelected()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  
  /*
  vtkMRMLNode* node = d->AnnotationListComboBox->currentNode();
  vtkMRMLAnnotationHierarchyNode* aNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast( node );
  
  if( aNode != NULL )
  {
    d->logic()->SetAnnotationHierarchyNode( aNode );
  }
  */
}
  


void qSlicerCollectFiducialsModuleWidget
::onRecordClicked()
{
  Q_D( qSlicerCollectFiducialsModuleWidget );
  
  d->logic()->AddFiducial();
}



void qSlicerCollectFiducialsModuleWidget
::setup()
{
  Q_D(qSlicerCollectFiducialsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  d->ProbeTransformComboBox->setNoneEnabled( true );
  // d->AnnotationListComboBox->setNoneEnabled( true );
  
  connect( d->ProbeTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onProbeTransformNodeSelected() ) );
  // connect( d->AnnotationListComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onFiducialListSelected() ) );
  connect( d->RecordButton, SIGNAL( clicked() ), this, SLOT( onRecordClicked() ) );
}

