
// Watchdog MRML includes
#include "vtkMRMLWatchdogNode.h"
#include "vtkMRMLDisplayableNode.h"

// Other MRML includes
#include "vtkMRMLNode.h"

#include "qMRMLWatchdogToolBar.h"
#include <QMainWindow>
#include <QMenu>
#include "qSlicerApplication.h"

// Other includes
#include <sstream>
#include <QString>

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
  //vtkDebugMacro("New watchdog node!");
  return new vtkMRMLWatchdogNode;
}

vtkMRMLWatchdogNode
::vtkMRMLWatchdogNode()
{
  //vtkDebugMacro("Initialize watchdog node!");
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  this->WatchdogToolbar=NULL;
}

vtkMRMLWatchdogNode
::~vtkMRMLWatchdogNode()
{
  //vtkDebugMacro("DELETE WATCHDOG NODE : "<< this->GetName());
  //this->RemoveToolbar();
  this->WatchdogToolbar=NULL;
}

vtkMRMLNode*
vtkMRMLWatchdogNode
::CreateNodeInstance()
{
  //vtkDebugMacro("Create watchdog node instance!");
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


void
vtkMRMLWatchdogNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  //this->InitializeToolbar();

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
          tempWatchedTool.label=QString(attValue).left(6).toStdString();
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
          tempWatchedTool.playSound=QString(attValue).toInt();
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
          tempWatchedTool.id=QString(attValue).toStdString();
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

int 
vtkMRMLWatchdogNode
::AddToolNode( vtkMRMLDisplayableNode* toolAdded)
{
  WatchedTool tempWatchedTool;
  if(toolAdded==NULL)
  {
    return 0;
  }
  tempWatchedTool.tool=toolAdded;
  tempWatchedTool.label=QString(toolAdded->GetName()).left(6).toStdString();
  tempWatchedTool.id=toolAdded->GetID();
  //tempWatchedTool.LastTimeStamp=mrmlNode->GetMTime();
  WatchedTools.push_back(tempWatchedTool);

  WatchdogToolbar->ToolNodeAdded(tempWatchedTool.label.c_str());

  vtkDebugMacro("New number of tools "<<GetNumberOfTools());
  return GetNumberOfTools();
}

void 
vtkMRMLWatchdogNode
::RemoveTool(int row)
{
  if(row>=0 && row<WatchedTools.size())
  {
    std::list<WatchedTool>::iterator it = WatchedTools.begin();
    advance (it,row);
    WatchedTools.erase(it);
    WatchdogToolbar->DeleteToolNode(row);
  }
}

void 
vtkMRMLWatchdogNode
::InitializeToolbar()
{
  //vtkDebugMacro("Initilize toolBAR");
  if(this->WatchdogToolbar==NULL)
  {
    //vtkDebugMacro("Initilize toolBAR");
    QMainWindow* window = qSlicerApplication::application()->mainWindow();
    this->WatchdogToolbar = new qMRMLWatchdogToolBar (window);
    window->addToolBar(this->WatchdogToolbar);
    this->WatchdogToolbar->setWindowTitle(QApplication::translate("qSlicerAppMainWindow",this->GetName(), 0, QApplication::UnicodeUTF8));
    foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
    {
      if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
      {
        QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
        //toolBarMenu->defaultAction() would be bbetter to use but Slicer App should set the default action
        toolBarMenu->insertAction(toolBarMenuActions.at(toolBarMenuActions.size()-1),this->WatchdogToolbar->toggleViewAction());
        break;
      }
    }
    this->WatchdogToolbar->SetFirstlabel(this->GetName());
  }
}


void 
vtkMRMLWatchdogNode
::RemoveToolbar()
{
  //vtkWarningMacro("DELETE WATCHDOG NODE : "<< this->GetName());
  if(this->WatchdogToolbar!= NULL)
  {
    QMainWindow* window = qSlicerApplication::application()->mainWindow();
    window->removeToolBar(this->WatchdogToolbar);
    foreach (QMenu* toolBarMenu,window->findChildren<QMenu*>())
    {
      if(toolBarMenu->objectName()==QString("WindowToolBarsMenu"))
      {
        QList<QAction*> toolBarMenuActions= toolBarMenu->actions();
        toolBarMenu->removeAction(this->WatchdogToolbar->toggleViewAction());
        break;
      }
    }
  this->WatchdogToolbar=NULL;
  }
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
  WatchdogToolbar->SwapToolNodes(toolA, toolB );
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
