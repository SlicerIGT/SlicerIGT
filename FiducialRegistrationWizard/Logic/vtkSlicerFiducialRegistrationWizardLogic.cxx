/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/


// FiducialRegistrationWizard includes
#include "vtkSlicerFiducialRegistrationWizardLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPCAStatistics.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>
#include <sstream>


// Helper methods -------------------------------------------------------------------

double EIGENVALUE_THRESHOLD = 1e-4;

//------------------------------------------------------------------------------
void MarkupsFiducialNodeToVTKPoints( vtkMRMLMarkupsFiducialNode* markupsFiducialNode, vtkPoints* points )
{
  points->Reset();
  for ( int i = 0; i < markupsFiducialNode->GetNumberOfFiducials(); i++ )
  {
    double currentFiducial[ 3 ] = { 0, 0, 0 };
    markupsFiducialNode->GetNthFiducialPosition( i, currentFiducial );
    points->InsertNextPoint( currentFiducial );
  }
}


// Slicer methods -------------------------------------------------------------------

vtkStandardNewMacro(vtkSlicerFiducialRegistrationWizardLogic);

//------------------------------------------------------------------------------
vtkSlicerFiducialRegistrationWizardLogic::vtkSlicerFiducialRegistrationWizardLogic()
: MarkupsLogic(NULL)
{
}

//------------------------------------------------------------------------------
vtkSlicerFiducialRegistrationWizardLogic::~vtkSlicerFiducialRegistrationWizardLogic()
{
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLFiducialRegistrationWizardNode >::New() );
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLFiducialRegistrationWizardNode* frwNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast(node);
  if ( frwNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( frwNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    events->InsertNextValue( vtkMRMLFiducialRegistrationWizardNode::InputDataModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( frwNode, events.GetPointer() );
    
    if ( frwNode->GetUpdateMode() == vtkMRMLFiducialRegistrationWizardNode::UPDATE_MODE_AUTO )
    {
      this->UpdateCalibration( frwNode ); // Will create modified event to update widget
    }
  }
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeRemoved: Invalid MRML scene or node" );
    return;
  }

  if ( node->IsA( "vtkMRMLFiducialRegistrationWizardNode" ) )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
} 

//------------------------------------------------------------------------------
std::string vtkSlicerFiducialRegistrationWizardLogic::GetOutputMessage( std::string nodeID )
{
  vtkMRMLFiducialRegistrationWizardNode* node = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ) );
  if (node==NULL)
  {
    vtkWarningMacro("vtkSlicerFiducialRegistrationWizardLogic::GetOutputMessage failed: vtkMRMLFiducialRegistrationWizardNode with the specified ID ("<<nodeID<<") not found");
    return "";
  }
  return node->GetCalibrationStatusMessage();
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::AddFiducial( vtkMRMLLinearTransformNode* probeTransformNode )
{
  if ( probeTransformNode == NULL )
  {
    vtkWarningMacro("vtkSlicerFiducialRegistrationWizardLogic::AddFiducial failed: input transform is invalid");
    return;
  }

  vtkMRMLMarkupsFiducialNode* activeMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( this->MarkupsLogic->GetActiveListID() ) );
  if ( activeMarkupsFiducialNode == NULL )
  {
    vtkWarningMacro("vtkSlicerFiducialRegistrationWizardLogic::AddFiducial failed: no active markup list is found");
    return;
  }
  
  this->AddFiducial(probeTransformNode, activeMarkupsFiducialNode);
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::AddFiducial( vtkMRMLLinearTransformNode* probeTransformNode, vtkMRMLMarkupsFiducialNode* fiducialNode )
{
  if ( probeTransformNode == NULL )
  {
    vtkErrorMacro("vtkSlicerFiducialRegistrationWizardLogic::AddFiducial failed: input transform is invalid");
    return;
  }
  if ( fiducialNode == NULL )
  {
    vtkErrorMacro("vtkSlicerFiducialRegistrationWizardLogic::AddFiducial failed: output fiducial node is invalid");
    return;
  }
  
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  probeTransformNode->GetMatrixTransformToWorld( transformToWorld );

  double coord[3] = { transformToWorld->GetElement( 0, 3 ), transformToWorld->GetElement( 1, 3 ), transformToWorld->GetElement( 2, 3 ) };
  fiducialNode->AddFiducialFromArray( coord );
}

//------------------------------------------------------------------------------
bool vtkSlicerFiducialRegistrationWizardLogic::UpdateCalibration( vtkMRMLNode* node )
{
  vtkMRMLFiducialRegistrationWizardNode* fiducialRegistrationWizardNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast( node );
  if ( fiducialRegistrationWizardNode == NULL )
  {
    vtkWarningMacro("vtkSlicerFiducialRegistrationWizardLogic::UpdateCalibration failed: input node is invalid");
    return false;
  }

  vtkMRMLMarkupsFiducialNode* fromMarkupsFiducialNode = fiducialRegistrationWizardNode->GetFromFiducialListNode();
  vtkMRMLMarkupsFiducialNode* toMarkupsFiducialNode = fiducialRegistrationWizardNode->GetToFiducialListNode();
  vtkMRMLTransformNode* outputTransformNode = fiducialRegistrationWizardNode->GetOutputTransformNode();
  int registrationMode = fiducialRegistrationWizardNode->GetRegistrationMode();

  if ( fromMarkupsFiducialNode == NULL )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("'From' fiducial list is not defined." );
    return false;
  }

  if ( toMarkupsFiducialNode == NULL )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("'To' fiducial list is not defined." );
    return false;
  }

  if ( outputTransformNode == NULL )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("Output transform is not defined." );
    return false;
  }

  if ( fromMarkupsFiducialNode->GetNumberOfFiducials() < 3 )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("'From' fiducial list has too few fiducials (minimum 3 required)." );
    return false;
  }
  if ( toMarkupsFiducialNode->GetNumberOfFiducials() < 3 )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("'To' fiducial list has too few fiducials (minimum 3 required)." );
    return false;
  }
  if ( fromMarkupsFiducialNode->GetNumberOfFiducials() != toMarkupsFiducialNode->GetNumberOfFiducials() )
  {
    std::stringstream msg;
    msg << "Fiducial lists have unequal number of fiducials ('From' has "<<fromMarkupsFiducialNode->GetNumberOfFiducials()
      <<", 'To' has " << toMarkupsFiducialNode->GetNumberOfFiducials() << ").";
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage(msg.str());
    return false;
  }

  // Convert the markupsfiducial nodes into vector of itk points
  vtkNew<vtkPoints> fromPoints;
  vtkNew<vtkPoints> toPoints;
  MarkupsFiducialNodeToVTKPoints( fromMarkupsFiducialNode, fromPoints.GetPointer() );
  MarkupsFiducialNodeToVTKPoints( toMarkupsFiducialNode, toPoints.GetPointer() );

  if ( this->CheckCollinear( fromPoints.GetPointer() ) )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("'From' fiducial list has strictly collinear points.");
    return false;
  }

  if ( this->CheckCollinear( toPoints.GetPointer() ) )
  {
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("'To' fiducial list has strictly collinear points.");
    return false;
  }

  if ( registrationMode == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_RIGID || 
       registrationMode == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_SIMILARITY )
  {
    // Compute transformation matrix. We don't set the landmark transform in the node directly because
    // vtkLandmarkTransform is not fully supported (e.g., it cannot be stored in file).
    vtkNew<vtkLandmarkTransform> landmarkTransform;
    landmarkTransform->SetSourceLandmarks( fromPoints.GetPointer() );
    landmarkTransform->SetTargetLandmarks( toPoints.GetPointer() );
    if ( registrationMode == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_RIGID )
    {
      landmarkTransform->SetModeToRigidBody();
    }
    else
    {
      landmarkTransform->SetModeToSimilarity();
    }
    landmarkTransform->Update();
    vtkNew<vtkMatrix4x4> calculatedTransform;
    landmarkTransform->GetMatrix(calculatedTransform.GetPointer());

    // Copy the resulting transform into the outputTransformNode
    if (!outputTransformNode->IsLinear())
    {
      // SetMatrix... only works on linear transforms, if we have a non-linear transform
      // in the node then we have to manually place a linear transform into it
      vtkNew<vtkTransform> newLinearTransform;
      newLinearTransform->SetMatrix(calculatedTransform.GetPointer());
      outputTransformNode->SetAndObserveTransformToParent(newLinearTransform.GetPointer());
    }
    else
    {
      outputTransformNode->SetMatrixTransformToParent(calculatedTransform.GetPointer());
    }
    
  }
  else if ( registrationMode == vtkMRMLFiducialRegistrationWizardNode::REGISTRATION_MODE_WARPING )
  {
    if (strcmp(outputTransformNode->GetClassName(), "vtkMRMLTransformNode") != 0)
    {
      vtkErrorMacro("vtkSlicerFiducialRegistrationWizardLogic::UpdateCalibration failed to save vtkThinPlateSplineTransform into transform node type "<<outputTransformNode->GetClassName());
      fiducialRegistrationWizardNode->SetCalibrationStatusMessage("Warping transform cannot be stored\nin linear transform node" );
      return false;
    }

    // Setup the transform
    // Warping transforms are usually defined using FromParent direction to make transformation of images faster and more accurate.
    vtkThinPlateSplineTransform* tpsTransform = vtkThinPlateSplineTransform::SafeDownCast(
      outputTransformNode->GetTransformFromParentAs("vtkThinPlateSplineTransform",
      false /* don't report conversion error */,
      true /* we need a modifiable transform */));
    if (tpsTransform == NULL)
    {
      // we cannot reuse the existing transform, create a new one
      vtkNew<vtkThinPlateSplineTransform> newTpsTransform;
      newTpsTransform->SetBasisToR();
      tpsTransform = newTpsTransform.GetPointer();
      outputTransformNode->SetAndObserveTransformFromParent(tpsTransform);
    }

    // Set inputs
    tpsTransform->SetSourceLandmarks( toPoints.GetPointer() );
    tpsTransform->SetTargetLandmarks( fromPoints.GetPointer() );
    tpsTransform->Update();
  }
  else
  {
    vtkErrorMacro( "vtkSlicerFiducialRegistrationWizardLogic::UpdateCalibration failed to set transform type: " <<
                   "invalid registration mode: " << vtkMRMLFiducialRegistrationWizardNode::RegistrationModeAsString( registrationMode ) );
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage( "Invalid transform type." );
    return false;
  }

  vtkAbstractTransform* outputTransform = outputTransformNode->GetTransformToParent();
  if (outputTransform == NULL)
  {
    vtkErrorMacro("Failed to retreive transform from node. RMS Error could not be evaluated");
    fiducialRegistrationWizardNode->SetCalibrationStatusMessage("Failed to retreive transform from node. RMS Error could not be evaluated.");
    return false;
  }

  std::stringstream successMessage;
  double rmsError = this->CalculateRegistrationError(fromPoints.GetPointer(), toPoints.GetPointer(), outputTransform);
  successMessage << "Success! RMS Error: " << rmsError;
  fiducialRegistrationWizardNode->SetCalibrationStatusMessage(successMessage.str());
  return true;
}

//------------------------------------------------------------------------------
double vtkSlicerFiducialRegistrationWizardLogic::CalculateRegistrationError( vtkPoints* fromPoints, vtkPoints* toPoints, vtkAbstractTransform* transform )
{
  // Transform the from points
  vtkSmartPointer<vtkPoints> transformedFromPoints = vtkSmartPointer<vtkPoints>::New();
  transform->TransformPoints( fromPoints, transformedFromPoints );

  // Calculate the RMS distance between the to points and the transformed from points
  double sumSquaredError = 0;
  for ( int i = 0; i < toPoints->GetNumberOfPoints(); i++ )
  {
    double currentToPoint[3] = { 0, 0, 0 };
    toPoints->GetPoint( i, currentToPoint );
    double currentTransformedFromPoint[3] = { 0, 0, 0 };
    transformedFromPoints->GetPoint( i, currentTransformedFromPoint );
    
    sumSquaredError += vtkMath::Distance2BetweenPoints( currentToPoint, currentTransformedFromPoint );
  }

  return sqrt( sumSquaredError / toPoints->GetNumberOfPoints() );
}

//------------------------------------------------------------------------------
bool vtkSlicerFiducialRegistrationWizardLogic::CheckCollinear( vtkPoints* points )
{
  // Initialize the x,y,z arrays for computing the PCA statistics
  vtkSmartPointer< vtkDoubleArray > xArray = vtkSmartPointer< vtkDoubleArray >::New();
  xArray->SetName( "xArray" );
  vtkSmartPointer< vtkDoubleArray > yArray = vtkSmartPointer< vtkDoubleArray >::New();
  yArray->SetName( "yArray" );
  vtkSmartPointer< vtkDoubleArray > zArray = vtkSmartPointer< vtkDoubleArray >::New();
  zArray->SetName( "zArray" );

  // Put the fiducial position values into the arrays
  double fiducialPosition[ 3 ] = { 0, 0, 0 };
  for ( int i = 0; i < points->GetNumberOfPoints(); i++ )
  {
    points->GetPoint( i, fiducialPosition );
    xArray->InsertNextValue( fiducialPosition[ 0 ] );
    yArray->InsertNextValue( fiducialPosition[ 1 ] );
    zArray->InsertNextValue( fiducialPosition[ 2 ] );
  }

  // Aggregate the arrays
  vtkSmartPointer< vtkTable > arrayTable = vtkSmartPointer< vtkTable >::New();
  arrayTable->AddColumn( xArray );
  arrayTable->AddColumn( yArray );
  arrayTable->AddColumn( zArray );
  
  /*
  // Setup the principal component analysis
  vtkSmartPointer< vtkPCAStatistics > pcaStatistics = vtkSmartPointer< vtkPCAStatistics >::New();
  pcaStatistics->SetInputData( vtkStatisticsAlgorithm::INPUT_DATA, arrayTable );
  pcaStatistics->SetColumnStatus( "xArray", 1 );
  pcaStatistics->SetColumnStatus( "yArray", 1 );
  pcaStatistics->SetColumnStatus( "zArray", 1 );
  pcaStatistics->SetDeriveOption( true );
  pcaStatistics->Update();

  // Calculate the eigenvalues
  vtkSmartPointer< vtkDoubleArray > eigenvalues = vtkSmartPointer< vtkDoubleArray >::New();
  pcaStatistics->GetEigenvalues( eigenvalues ); // Eigenvalues are largest to smallest
  
  // Test that each eigenvalues is bigger than some threshold
  int goodEigenvalues = 0;
  for ( int i = 0; i < eigenvalues->GetNumberOfTuples(); i++ )
  {
    if ( abs( eigenvalues->GetValue( i ) ) > EIGENVALUE_THRESHOLD )
    {
      goodEigenvalues++;
    }
  }

  if ( goodEigenvalues <= 1 )
  {
    return true;
  }
  */
  return false;
  
}

//------------------------------------------------------------------------------
void vtkSlicerFiducialRegistrationWizardLogic::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLFiducialRegistrationWizardNode* frwNode = vtkMRMLFiducialRegistrationWizardNode::SafeDownCast(caller);
  if ( frwNode == NULL)
  {
    return;
  }
  
  if (event==vtkMRMLFiducialRegistrationWizardNode::InputDataModifiedEvent)
  {
    // only recompute output if the input is changed
    // (for example we do not recompute the calibration output if the computed calibration transform or status message is changed)
    if ( frwNode->GetUpdateMode() == vtkMRMLFiducialRegistrationWizardNode::UPDATE_MODE_AUTO )
    {
      this->UpdateCalibration( frwNode ); // Will create modified event to update widget
    }
  }
}
