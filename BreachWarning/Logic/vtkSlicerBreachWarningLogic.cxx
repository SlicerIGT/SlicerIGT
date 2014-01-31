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

// BreachWarning includes
#include "vtkSlicerBreachWarningLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkPCAStatistics.h>
#include <vtkObjectFactory.h>

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

vtkStandardNewMacro(vtkSlicerBreachWarningLogic);



vtkSlicerBreachWarningLogic::vtkSlicerBreachWarningLogic()
{
}



vtkSlicerBreachWarningLogic::~vtkSlicerBreachWarningLogic()
{
}



void vtkSlicerBreachWarningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


void vtkSlicerBreachWarningLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerBreachWarningLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLBreachWarningNode* frwNode = vtkMRMLBreachWarningNode::New();
  this->GetMRMLScene()->RegisterNodeClass( frwNode );
  frwNode->Delete();
}



void vtkSlicerBreachWarningLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}



void vtkSlicerBreachWarningLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

// Variable getters and setters -----------------------------------------------------
// Note: vtkSetMacro doesn't call a modified event if the replacing value is the same as before
// We want a modified event always
std::string vtkSlicerBreachWarningLogic
::GetOutputMessage( std::string nodeID )
{
  return this->OutputMessages[ nodeID ];
}


void vtkSlicerBreachWarningLogic
::SetOutputMessage( std::string nodeID, std::string newOutputMessage )
{
  this->OutputMessages[ nodeID ] = newOutputMessage;
  this->Modified();
}



// Module-specific methods ----------------------------------------------------------

void vtkSlicerBreachWarningLogic
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


void vtkSlicerBreachWarningLogic
::CalculateTransform( vtkMRMLNode* node )
{
  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( node );
  if ( BreachWarningNode == NULL )
  {
    this->SetOutputMessage( BreachWarningNode->GetID(), "Failed to find module node." ); // Note: This should never happen
    return;
  }

  vtkMRMLMarkupsFiducialNode* fromMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( BreachWarningNode->GetFromFiducialListID() ) );
  vtkMRMLMarkupsFiducialNode* toMarkupsFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( BreachWarningNode->GetToFiducialListID() ) );
  vtkMRMLLinearTransformNode* outputTransform = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( BreachWarningNode->GetOutputTransformID() ) );
  std::string transformType = BreachWarningNode->GetRegistrationMode();


  if ( fromMarkupsFiducialNode == NULL || toMarkupsFiducialNode == NULL )
  {
    this->SetOutputMessage( BreachWarningNode->GetID(), "One or more fiducial lists not defined." );
    return;
  }

  if ( outputTransform == NULL )
  {
    this->SetOutputMessage( BreachWarningNode->GetID(), "Output transform is not defined." );
    return;
  }

  if ( fromMarkupsFiducialNode->GetNumberOfFiducials() < 3 || toMarkupsFiducialNode->GetNumberOfFiducials() < 3 )
  {
    this->SetOutputMessage( BreachWarningNode->GetID(), "One or more fiducial lists has too few fiducials (minimum 3 required)." );
    return;
  }

  if ( fromMarkupsFiducialNode->GetNumberOfFiducials() != toMarkupsFiducialNode->GetNumberOfFiducials() )
  {
    this->SetOutputMessage( BreachWarningNode->GetID(), "Fiducial lists have unequal number of fiducials." );
    return;
  }

  // Convert the markupsfiducial nodes into vector of itk points
  vtkPoints* fromPoints = MarkupsFiducialNodeToVTKPoints( fromMarkupsFiducialNode );
  vtkPoints* toPoints = MarkupsFiducialNodeToVTKPoints( toMarkupsFiducialNode );

  if ( this->CheckCollinear( fromPoints ) || this->CheckCollinear( toPoints ) )
  {
    this->SetOutputMessage( BreachWarningNode->GetID(), "One or more fiducial lists have strictly collinear points." );
    return;
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
  this->SetOutputMessage( BreachWarningNode->GetID(), successMessage.str() );
}


double vtkSlicerBreachWarningLogic
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


bool vtkSlicerBreachWarningLogic
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

void vtkSlicerBreachWarningLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLBreachWarningNode* callerNode = vtkMRMLBreachWarningNode::SafeDownCast( caller );
  // The caller must be a vtkMRMLBreachWarningNode
  if ( callerNode != NULL )
  {
    this->CalculateTransform( callerNode ); // Will create modified event to update widget
  }
}


void vtkSlicerBreachWarningLogic
::ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLScene* callerNode = vtkMRMLScene::SafeDownCast( caller );

  // If the added node was a fiducial registration wizard node then observe it
  vtkMRMLNode* addedNode = reinterpret_cast< vtkMRMLNode* >( callData );
  vtkMRMLBreachWarningNode* BreachWarningNode = vtkMRMLBreachWarningNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && BreachWarningNode != NULL )
  {
    // This will get called exactly once, and we will add the observer only once (since node is never replaced)
    BreachWarningNode->AddObserver( vtkCommand::ModifiedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
    BreachWarningNode->UpdateScene( this->GetMRMLScene() );
    BreachWarningNode->ObserveAllReferenceNodes(); // This will update
    this->CalculateTransform( BreachWarningNode ); // Will create modified event to update widget
  }

}