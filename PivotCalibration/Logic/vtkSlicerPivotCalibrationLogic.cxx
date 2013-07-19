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

// PivotCalibration Logic includes
#include "vtkSlicerPivotCalibrationLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>

#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <cassert>

// ITK includes
#include "itkVersorRigid3DTransform.h"

#include "vnl/algo/vnl_svd.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"

// Callback for the interaction
class vtkAddSampleCallback : public vtkCommand
{
public:
  static vtkAddSampleCallback *New()
    { return new vtkAddSampleCallback; }
  
  vtkAddSampleCallback(){}
    
  void Execute(vtkObject* caller, unsigned long event, void*)
  {
    //qSlicerPivotCalibrationModuleWidget* widget = reinterpret_cast<qSlicerPivotCalibrationModuleWidget*>(caller);
    //widget->AddSample();
    this->pivotCalibrationLogic->Test();
    
    vtkMatrix4x4* matrixToParent = this->transformNode->GetMatrixTransformToParent();
    vtkMatrix4x4* matrixCopy = vtkMatrix4x4::New();
    matrixCopy->DeepCopy(matrixToParent);
    
    //pivotCalibrationLogic->AddSample(matrixCopy);
  }
  
  void SetLogic(vtkSlicerPivotCalibrationLogic* logic)
  {
    this->pivotCalibrationLogic = logic;
  }
  
  void SetNode(vtkMRMLLinearTransformNode* node)
  {
    this->transformNode = node;
  }  
  
  vtkSlicerPivotCalibrationLogic* pivotCalibrationLogic;
  vtkMRMLLinearTransformNode* transformNode;
  
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPivotCalibrationLogic);

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::vtkSlicerPivotCalibrationLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::~vtkSlicerPivotCalibrationLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf( os, indent );
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::InitializeObserver(vtkMRMLNode* node)
{
  //vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
  vtkSmartPointer<vtkMRMLLinearTransformNode> testNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
  this->GetMRMLScene()->AddNode(testNode);
  
  vtkAddSampleCallback* callback = vtkAddSampleCallback::New();
  callback->SetLogic(this);
  callback->SetNode(testNode);
  testNode->AddObserver(vtkCommand::ModifiedEvent, callback);
  //callback->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::AddSample(vtkMatrix4x4* transformMatrix)
{
  this->transforms.push_back(transformMatrix);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ComputeCalibration()
{
  unsigned int rows = 3*this->transforms.size();
  unsigned int columns = 6;

  vn1_matrix<double> A(rows, columns), minusI(3,3,0), R(3,3);
  vn1_vector<double> b(rows), x(columns), t(3);
  minusI(0, 0) = -1;
  minusI(1, 1) = -1;
  minusI(2, 2) = -1;
  
  std::vector<vtkMatrix4x4*>::const_iterator it, transformsEnd = this->Transforms.end();
  unsigned int currentRow;
  for(currentRow = 0, it = this->transforms.begin(); it != transformsEnd; it++, currentRow += 3)
  {
    for (int i = 0; i < 3; i++)
    {
      t(i) = (*it)->GetElement(i, 3);
    }
    t *= -1;
    b.update(t, currentRow);

    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++ )
      {
        R(i, j) = (*it)->GetElement(i, j);
      }
    }
    A.update(R, currentRow, 0);
    A.update( minusI, currentRow, 3 );    
  }
  
  vnl_svd<double> svdA(A);
  
  svdA.zero_out_absolute( 1e-1 );
  
  x = svdA.solve( b );
  
  //set the RMSE
  this->RMSE = ( A * x - b ).rms();

  //set the transformation
  this->Translation[0] = x[0];
  this->Translation[1] = x[1];
  this->Translation[2] = x[2];

  //set the pivot point
  this->PivotPosition[0] = x[3];
  this->PivotPosition[1] = x[4];
  this->PivotPosition[2] = x[5];  
}
  
/*
//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::OnMRMLSceneEndImport()
{
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::OnMRMLSceneStartClose()
{
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::OnMRMLSceneEndClose()
{
  this->Modified();
}
//*/

//Add new functions here




/*
//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}
//*/

