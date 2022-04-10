/*==============================================================================

  Copyright (c) Thomas Vaughan
  Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/


#ifndef __vtkMRMLCollectPointsNode_h
#define __vtkMRMLCollectPointsNode_h

// vtk includes
#include <vtkObject.h>
#include <vtkCommand.h>

// Slicer includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

// FiducialRegistrationWizard includes
#include "vtkSlicerCollectPointsModuleMRMLExport.h"

class VTK_SLICER_COLLECTPOINTS_MODULE_MRML_EXPORT vtkMRMLCollectPointsNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    /// The node stores both inputs (e.g., tooltip position, model, colors, etc.) and computed parameters.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 565 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 565
  };

  enum
  {
    Manual = 0,
    Automatic,
    CollectMode_Last // valid types go above this line
  };

  vtkTypeMacro( vtkMRMLCollectPointsNode, vtkMRMLNode );
  
  // Standard MRML node methods
  static vtkMRMLCollectPointsNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance() override;
  virtual const char* GetNodeTagName() override { return "CollectPoints"; };
  void PrintSelf( ostream& os, vtkIndent indent ) override;
  virtual void ReadXMLAttributes( const char** atts ) override;
  virtual void WriteXML( ostream& of, int indent ) override;
  virtual void Copy( vtkMRMLNode *node ) override;

protected:

  vtkMRMLCollectPointsNode();
  virtual ~vtkMRMLCollectPointsNode();
  vtkMRMLCollectPointsNode ( const vtkMRMLCollectPointsNode& );
  void operator=( const vtkMRMLCollectPointsNode& );

public:
  // The point will be collected in the anchor coordinate system
  // at the location indicated by this probe transform.
  vtkMRMLTransformNode* GetSamplingTransformNode();
  void SetAndObserveSamplingTransformNodeID( const char* nodeID );

  vtkMRMLTransformNode* GetAnchorTransformNode();
  void SetAndObserveAnchorTransformNodeID( const char* nodeID );
  
  // need to determine type at run-time
  void CreateDefaultDisplayNodesForOutputNode();
  vtkMRMLNode* GetOutputNode();
  void SetOutputNodeID( const char* nodeID );
  int GetNumberOfPointsInOutput();

  vtkGetMacro(LabelBase, std::string);
  vtkSetMacro(LabelBase, std::string);
  vtkGetMacro(NextLabelNumber, int);
  vtkSetMacro(NextLabelNumber, int);

  vtkGetMacro( CollectMode, int );
  vtkSetMacro( CollectMode, int );
  void SetCollectModeToManual() { this->SetCollectMode( Manual ); }
  void SetCollectModeToAutomatic() { this->SetCollectMode( Automatic ); }

  vtkGetMacro( MinimumDistance, double );
  vtkSetMacro( MinimumDistance, double );

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData ) override;

  static int GetCollectModeFromString( const char* name );
  static const char* GetCollectModeAsString( int id );

  // deprecated on March 8 2018
  vtkMRMLTransformNode* GetProbeTransformNode();
  void SetAndObserveProbeTransformNodeID( const char* nodeID );
  
private:
  // the next collected point will have label "[LabelBase]-[NextLabelNumber]"
  // The counter can be reset or changed by the user, if necessary
  // These are "advanced" options
  std::string LabelBase;
  int NextLabelNumber;

  // For automatic CollectMode only:
  // Only collect a new point if the probe transform has moved by
  // at least this much from the previous point
  double MinimumDistance;

  // Determine when new points are collected:
  // Manual - when the user clicks on "Collect"
  // Automatic - anytime the input probe transform or any of the parameters are changed
  int CollectMode;
};

#endif
