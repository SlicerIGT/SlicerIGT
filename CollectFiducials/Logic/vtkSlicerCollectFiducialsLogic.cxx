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

// CollectFiducials includes
#include "vtkSlicerCollectFiducialsLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>
#include <sstream>


vtkStandardNewMacro(vtkSlicerCollectFiducialsLogic);

//------------------------------------------------------------------------------
vtkSlicerCollectFiducialsLogic::vtkSlicerCollectFiducialsLogic()
{
}

//------------------------------------------------------------------------------
vtkSlicerCollectFiducialsLogic::~vtkSlicerCollectFiducialsLogic()
{
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::AddPoint( vtkMRMLCollectFiducialsNode* collectFiducialsNode )
{
  if ( collectFiducialsNode == NULL )
  {
    vtkErrorMacro( "No parameter node set. Aborting." );
    return;
  }

  // find the point coordinates
  vtkSmartPointer< vtkMRMLLinearTransformNode > probeNode = vtkMRMLLinearTransformNode::SafeDownCast( collectFiducialsNode->GetProbeTransformNode() );
  if ( probeNode == NULL )
  {
    vtkErrorMacro( "No probe transform node set. Aborting." );
    return;
  }
  vtkSmartPointer< vtkMatrix4x4 > probeToWorld = vtkSmartPointer< vtkMatrix4x4 >::New();
  probeNode->GetMatrixTransformToWorld( probeToWorld );
  double pointCoordinates[ 3 ] = { probeToWorld->GetElement( 0, 3 ), probeToWorld->GetElement( 1, 3 ), probeToWorld->GetElement( 2, 3 ) };

  // add the point to the markups node
  vtkMRMLMarkupsFiducialNode* outputMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( collectFiducialsNode->GetOutputNode() );
  if ( outputMarkupsNode == NULL )
  {
    vtkErrorMacro( "No output markups node set. Aborting." );
    return;
  }
  int pointIndexInMarkups = outputMarkupsNode->AddFiducialFromArray( pointCoordinates );

  // add the label to the point
  std::stringstream ss;
  ss << collectFiducialsNode->GetLabelBase() << collectFiducialsNode->GetLabelCounter();
  outputMarkupsNode->SetNthFiducialLabel( pointIndexInMarkups, ss.str().c_str() );

  // always increase the label counter automatically
  collectFiducialsNode->SetLabelCounter( collectFiducialsNode->GetLabelCounter() + 1 );

  //TODO: Add ability to change glyph scale when feature is added to Markups module
  //this->MarkupsFiducialNode-> ->SetGlyphScale( glyphScale );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::RegisterNodes()
{
  if( !this->GetMRMLScene() )
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLCollectFiducialsNode >::New() );
}

//------------------------------------------------------------------------------
void vtkSlicerCollectFiducialsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}
