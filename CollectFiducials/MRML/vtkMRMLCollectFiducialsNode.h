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


#ifndef __vtkMRMLCollectFiducialsNode_h
#define __vtkMRMLCollectFiducialsNode_h

// vtk includes
#include <vtkObject.h>
#include <vtkCommand.h>

// Slicer includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

// FiducialRegistrationWizard includes
#include "vtkSlicerCollectFiducialsModuleMRMLExport.h"

class VTK_SLICER_COLLECTFIDUCIALS_MODULE_MRML_EXPORT vtkMRMLCollectFiducialsNode
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

  vtkTypeMacro( vtkMRMLCollectFiducialsNode, vtkMRMLNode );
  
  // Standard MRML node methods
  static vtkMRMLCollectFiducialsNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "CollectFiducials"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:

  vtkMRMLCollectFiducialsNode();
  virtual ~vtkMRMLCollectFiducialsNode();
  vtkMRMLCollectFiducialsNode ( const vtkMRMLCollectFiducialsNode& );
  void operator=( const vtkMRMLCollectFiducialsNode& );

public:
  // The point will be collected in the world coordinate system
  // at the location indicated by this probe transform.
  vtkMRMLLinearTransformNode* GetProbeTransformNode();
  void SetAndObserveProbeTransformNodeId( const char* nodeId );
  
  // TODO: the output node can either be of type vtkMRMLMarkupsNode, or vtkMRMLModelNode
  // need to determine type at run-time
  vtkMRMLMarkupsFiducialNode* GetOutputNode();
  void SetOutputNodeId( const char* nodeId );

  vtkGetMacro(LabelBase, std::string);
  vtkSetMacro(LabelBase, std::string);
  vtkGetMacro(LabelCounter, int);
  vtkSetMacro(LabelCounter, int);

  vtkGetMacro( CollectMode, int );
  vtkSetMacro( CollectMode, int );
  void SetCollectModeToManual() { this->SetCollectMode( Manual ); }
  void SetCollectModeToAutomatic() { this->SetCollectMode( Automatic ); }

  vtkGetMacro( MinimumDistanceMm, double );
  vtkSetMacro( MinimumDistanceMm, double );

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

  static int GetCollectModeFromString( const char* name );
  static const char* GetCollectModeAsString( int id );
  
private:
  // the next collected point will have label "[LabelBase]-[LabelCounter]"
  // The counter can be reset or changed by the user, if necessary
  // These are "advanced" options
  std::string LabelBase;
  int LabelCounter;

  // For automatic CollectMode only:
  // Only collect a new point if the probe transform has moved by
  // at least this much from the previous point
  double MinimumDistanceMm;

  // Determine when new points are collected:
  // Manual - when the user clicks on "Collect"
  // Automatic - anytime the input probe transform or any of the parameters are changed
  int CollectMode;
};

#endif
