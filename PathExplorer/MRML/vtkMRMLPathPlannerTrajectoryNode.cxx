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

#include "vtkMRMLPathPlannerTrajectoryNode.h"

#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include <vtksys/SystemTools.hxx>

#include <vtkObjectFactory.h>

#include <string>

static const char* ENTRY_POINTS_ROLE = "entryPointMarkupsNode";
static const char* TARGET_POINTS_ROLE = "targetPointMarkupsNode";
static const char* PATH_LINE_ROLE_PREFIX = "pathLineNode";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPathPlannerTrajectoryNode);

//----------------------------------------------------------------------------
vtkMRMLPathPlannerTrajectoryNode::vtkMRMLPathPlannerTrajectoryNode()
{
  this->HideFromEditors = false;

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLMarkupsNode::PointModifiedEvent);
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(ENTRY_POINTS_ROLE, NULL, events);
  this->AddNodeReferenceRole(TARGET_POINTS_ROLE, NULL, events);
}

//----------------------------------------------------------------------------
vtkMRMLPathPlannerTrajectoryNode::~vtkMRMLPathPlannerTrajectoryNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStdStringVectorMacro(pathList, PathList, std::deque);
  vtkMRMLWriteXMLEndMacro();
}


//----------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStdStringVectorMacro(pathList, PathList, std::deque);
  vtkMRMLReadXMLEndMacro();
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLPathPlannerTrajectoryNode* node = vtkMRMLPathPlannerTrajectoryNode::SafeDownCast(anode);
  this->PathList = node->PathList;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Paths:\n";
  for (const auto& path : this->PathList)
  {
    os << indent.GetNextIndent() << path.EntryPointId << " -> " << path.TargetPointId << " (" << path.PathNodeReferenceRole << ")\n";
  }
}

//---------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::ProcessMRMLEvents(vtkObject* caller,
  unsigned long event,
  void* callData)
{
  if (event == vtkMRMLMarkupsNode::PointModifiedEvent)
  {
    this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
  }

  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLPathPlannerTrajectoryNode::GetEntryPointsNode()
{
  vtkMRMLMarkupsNode* node = vtkMRMLMarkupsNode::SafeDownCast(this->GetNodeReference(ENTRY_POINTS_ROLE));
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::SetAndObserveEntryPointsNodeId(const char* nodeId)
{
  this->SetAndObserveNodeReferenceID(ENTRY_POINTS_ROLE, nodeId);
  this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLPathPlannerTrajectoryNode::GetTargetPointsNode()
{
  vtkMRMLMarkupsNode* node = vtkMRMLMarkupsNode::SafeDownCast(this->GetNodeReference(TARGET_POINTS_ROLE));
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::SetAndObserveTargetPointsNodeId(const char* nodeId)
{
  this->SetAndObserveNodeReferenceID(TARGET_POINTS_ROLE, nodeId);
  this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
int vtkMRMLPathPlannerTrajectoryNode::AddPath(const std::string& entryPointId, const std::string& targetPointId)
{
  PathType newPath;
  newPath.EntryPointId = entryPointId;
  newPath.TargetPointId = targetPointId;
  this->PathList.push_back(newPath);
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
  return this->PathList.size() - 1;
}

//------------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::RemovePath(int pathIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size())
  {
    vtkErrorMacro("vtkMRMLPathPlannerTrajectoryNode::RemovePath failed: pathIndex " << pathIndex << " is out of range");
    return;
  }
  this->PathList.erase(this->PathList.begin() + pathIndex);
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
int vtkMRMLPathPlannerTrajectoryNode::GetNumberOfPaths()
{
  return this->PathList.size();
}

//------------------------------------------------------------------------------
bool vtkMRMLPathPlannerTrajectoryNode::IsPathValid(int pathIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size())
  {
    return false;
  }
  const PathType& path = this->PathList[pathIndex];
  vtkMRMLMarkupsNode* entryPoints = this->GetEntryPointsNode();
  if (!entryPoints || !entryPoints->GetNthControlPointByID(path.EntryPointId.c_str()))
  {
    return false;
  }
  vtkMRMLMarkupsNode* targetPoints = this->GetTargetPointsNode();
  if (!targetPoints || !targetPoints->GetNthControlPointByID(path.TargetPointId.c_str()))
  {
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
bool vtkMRMLPathPlannerTrajectoryNode::SetNthPathLineNode(int pathIndex, vtkMRMLMarkupsLineNode* pathNode)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size())
  {
    return false;
  }
  if (!pathNode || !pathNode->GetID())
  {
    return false;
  }
  PathType& path = this->PathList[pathIndex];
  // generate unique role name for storing reference to path node
  if (path.PathNodeReferenceRole.empty())
  {
    for (int i = 0; ; i++)
    {
      std::stringstream pathNodeRefSS;
      pathNodeRefSS << "path_" << i;
      bool pathNodeRefIsUnique = true;
      for (const auto& checkedPath : this->PathList)
      {
        if (checkedPath.PathNodeReferenceRole == pathNodeRefSS.str())
        {
          pathNodeRefIsUnique = false;
          break;
        }
      }
      if (pathNodeRefIsUnique)
      {
        path.PathNodeReferenceRole = pathNodeRefSS.str();
        break;
      }
    }
  }
  this->SetNodeReferenceID(path.PathNodeReferenceRole.c_str(), pathNode->GetID());
  return true;
}

//------------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkMRMLPathPlannerTrajectoryNode::GetNthPathLineNode(int pathIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size())
  {
    return nullptr;
  }
  PathType& path = this->PathList[pathIndex];
  if (path.PathNodeReferenceRole.empty())
    {
    return nullptr;
    }
  return vtkMRMLMarkupsLineNode::SafeDownCast(this->GetNodeReference(path.PathNodeReferenceRole.c_str()));
}

//------------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::SetNthEntryPointIndex(int pathIndex, int pointIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size() || !this->GetEntryPointsNode())
  {
    return;
  }
  PathType& path = this->PathList[pathIndex];
  path.EntryPointId = this->GetEntryPointsNode()->GetNthControlPointID(pointIndex);
}

//------------------------------------------------------------------------------
int vtkMRMLPathPlannerTrajectoryNode::GetNthEntryPointIndex(int pathIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size() || !this->GetEntryPointsNode())
  {
    return -1;
  }
  PathType& path = this->PathList[pathIndex];
  return this->GetEntryPointsNode()->GetNthControlPointIndexByID(path.EntryPointId.c_str());
}

//------------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::SetNthTargetPointIndex(int pathIndex, int pointIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size() || !this->GetTargetPointsNode())
  {
    return;
  }
  PathType& path = this->PathList[pathIndex];
  path.TargetPointId = this->GetTargetPointsNode()->GetNthControlPointID(pointIndex);
}

//------------------------------------------------------------------------------
int vtkMRMLPathPlannerTrajectoryNode::GetNthTargetPointIndex(int pathIndex)
{
  if (pathIndex < 0 || pathIndex >= this->PathList.size() || !this->GetTargetPointsNode())
  {
    return -1;
  }
  PathType& path = this->PathList[pathIndex];
  return this->GetTargetPointsNode()->GetNthControlPointIndexByID(path.TargetPointId.c_str());
}

//------------------------------------------------------------------------------
void vtkMRMLPathPlannerTrajectoryNode::SetPathList(const std::deque<std::string>& pathList)
{
  this->PathList.clear();
  for (int i = 0; i < pathList.size() / 3; i++)
  {
    PathType newPath;
    newPath.EntryPointId = pathList[i * 3];
    newPath.TargetPointId = pathList[i * 3 + 1];
    newPath.PathNodeReferenceRole = pathList[i * 3 + 2];
    this->PathList.push_back(newPath);
  }
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLPathPlannerTrajectoryNode::InputDataModifiedEvent);
}

//------------------------------------------------------------------------------
std::deque<std::string> vtkMRMLPathPlannerTrajectoryNode::GetPathList()
{
  std::deque<std::string> pathList;
  for (const auto& path : this->PathList)
  {
    pathList.push_back(path.EntryPointId);
    pathList.push_back(path.TargetPointId);
    pathList.push_back(path.PathNodeReferenceRole);
  }
  return pathList;
}
