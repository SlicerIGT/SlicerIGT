// Qt includes
#include <QButtonGroup>

#include "qSlicerReslicePropertyWidget.h"
#include "ui_qSlicerReslicePropertyWidget.h"

// MRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>
#include <qMRMLThreeDWidget.h>

#include "qMRMLViewControllerBar_p.h"
#include <qMRMLViewControllerBar.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>

// MRML includes
#include "vtkSmartPointer.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeNode.h"

// MRMLLogic includes
#include "vtkMRMLLayoutLogic.h"

#include "vtkSlicerVolumeResliceDriverLogic.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <vtkImageData.h>

#include "ctkPopupWidget.h"

#include "qMRMLColors.h"


class qSlicerReslicePropertyWidgetPrivate;
class vtkMRMLNode;
class vtkObject;


//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_OpenIGTLinkIF
class qSlicerReslicePropertyWidgetPrivate :
  public qMRMLViewControllerBarPrivate
  , public Ui_qSlicerReslicePropertyWidget
{
  Q_DECLARE_PUBLIC(qSlicerReslicePropertyWidget);
protected:
  //qSlicerReslicePropertyWidget* const q_ptr;
public:
  typedef qMRMLViewControllerBarPrivate Superclass;

  qSlicerReslicePropertyWidgetPrivate(qSlicerReslicePropertyWidget& object);
  virtual ~qSlicerReslicePropertyWidgetPrivate() ;

  virtual void init();
  void updateSlice(vtkMatrix4x4* transform);
  void updateSliceByTransformNode(vtkMRMLLinearTransformNode* tnode);
  void updateSliceByImageNode(vtkMRMLScalarVolumeNode* inode);

  QButtonGroup methodButtonGroup;
  QButtonGroup orientationButtonGroup;

  vtkMRMLScene * scene;
  vtkMRMLSliceNode * sliceNode;
  vtkMRMLNode  * driverNode;

protected:
  virtual void setupPopupUi();

};

//------------------------------------------------------------------------------
qSlicerReslicePropertyWidgetPrivate
::qSlicerReslicePropertyWidgetPrivate( qSlicerReslicePropertyWidget& object )
 : Superclass(object)
{
  this->scene = NULL;
  this->sliceNode = NULL;
  this->driverNode = NULL;
}


//------------------------------------------------------------------------------
qSlicerReslicePropertyWidgetPrivate::~qSlicerReslicePropertyWidgetPrivate()
{
}


//---------------------------------------------------------------------------
void qSlicerReslicePropertyWidgetPrivate::setupPopupUi()
{
  Q_Q(qSlicerReslicePropertyWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qSlicerReslicePropertyWidget::setupUi(this->PopupWidget);

  this->methodButtonGroup.addButton(this->positionRadioButton, vtkSlicerVolumeResliceDriverLogic::METHOD_POSITION);
  this->methodButtonGroup.addButton(this->orientationRadioButton, vtkSlicerVolumeResliceDriverLogic::METHOD_ORIENTATION);
  this->positionRadioButton->setChecked(true);
  this->orientationButtonGroup.addButton(this->inPlaneRadioButton, vtkSlicerVolumeResliceDriverLogic::ORIENTATION_INPLANE);
  this->orientationButtonGroup.addButton(this->inPlane90RadioButton, vtkSlicerVolumeResliceDriverLogic::ORIENTATION_INPLANE90);
  this->orientationButtonGroup.addButton(this->transverseRadioButton, vtkSlicerVolumeResliceDriverLogic::ORIENTATION_TRANSVERSE);
  this->inPlaneRadioButton->setChecked(true);
  
  QObject::connect(this->driverNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setDriverNode(vtkMRMLNode*)));

  QObject::connect(this->positionRadioButton, SIGNAL(clicked()), q, SLOT(onMethodChanged()));
  QObject::connect(this->orientationRadioButton, SIGNAL(clicked()), q, SLOT(onMethodChanged()));
  QObject::connect(this->inPlaneRadioButton, SIGNAL(clicked()), q, SLOT(onOrientationChanged()));
  QObject::connect(this->inPlane90RadioButton, SIGNAL(clicked()), q, SLOT(onOrientationChanged()));
  QObject::connect(this->transverseRadioButton, SIGNAL(clicked()), q, SLOT(onOrientationChanged()));

}


//------------------------------------------------------------------------------
void qSlicerReslicePropertyWidgetPrivate::init()
{
  //Q_Q(qSlicerReslicePropertyWidget);
  this->Superclass::init();
  this->ViewLabel->setText(qSlicerReslicePropertyWidget::tr("1"));
  this->BarLayout->addStretch(1);
  this->setColor(qMRMLColors::threeDViewBlue());
}

//------------------------------------------------------------------------------
void qSlicerReslicePropertyWidgetPrivate::updateSlice(vtkMatrix4x4 * transform)
{
  //Q_Q(qSlicerReslicePropertyWidget);

  float tx = transform->Element[0][0];
  float ty = transform->Element[1][0];
  float tz = transform->Element[2][0];
  float nx = transform->Element[0][2];
  float ny = transform->Element[1][2];
  float nz = transform->Element[2][2];
  float px = transform->Element[0][3];
  float py = transform->Element[1][3];
  float pz = transform->Element[2][3];

  if (orientationButtonGroup.checkedId() == vtkSlicerVolumeResliceDriverLogic::ORIENTATION_INPLANE)
    {
    if (this->methodButtonGroup.checkedId() == vtkSlicerVolumeResliceDriverLogic::METHOD_ORIENTATION)
      {
      this->sliceNode->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 1);
      }
    else
      {
      this->sliceNode->SetOrientationToAxial();
      this->sliceNode->JumpSlice(px, py, pz);
      }
    }
  else if (orientationButtonGroup.checkedId() == vtkSlicerVolumeResliceDriverLogic::ORIENTATION_INPLANE90)
    {
    if (this->methodButtonGroup.checkedId() == vtkSlicerVolumeResliceDriverLogic::METHOD_ORIENTATION)
      {
      this->sliceNode->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 2);
      }
    else
      {
      this->sliceNode->SetOrientationToSagittal();
      this->sliceNode->JumpSlice(px, py, pz);
      }
    }
  else if (orientationButtonGroup.checkedId() == vtkSlicerVolumeResliceDriverLogic::ORIENTATION_TRANSVERSE)
    {
    if (this->methodButtonGroup.checkedId() == vtkSlicerVolumeResliceDriverLogic::METHOD_ORIENTATION) 
      {
      this->sliceNode->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, 0);
      }
    else
      {
      this->sliceNode->SetOrientationToCoronal();
      this->sliceNode->JumpSlice(px, py, pz);
      }
    }
  this->sliceNode->UpdateMatrices();
}


//---------------------------------------------------------------------------
void qSlicerReslicePropertyWidgetPrivate::updateSliceByTransformNode(vtkMRMLLinearTransformNode* tnode)
{
  //Q_Q(qSlicerReslicePropertyWidget);

  if (!tnode)
    {
    return;
    }

  vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New();
  if (transform)
    {
    transform->Identity();
    int getTransf = tnode->GetMatrixTransformToWorld(transform);
    if(getTransf != 0)
      {
      this->updateSlice(transform);
      }
    }
}


//---------------------------------------------------------------------------
void qSlicerReslicePropertyWidgetPrivate::updateSliceByImageNode(vtkMRMLScalarVolumeNode* inode)
{
  //Q_Q(qSlicerReslicePropertyWidget);

  vtkMRMLVolumeNode* volumeNode = inode;

  if (volumeNode == NULL)
    {
    return;
    }

  vtkSmartPointer<vtkMatrix4x4> rtimgTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix(rtimgTransform);

  float tx = rtimgTransform->GetElement(0, 0);
  float ty = rtimgTransform->GetElement(1, 0);
  float tz = rtimgTransform->GetElement(2, 0);
  float sx = rtimgTransform->GetElement(0, 1);
  float sy = rtimgTransform->GetElement(1, 1);
  float sz = rtimgTransform->GetElement(2, 1);
  float nx = rtimgTransform->GetElement(0, 2);
  float ny = rtimgTransform->GetElement(1, 2);
  float nz = rtimgTransform->GetElement(2, 2);
  float px = rtimgTransform->GetElement(0, 3);
  float py = rtimgTransform->GetElement(1, 3);
  float pz = rtimgTransform->GetElement(2, 3);

  vtkImageData* imageData;
  imageData = volumeNode->GetImageData();
  int size[3];
  imageData->GetDimensions(size);

  // normalize
  float psi = sqrt(tx*tx + ty*ty + tz*tz);
  float psj = sqrt(sx*sx + sy*sy + sz*sz);
  float psk = sqrt(nx*nx + ny*ny + nz*nz);
  float ntx = tx / psi;
  float nty = ty / psi;
  float ntz = tz / psi;
  float nsx = sx / psj;
  float nsy = sy / psj;
  float nsz = sz / psj;
  float nnx = nx / psk;
  float nny = ny / psk;
  float nnz = nz / psk;

  // Shift the center
  // NOTE: The center of the image should be shifted due to different
  // definitions of image origin between VTK (Slicer) and OpenIGTLink;
  // OpenIGTLink image has its origin at the center, while VTK image
  // has one at the corner.

  float hfovi = psi * size[0] / 2.0;
  float hfovj = psj * size[1] / 2.0;
  //float hfovk = psk * imgheader->size[2] / 2.0;
  float hfovk = 0;

  float cx = ntx * hfovi + nsx * hfovj + nnx * hfovk;
  float cy = nty * hfovi + nsy * hfovj + nny * hfovk;
  float cz = ntz * hfovi + nsz * hfovj + nnz * hfovk;

  rtimgTransform->SetElement(0, 0, ntx);
  rtimgTransform->SetElement(1, 0, nty);
  rtimgTransform->SetElement(2, 0, ntz);
  rtimgTransform->SetElement(0, 1, nsx);
  rtimgTransform->SetElement(1, 1, nsy);
  rtimgTransform->SetElement(2, 1, nsz);
  rtimgTransform->SetElement(0, 2, nnx);
  rtimgTransform->SetElement(1, 2, nny);
  rtimgTransform->SetElement(2, 2, nnz);
  rtimgTransform->SetElement(0, 3, px + cx);
  rtimgTransform->SetElement(1, 3, py + cy);
  rtimgTransform->SetElement(2, 3, pz + cz);

  vtkMRMLLinearTransformNode* parentNode =
    vtkMRMLLinearTransformNode::SafeDownCast(volumeNode->GetParentTransformNode());
  if (parentNode)
    {
    vtkSmartPointer<vtkMatrix4x4> parentTransform = vtkSmartPointer<vtkMatrix4x4>::New();
    parentTransform->Identity();
    int r = parentNode->GetMatrixTransformToWorld(parentTransform);
    if (r)
      {
      vtkSmartPointer<vtkMatrix4x4> transform = vtkSmartPointer<vtkMatrix4x4>::New();
      vtkMatrix4x4::Multiply4x4(parentTransform, rtimgTransform,  transform);
      updateSlice(transform);
      return;
      }
    }

  updateSlice(rtimgTransform);

}



//------------------------------------------------------------------------------
qSlicerReslicePropertyWidget
::qSlicerReslicePropertyWidget( vtkSlicerVolumeResliceDriverLogic* logic, QWidget *_parent )
  : Superclass( new qSlicerReslicePropertyWidgetPrivate( *this ), _parent )
{
  Q_D(qSlicerReslicePropertyWidget);
  d->init();
  this->Logic = logic;
}



qSlicerReslicePropertyWidget
::~qSlicerReslicePropertyWidget()
{
  if ( this->Logic != NULL )
  {
    this->Logic = NULL;
  }
}



void qSlicerReslicePropertyWidget
::SetLogic( vtkSlicerVolumeResliceDriverLogic* logic )
{
  this->Logic = logic;
}



void qSlicerReslicePropertyWidget::setSliceViewName(const QString& newSliceViewName)
{
  Q_D(qSlicerReslicePropertyWidget);
  
  d->ViewLabel->setText(newSliceViewName);
}

//---------------------------------------------------------------------------
void qSlicerReslicePropertyWidget::setSliceViewColor(const QColor& newSliceViewColor)
{
  Q_D(qSlicerReslicePropertyWidget);

  //if (d->sliceNode)
  //  {
  //  qCritical() << "qMRMLSliceControllerWidget::setSliceViewColor should be called before setMRMLSliceNode !";
  //  return;
  //  }

  d->setColor(newSliceViewColor);
}

//------------------------------------------------------------------------------
void qSlicerReslicePropertyWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qSlicerReslicePropertyWidget);

  //d->SliceLogic->SetSliceNode(newSliceNode);
  d->sliceNode = newSliceNode;
  if (newSliceNode && newSliceNode->GetScene())
    {
    this->setMRMLScene(newSliceNode->GetScene());
    }
}

//----------------------------------------------------------------------------
void qSlicerReslicePropertyWidget::setMRMLScene(vtkMRMLScene * newScene)
{
  Q_D(qSlicerReslicePropertyWidget);

  if (d->scene != newScene)
    {
    d->scene = newScene;
    if (d->driverNodeSelector)
      {
      d->driverNodeSelector->setMRMLScene(newScene);
      }
    }
}



void qSlicerReslicePropertyWidget
::setDriverNode( vtkMRMLNode* newNode )
{
  Q_D(qSlicerReslicePropertyWidget);
  
  if ( d->sliceNode == NULL )
  {
    return;
  }
  
  if ( newNode == NULL )
  {
    this->Logic->SetDriverForSlice( "", d->sliceNode );
  }
  else
  {
    this->Logic->SetDriverForSlice( newNode->GetID(), d->sliceNode );
  }
}



void qSlicerReslicePropertyWidget
::onMRMLNodeModified()
{
  Q_D(qSlicerReslicePropertyWidget);
  
  /*
  if (!d->driverNode)
    {
    return;
    }
  if (d->sliceNode)
    {
    vtkMRMLLinearTransformNode* transNode = vtkMRMLLinearTransformNode::SafeDownCast(d->driverNode);
    if (transNode)
      {
      d->updateSliceByTransformNode(transNode);
      }
    vtkMRMLScalarVolumeNode* imageNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->driverNode);
    if (imageNode)
      {
      d->updateSliceByImageNode(imageNode);
      }
    }
  */
  
}



void qSlicerReslicePropertyWidget
::onMethodChanged()
{
  Q_D(qSlicerReslicePropertyWidget);
  
  this->Logic->SetMethodForSlice( d->methodButtonGroup.checkedId(), d->sliceNode );
}



void qSlicerReslicePropertyWidget
::onOrientationChanged()
{
  Q_D(qSlicerReslicePropertyWidget);
  
  this->Logic->SetOrientationForSlice( d->orientationButtonGroup.checkedId(), d->sliceNode );
}
