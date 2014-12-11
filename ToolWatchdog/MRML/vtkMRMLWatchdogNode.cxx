
// Watchdog MRML includes
#include "vtkMRMLWatchdogNode.h"

// Other MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"
//#include "vtkMRMLTransformNode.h"
//#include "vtkMRMLLinearTransformNode.h"
//#include "vtkMRMLDisplayNode.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkIntArray.h>
#include <vtkCommand.h>

// Other includes
#include <sstream>
#include <QString>

//// Constants
//static const char* TOOL_ROLE = "WatchdogTransformNode";

vtkMRMLWatchdogNode* vtkMRMLWatchdogNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWatchdogNode" );
  if( ret )
    {
      return ( vtkMRMLWatchdogNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWatchdogNode;
}

vtkMRMLWatchdogNode
::vtkMRMLWatchdogNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  
  //this->AddNodeReferenceRole( TOOL_ROLE );
}

vtkMRMLWatchdogNode
::~vtkMRMLWatchdogNode()
{
}

vtkMRMLNode*
vtkMRMLWatchdogNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWatchdogNode" );
  if( ret )
    {
      return ( vtkMRMLWatchdogNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWatchdogNode;
}

void
vtkMRMLWatchdogNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);
}

void
vtkMRMLWatchdogNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "WarningColorR" ) )
    {
      std::stringstream ss;
      ss << attValue;
      double r = 0.0;
      ss >> r;
    }
  }
}

void
vtkMRMLWatchdogNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLWatchdogNode *node = ( vtkMRMLWatchdogNode* ) anode;
  this->Modified();
}

void
vtkMRMLWatchdogNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "ToolID: ";
  for (int i=0; i<this->GetNumberOfTools();i++)
  {
    os << indent << this->GetToolNode(i)->tool->GetID() << std::endl;
  }
  
}

WatchedTool *
vtkMRMLWatchdogNode
::GetToolNode(int currentRow)
{
  //vtkMRMLDisplayableNode* ltNode = vtkMRMLDisplayableNode::SafeDownCast( this->GetNodeReference( TOOL_ROLE ) );
  std::list<WatchedTool>::iterator it = WatchedTools.begin();
  advance (it,currentRow);
  WatchedTool * watchedTool = &(*it);
  return watchedTool;
}

std::list<WatchedTool> * 
vtkMRMLWatchdogNode
::GetToolNodes()
{
  return &WatchedTools;
}

void
vtkMRMLWatchdogNode
::AddToolNode( vtkMRMLDisplayableNode* toolAdded)
{
  WatchedTool tempWatchedTool;
  //vtkMRMLDisplayableNode *toolAdded=vtkMRMLDisplayableNode::SafeDownCast(mrmlNode);
  if(toolAdded==NULL)
  {
    return;
  }
  tempWatchedTool.tool=toolAdded;
  tempWatchedTool.label=QString(toolAdded->GetName()).left(6).toStdString();
  //tempWatchedTool.tool=mrmlNode;
  //tempWatchedTool.LastTimeStamp=mrmlNode->GetMTime();
  WatchedTools.push_back(tempWatchedTool);
}

void 
vtkMRMLWatchdogNode
::RemoveTool(int row)
{
  std::list<WatchedTool>::iterator it = WatchedTools.begin();
  advance (it,row);
  WatchedTools.erase(it);

  //int index=0;
  //for (std::list<WatchedTransform>::iterator it = WatchedTransfroms.begin() ; it != WatchedTransfroms.end(); ++it)
  //{
  //  QString transName((*it).transform->GetName());
  //  if(transName.compare(transformName)==0)
  //  {
  //    it = WatchedTransfroms.begin();
  //    WatchedTransfroms.erase(it+index);
  //    break;
  //  }
  //  index++;
  //}
}

void 
vtkMRMLWatchdogNode
::SwapTools( int toolA, int toolB )
{
  std::list<WatchedTool>::iterator itA = WatchedTools.begin();
  advance (itA,toolA);
  std::list<WatchedTool>::iterator itB = WatchedTools.begin();
  advance (itB,toolB);

  WatchedTool toolTemp;
  toolTemp.status=itA->status;
  toolTemp.tool=itA->tool;
  toolTemp.lastTimeStamp=itA->lastTimeStamp;
  toolTemp.label=itA->label;

  itA->status=itB->status;
  itA->tool=itB->tool;
  itA->lastTimeStamp=itB->lastTimeStamp;
  itA->label=itB->label;

  itB->status=toolTemp.status;
  itB->tool=toolTemp.tool;
  itB->lastTimeStamp=toolTemp.lastTimeStamp;
  itB->label=toolTemp.label;
}

bool 
vtkMRMLWatchdogNode
::HasTool(char * toolName)
{
  for (std::list<WatchedTool>::iterator it = WatchedTools.begin() ; it != WatchedTools.end(); ++it)
  {
    if((*it).tool== NULL)
    {
      continue;
    }
    QString toolNameTemp((*it).tool->GetName());
    
    if(toolNameTemp.compare(toolName)==0)
    {
      return true;
    }
  }
  return false;
}

int 
vtkMRMLWatchdogNode
::GetNumberOfTools()
{
  return WatchedTools.size();
}

//void
//vtkMRMLWatchdogNode
//::SetAndObserveToolNodeId( const char* nodeId )
//{
//  vtkNew<vtkIntArray> events;
//  events->InsertNextValue( vtkCommand::ModifiedEvent );
//  events->InsertNextValue( vtkMRMLLinearTransformNode::TransformModifiedEvent );
//  this->SetAndObserveNodeReferenceID( TOOL_ROLE, nodeId, events.GetPointer() );
//}



//void
//vtkMRMLWatchdogNode
//::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
//{
//  //vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
//  //if ( callerNode == NULL ) return;
//
//  //const char* ObservedTransformNodeId = this->GetToolNode()->GetID();
//  //if ( strcmp( ObservedTransformNodeId, callerNode->GetID() ) == 0 )
//  //{
//  //  this->Modified(); // This will tell the logic to update
//  //}
//}
