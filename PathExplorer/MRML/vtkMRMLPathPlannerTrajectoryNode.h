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
 
  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 8P41EB015898
 
==============================================================================*/

#ifndef __vtkMRMLPathPlannerTrajectoryNode_h
#define __vtkMRMLPathPlannerTrajectoryNode_h

#include "vtkSlicerPathExplorerModuleMRMLExport.h"

#include "vtkCommand.h"
#include "vtkMRMLDisplayableNode.h"

#include <deque>

class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsLineNode;
 
class  VTK_SLICER_PATHEXPLORER_MODULE_MRML_EXPORT vtkMRMLPathPlannerTrajectoryNode
  : public vtkMRMLDisplayableNode  // parent is displayable node so that color and visibility can be set for all children nodes
{
public:
  static vtkMRMLPathPlannerTrajectoryNode *New();
  vtkTypeMacro(vtkMRMLPathPlannerTrajectoryNode, vtkMRMLDisplayableNode);

  enum Events
  {
    /// The node stores both inputs (e.g., tooltip position, model, colors, etc.) and computed parameters.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 555 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 555
  };

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "PathPlannerTrajectory";};

  virtual const char* GetIcon() {return ":/Icons/PathPlannerTrajectory.png";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLPathPlannerTrajectoryNode);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  /// Get/Set entry points markups node
  vtkMRMLMarkupsNode* GetEntryPointsNode();
  void SetAndObserveEntryPointsNodeId(const char* markupsNodeId);

  /// Get/Set target points markups node
  vtkMRMLMarkupsNode* GetTargetPointsNode();
  void SetAndObserveTargetPointsNodeId(const char* markupsNodeId);

  /// Add path between entry and target points.
  /// Return path index.
  virtual int AddPath(const std::string& entryPointId, const std::string& targetPointId);

  /// Remove a path.
  virtual void RemovePath(int pathIndex);

  /// Return number of paths.
  virtual int GetNumberOfPaths();

  bool IsPathValid(int pathIndex);

  bool SetNthPathLineNode(int pathIndex, vtkMRMLMarkupsLineNode* pathNode);
  vtkMRMLMarkupsLineNode* GetNthPathLineNode(int pathIndex);

  void SetNthEntryPointIndex(int pathIndex, int pointIndex);
  int GetNthEntryPointIndex(int pathIndex);
  
  void SetNthTargetPointIndex(int pathIndex, int pointIndex);
  int GetNthTargetPointIndex(int pathIndex);

protected:
  vtkMRMLPathPlannerTrajectoryNode();
  ~vtkMRMLPathPlannerTrajectoryNode();
  vtkMRMLPathPlannerTrajectoryNode(const vtkMRMLPathPlannerTrajectoryNode&);
  void operator=(const vtkMRMLPathPlannerTrajectoryNode&);

  void SetPathList(const std::deque<std::string>& pathList);
  std::deque<std::string> GetPathList();

  struct PathType
  {
    std::string EntryPointId;
    std::string TargetPointId;
    std::string PathNodeReferenceRole;
  };

  std::deque<PathType> PathList;
};

#endif
