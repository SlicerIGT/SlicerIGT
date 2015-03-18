
// Watchdog MRML includes
#include "vtkMRMLWatchdogNode.h"
#include "vtkMRMLDisplayableNode.h"

// Other MRML includes
#include "vtkMRMLNode.h"

// Other includes
#include <sstream>

vtkMRMLWatchdogNode* vtkMRMLWatchdogNode::New()
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

vtkMRMLWatchdogNode::vtkMRMLWatchdogNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
}

vtkMRMLWatchdogNode::~vtkMRMLWatchdogNode()
{
}

vtkMRMLNode* vtkMRMLWatchdogNode::CreateNodeInstance()
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

void vtkMRMLWatchdogNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);

  of << indent << " NumberOfWatchedTools=\"" << WatchedTools.size() << "\"";
 int i =1;
  for (std::list<WatchedTool>::iterator it = WatchedTools.begin() ; it != WatchedTools.end(); ++it)
  {
    if((*it).tool== NULL)
    {
      continue;
    }
    of << indent << " WatchedToolName" << i<<"=\""<<(*it).tool->GetName() << "\"";
    of << indent << " WatchedToolSoundActivated" << i<<"=\""<< (*it).playSound << "\"";
    of << indent << " WatchedToolID" << i<<"=\""<< (*it).tool->GetID() << "\"";
    i++;
  }
}

void vtkMRMLWatchdogNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    if ( ! strcmp( attName, "NumberOfWatchedTools" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
      int r = 0;
      nameString >> r;
      //vtkDebugMacro("Number of watched tools read "<< r );
      for (int i =0; i<r; i++)
      {
        WatchedTool tempWatchedTool;
        std::stringstream ss;
        ss<<"WatchedToolName";
        ss << i+1;
        attName  = *(atts++);
        attValue = *(atts++);
        vtkDebugMacro("WatchedToolName read "<< ss.str().c_str() << " atName = "<< attName<< " atValue = "<< attValue);
        if ( ! strcmp( attName, ss.str().c_str() ) )
        {
          tempWatchedTool.label= std::string(attValue).substr(0,6);
        }
        else 
        {
          vtkWarningMacro("WatchedToolName read "<< attName<< " different than expected = " << ss.str().c_str() );
          continue;
        }

        std::stringstream soundString;
        soundString<<"WatchedToolSoundActivated";
        soundString << i+1;
        attName  = *(atts++);
        vtkDebugMacro("WatchedToolSoundActivated read "<< soundString.str().c_str() << " atName = " << attName << " atValue = " << attValue);
        if ( ! strcmp( attName, soundString.str().c_str()) )
        {
          attValue = *(atts++);

          std::stringstream ss;
          ss << attValue;
          ss>>(tempWatchedTool.playSound);
          attName  = *(atts++);
        }
        else 
        {
          vtkWarningMacro("There was not sound activated read "<< attName<< " different than expected = " << soundString.str().c_str() );
        }

        std::stringstream IdString;
        IdString<<"WatchedToolID";
        IdString << i+1;
        attValue = *(atts++);
        vtkDebugMacro("WatchedToolID read "<< IdString.str().c_str() << " atName = " << attName << " atValue = " << attValue);
        if ( ! strcmp( attName, IdString.str().c_str()) )
        {
          tempWatchedTool.id=std::string(attValue);
          WatchedTools.push_back(tempWatchedTool);
        }
        else 
        {
          vtkWarningMacro("WatchedToolID read "<< attName<< " different than expected = " << IdString.str().c_str() );
          continue;
        }
      }
    }
  }
  vtkDebugMacro("XML atts number of tools read "<<GetNumberOfTools());
}

void vtkMRMLWatchdogNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLWatchdogNode *node = ( vtkMRMLWatchdogNode* ) anode;
  this->Modified();
}

void vtkMRMLWatchdogNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "ToolID: ";
  for (int i=0; i<this->GetNumberOfTools();i++)
  {
    os << indent << this->GetToolNode(i)->tool->GetID() << std::endl;
  }
  
}

WatchedTool * vtkMRMLWatchdogNode::GetToolNode(int currentRow)
{
  std::list<WatchedTool>::iterator it = WatchedTools.begin();
  advance (it,currentRow);
  WatchedTool * watchedTool = &(*it);
  return watchedTool;
}

std::list<WatchedTool> * vtkMRMLWatchdogNode::GetToolNodes()
{
  return &WatchedTools;
}

int vtkMRMLWatchdogNode::AddToolNode( vtkMRMLDisplayableNode* toolAdded)
{
  WatchedTool tempWatchedTool;
  if(toolAdded==NULL)
  {
    return 0;
  }
  tempWatchedTool.tool=toolAdded;
  std::string toolAddedName(toolAdded->GetName());
  tempWatchedTool.label= toolAddedName.substr(0,6)+ toolAddedName.substr( toolAddedName.size()-2, toolAddedName.size());
;
  tempWatchedTool.id=toolAdded->GetID();
  //tempWatchedTool.LastTimeStamp=mrmlNode->GetMTime();
  WatchedTools.push_back(tempWatchedTool);

  //vtkDebugMacro("New number of tools "<<GetNumberOfTools());
  return GetNumberOfTools();
}

void vtkMRMLWatchdogNode::RemoveTool(int row)
{
  if(row>=0 && row<WatchedTools.size())
  {
    std::list<WatchedTool>::iterator it = WatchedTools.begin();
    advance (it,row);
    WatchedTools.erase(it);
  }
}


void vtkMRMLWatchdogNode::SwapTools( int toolA, int toolB )
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
  toolTemp.id=itA->id;
  toolTemp.lastElapsedTimeStamp=itA->lastElapsedTimeStamp;
  toolTemp.playSound=itA->playSound;

  itA->status=itB->status;
  itA->tool=itB->tool;
  itA->lastTimeStamp=itB->lastTimeStamp;
  itA->label=itB->label;
  itA->id=itB->id;
  itA->lastElapsedTimeStamp=itB->lastElapsedTimeStamp;
  itA->playSound=itB->playSound;

  itB->status=toolTemp.status;
  itB->tool=toolTemp.tool;
  itB->lastTimeStamp=toolTemp.lastTimeStamp;
  itB->label=toolTemp.label;
  itB->id=toolTemp.id;
  itB->lastElapsedTimeStamp=toolTemp.lastElapsedTimeStamp;
  itB->playSound=toolTemp.playSound;
}

bool vtkMRMLWatchdogNode::HasTool(char * toolName)
{
  for (std::list<WatchedTool>::iterator it = WatchedTools.begin() ; it != WatchedTools.end(); ++it)
  {
    if((*it).tool== NULL)
    {
      continue;
    }
    if(!strcmp((*it).tool->GetName(), toolName))//(toolNameTemp.compare(toolName)==0)
    {
      return true;
    }
  }
  return false;
}

int vtkMRMLWatchdogNode::GetNumberOfTools()
{
  return WatchedTools.size();
}
