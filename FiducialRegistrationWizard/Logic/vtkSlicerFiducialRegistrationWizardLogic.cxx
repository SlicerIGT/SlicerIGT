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

// FiducialRegistrationWizard includes
#include "vtkSlicerFiducialRegistrationWizardLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkPCAStatistics.h>

// STD includes
#include <cassert>
#include <sstream>


// Helper methods -------------------------------------------------------------------

double EIGENVALUE_THRESHOLD = 1e-4;

vtkPoints* MarkupsFiducialNodeToVTKPoints( vtkMRMLMarkupsFiducialNode* markupsFiducialNode )
{
  vtkPoints* points = vtkPoints::New();
  for ( int i = 0; i < markupsFiducialNode->GetNumberOfFiducials(); i++ )
  {
    double currentFiducial[ 3 ] = { 0, 0, 0 };
    markupsFiducialNode->GetNthFiducialPosition( i, currentFiducial );
    points->InsertNextPoint( currentFiducial );
  }

  return points;
}


// Slicer methods -------------------------------------------------------------------

vtkStandardNewMacro(vtkSlicerFiducialRegistrationWizardLogic);



vtkSlicerFiducialRegistrationWizardLogic::vtkSlicerFiducialRegistrationWizardLogic()
{
}



vtkSlicerFiducialRegistrationWizardLogic::~vtkSlicerFiducialRegistrationWizardLogic()
{
}



void vtkSlicerFiducialRegistrationWizardLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


void vtkSlicerFiducialRegistrationWizardLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerFiducialRegistrationWizardLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLFiducialRegistrationWizardNode* frwNode = vtkMRMLFiducialRegistrationWizardNode::New();
  this->GetMRMLScene()->RegisterNodeClass( frwNode );
  frwNode->Delete();
}



void vtkSlicerFiducialRegistrationWizardLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerFiducialRegistrationWizardLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}



void vtkSlicerFiducialRegistrationWizardLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}


// Module-specific methods ----------------------------------------------------------

void vtkSlicerFiducialRegistrationWizardLogic
::AddFiducial( vtkMRMLLinearTransformNode* probeTransformNode )
{
  if ( probeTransformNode == NULL )
  {
    return;
  }

  vtkMRMLMarkupsFiducialNode* activeMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( this->MarkupsLogic->GetActiveListID() ) );

  if ( activeMarkupsFiducialNode == NULL )
  {
    return;
  }
  
  vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
  probeTransformNode->GetMatrixTransformToWorld( transformToWorld );
  
  double coord[ 3 ] = { transformToWorld->GetElement( 0, 3 ), transformToWorld->GetElement( 1, 3 ), transformToWorld->GetElement( 2, 3 ) };

  activeMarkupsFiducialNode->AddFiducialFromArray( coord );

  transformToWorld->Delete();
}


std::string vtkSlicerFiducialRegistrationWizardLogic
::CalculateTransform( vtkMRMLMarkupsFiducialNode* fromMarkupsFiducialNode, vtkMRMLMarkupsFiducialNode* toMarkupsFiducialNode, vtkMRMLLinearTransformNode* outputTransform, std::string transformType )
{

  if ( fromMarkupsFiducialNode == NULL || toMarkupsFiducialNode == NULL )
  {
    return "One or more fiducial lists not defined.";
  }

  if ( outputTransform == NULL )
  {
    return "Output transform is not defined.";
  }

  if ( fromMarkupsFiducialNode->GetNumberOfFiducials() < 3 || toMarkupsFiducialNode->GetNumberOfFiducials() < 3 )
  {
    return "One or more fiducial lists has too few fiducials (minimum 3 required).";
  }

  if ( fromMarkupsFiducialNode->GetNumberOfFiducials() != toMarkupsFiducialNode->GetNumberOfFiducials() )
  {
    return "Fiducial lists have unequal number of fiducials.";
  }

  // Convert the markupsfiducial nodes into vector of itk points
  vtkPoints* fromPoints = MarkupsFiducialNodeToVTKPoints( fromMarkupsFiducialNode );
  vtkPoints* toPoints = MarkupsFiducialNodeToVTKPoints( toMarkupsFiducialNode );

  if ( this->CheckCollinear( fromPoints ) || this->CheckCollinear( toPoints ) )
  {
    return "One or more fiducial lists have strictly collinear points.";
  }

  // Setup the registration
  vtkLandmarkTransform* transform = vtkLandmarkTransform::New();

  transform->SetSourceLandmarks( fromPoints );
  transform->SetTargetLandmarks( toPoints );

  if ( transformType.compare( "Similarity" ) == 0 )
  {
    transform->SetModeToSimilarity();
  }
  else
  {
    transform->SetModeToRigidBody();
  }

  transform->Update();

  // Copy the resulting transform into the outputTransform
  vtkMatrix4x4* calculatedTransform = vtkMatrix4x4::New();
  transform->GetMatrix( calculatedTransform );
  outputTransform->SetAndObserveMatrixTransformToParent( calculatedTransform );

  double rmsError = this->CalculateRegistrationError( fromPoints,toPoints, transform );

  // Delete stuff // TODO: Use smart pointers
  fromPoints->Delete();
  toPoints->Delete();
  transform->Delete();
  calculatedTransform->Delete();

  std::stringstream successMessage;
  successMessage << "Success! RMS Error: " << rmsError;
  return successMessage.str();
}


double vtkSlicerFiducialRegistrationWizardLogic
::CalculateRegistrationError( vtkPoints* fromPoints, vtkPoints* toPoints, vtkLinearTransform* transform )
{
  // Transform the from points
  vtkPoints* transformedFromPoints = vtkPoints::New();
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

  // Delete // TODO: User smart pointers
  transformedFromPoints->Delete();

  return sqrt( sumSquaredError / toPoints->GetNumberOfPoints() );
}


bool vtkSlicerFiducialRegistrationWizardLogic
::CheckCollinear( vtkPoints* points )
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

  // Setup the principal component analysis
  vtkSmartPointer< vtkPCAStatistics > pcaStatistics = vtkSmartPointer< vtkPCAStatistics >::New();
  pcaStatistics->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, arrayTable );
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

  return false;
}


// Node update methods ----------------------------------------------------------

vtkMRMLNode* vtkSlicerFiducialRegistrationWizardLogic
::GetFiducialRegistrationWizardNode()
{
  vtkSmartPointer< vtkMRMLNode > fiducialRegistrationWizardNode;
  fiducialRegistrationWizardNode = this->GetMRMLScene()->GetSingletonNode( "FiducialRegistrationWizard", "vtkMRMLFiducialRegistrationWizardNode" );

  // Add a new node to the scene if one doesn't already exist
  if ( fiducialRegistrationWizardNode == NULL )
  {
    fiducialRegistrationWizardNode.TakeReference( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLFiducialRegistrationWizardNode" ) );
    fiducialRegistrationWizardNode->SetSingletonTag( "FiducialRegistrationWizard" );
    fiducialRegistrationWizardNode->SetScene( this->GetMRMLScene() );
    this->GetMRMLScene()->AddNode( fiducialRegistrationWizardNode );
  }

  return fiducialRegistrationWizardNode;
}