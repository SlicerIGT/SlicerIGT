
// Watchdog MRML includes
#include "vtkMRMLWatchdogNode.h"

// Other MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"

#include "qMRMLWatchdogToolBar.h"
#include "QMainWindow.h"
#include <QMenu>
#include "qSlicerApplication.h"
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
  //vtkWarningMacro("New watchdog node!");
  return new vtkMRMLWatchdogNode;
}

vtkMRMLWatchdogNode
::vtkMRMLWatchdogNode()
{
  vtkWarningMacro("Initialize watchdog node!");
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  this->WatchdogToolbar=NULL;



  
  //this->AddNodeReferenceRole( TOOL_ROLE );
}

vtkMRMLWatchdogNode
::~vtkMRMLWatchdogNode()
{
  this->WatchdogToolbar=NULL;
}

vtkMRMLNode*
vtkMRMLWatchdogNode
::CreateNodeInstance()
{
  vtkWarningMacro("Create watchdog node instance!");


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
    of << indent << " WatchedToolID" << i<<"=\""<< (*it).tool->GetID() << "\"";
    i++;
  }
}


void
vtkMRMLWatchdogNode
::SetName( const char * name )
{
  Superclass::SetName( name );
  vtkWarningMacro("Set Name");
  //vtkMRMLWatchdogNode* node =( vtkMRMLWatchdogNode* )ret;
  if(this->WatchdogToolbar==NULL)
  {
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
        //This might be connected in widget
        //connect(watchdogToolbar, SIGNAL(visibilityChanged(bool)), this, SLOT( onToolbarVisibilityChanged(bool)) );
        break;
      }
    }
  }

  this->WatchdogToolbar->SetFirstlabel(this->GetName());
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
    
    if ( ! strcmp( attName, "NumberOfWatchedTools" ) )
    {
      std::stringstream ssV;
      ssV << attValue;
      int r = 0;
      ssV >> r;
      vtkWarningMacro("Number of watched tools read "<< r );
      //vtkCollection* toolsAdded=this->GetScene()->GetNodesByName(attValue /*ss.str().c_str()*/);
      for (int i =0; i<r; i++)
      {
        WatchedTool tempWatchedTool;
        std::stringstream ss;
        ss<<"WatchedToolName";
        ss << i+1;
        attName  = *(atts++);
        attValue = *(atts++);
        vtkWarningMacro("WatchedToolName read "<< ss.str().c_str() << " atName = "<< attName<< " atValue = "<< attValue);
        if ( ! strcmp( attName, ss.str().c_str() ) )
        {
          tempWatchedTool.label=QString(attValue).toStdString();
          vtkWarningMacro("WatchedToolLabel value"<< tempWatchedTool.label.c_str() );
        }
        else 
        {
          vtkWarningMacro("WatchedToolName read "<< attName<< " different than expected = " << ss.str().c_str() );
          continue;
        }
        std::stringstream ssA;
        ssA<<"WatchedToolID";
        ssA << i+1;
        attName  = *(atts++);
        attValue = *(atts++);
        vtkWarningMacro("WatchedToolID read "<< ssA.str().c_str() << " atName = " << attName << " atValue = " << attValue);
        if ( ! strcmp( attName, ssA.str().c_str()) )
        {
          //std::stringstream ss;
          //ss << attValue;
          //this->GetScene()->GetNodeByID(attValue /*ss.str().c_str()*/);
          tempWatchedTool.id=QString(attValue).toStdString();
          vtkWarningMacro("WatchedToolID value"<< tempWatchedTool.id.c_str() );
          //tempWatchedTool.tool=mrmlNode;
          //tempWatchedTool.LastTimeStamp=mrmlNode->GetMTime();
          this->WatchdogToolbar->ToolNodeAdded(tempWatchedTool.label.c_str());
          WatchedTools.push_back(tempWatchedTool);
        }
        else 
        {
          vtkWarningMacro("WatchedToolID read "<< attName<< " different than expected = " << ssA.str().c_str() );
          continue;
        }
      }
    }
  }
  vtkWarningMacro("XML atts number of tools read "<<GetNumberOfTools());


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
  tempWatchedTool.id=toolAdded->GetID();
  //tempWatchedTool.tool=mrmlNode;
  //tempWatchedTool.LastTimeStamp=mrmlNode->GetMTime();
  WatchedTools.push_back(tempWatchedTool);

  vtkWarningMacro("number of tools "<<GetNumberOfTools());
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
  toolTemp.id=itA->id;

  itA->status=itB->status;
  itA->tool=itB->tool;
  itA->lastTimeStamp=itB->lastTimeStamp;
  itA->label=itB->label;
  itA->id=itB->id;

  itB->status=toolTemp.status;
  itB->tool=toolTemp.tool;
  itB->lastTimeStamp=toolTemp.lastTimeStamp;
  itB->label=toolTemp.label;
  itB->id=toolTemp.id;
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
