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
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

// ITK includes
#include "itkVersorRigid3DTransform.h"
#include "itkPluginUtilities.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPivotCalibrationLogic);

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::vtkSlicerPivotCalibrationLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPivotCalibrationLogic::~vtkSlicerPivotCalibrationLogic()
{
  this->ClearSamples();
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
void vtkSlicerPivotCalibrationLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  if (caller != NULL)
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
    if (this->recordingState == true && transformNode->GetID() == this->transformId)
    {
      vtkMatrix4x4* matrixToParent = transformNode->GetMatrixTransformToParent();
      vtkMatrix4x4* matrixCopy = vtkMatrix4x4::New();
      matrixCopy->DeepCopy(matrixToParent);
      
      this->AddSample(matrixCopy);
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::InitializeObserver(vtkMRMLNode* node)
{
  if (node != NULL)
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
    
    this->transformId = transformNode->GetID();
    
    node->AddObserver(vtkMRMLLinearTransformNode::TransformModifiedEvent, (vtkCommand*) this->GetMRMLNodesCallbackCommand());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::AddSample(vtkMatrix4x4* transformMatrix)
{
  this->transforms.push_back(transformMatrix);
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ClearSamples()
{
  std::vector<vtkMatrix4x4*>::const_iterator it, transformsEnd = this->transforms.end();
  for(it = this->transforms.begin(); it != transformsEnd; it++)
  {
    (*it)->Delete();
  }
  this->transforms.clear();
}

//---------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::ComputeCalibration()
{
  if (this->transforms.size() > 0)
  {
    unsigned int rows = 3*this->transforms.size();
    unsigned int columns = 6;

    vnl_matrix<double> A(rows, columns), minusI(3,3,0), R(3,3);
    vnl_vector<double> b(rows), x(columns), t(3);
    minusI(0, 0) = -1;
    minusI(1, 1) = -1;
    minusI(2, 2) = -1;
    
    
    std::vector<vtkMatrix4x4*>::const_iterator it, transformsEnd = this->transforms.end();
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
}

//-----------------------------------------------------------------------------
void vtkSlicerPivotCalibrationLogic::setRecordingState(bool state)
{
  this->recordingState = state;
}


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

