// MarkupsToModel MRML includes
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLDisplayableNode.h"

// Other MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"

// VTK includes
#include <vtkNew.h>

// Other includes
#include <sstream>

//std::string getToolLabel(char * toolName)
//{
//  //std::string toolAddedName(toolName);
//  //if(toolAddedName.size()>6)
//  //{
//  //  return toolAddedName.substr(0,4)+ toolAddedName.substr( toolAddedName.size()-2, toolAddedName.size());
//  //}
//  //else
//  //{
//  //  return toolAddedName.substr(0,6);
//  //}
//}

vtkMRMLMarkupsToModelNode* vtkMRMLMarkupsToModelNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMarkupsToModelNode" );
  if( ret )
  {
    return ( vtkMRMLMarkupsToModelNode* )ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMarkupsToModelNode;
}

vtkMRMLMarkupsToModelNode::vtkMRMLMarkupsToModelNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLMarkupsNode::PointModifiedEvent );//PointEndInteractionEvent

  this->AddNodeReferenceRole( MARKUPS_ROLE, NULL, events.GetPointer() );
  this->ModelNodeName = "";
  this->ModelNode = NULL;

  this->AutoUpdateOutput=true;
  this->CleanMarkups=true;
  this->ButterflySubdivision=true;
  this->DelaunayAlpha=0.0;
}

vtkMRMLMarkupsToModelNode::~vtkMRMLMarkupsToModelNode()
{
}

vtkMRMLNode* vtkMRMLMarkupsToModelNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMarkupsToModelNode" );
  if( ret )
    {
      return ( vtkMRMLMarkupsToModelNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMarkupsToModelNode;
}

void vtkMRMLMarkupsToModelNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
  vtkIndent indent(nIndent);

  //of << indent << " NumberOfMarkups=\"" << this->Markups->GetNumberOfFiducials() << "\"";
  of << indent << " MarkupsID=\"" << this->MarkupsNodeID << "\"";
  
}

void vtkMRMLMarkupsToModelNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    //if ( ! strcmp( attName, "NumberOfMarkups" ) )
    //{
    //  std::stringstream nameString;
    //  nameString << attValue;
    //  int r = 0;
    //  nameString >> r;
    //}
    if ( ! strcmp( attName, "MarkupsID" ) )
    {
      std::stringstream nameString;
      nameString << attValue;
    }
  }
  //vtkDebugMacro("XML atts number of tools read "<<GetNumberOfTools());
}

void vtkMRMLMarkupsToModelNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  vtkMRMLMarkupsToModelNode *node = ( vtkMRMLMarkupsToModelNode* ) anode;
  this->Modified();
}

void vtkMRMLMarkupsToModelNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent); // This will take care of referenced nodes
  os << indent << "ModelID: ";
  //for (int i=0; i<this->GetNumberOfTools();i++)
  //{
    //os << indent << this->Markups->GetID() << std::endl;
  //}
  
}

vtkMRMLMarkupsFiducialNode * vtkMRMLMarkupsToModelNode::GetMarkupsNode()
{
  //std::list<MarkupsTool>::iterator it = this->Markups.begin();
  //advance (it,currentRow);
  //MarkupsTool * watchedTool = &(*it);
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( MARKUPS_ROLE ) );
  return markupsNode;
  //return this->Markups;
}

//void vtkMRMLMarkupsToModelNode::AddMarkupsNode( vtkMRMLMarkupsFiducialNode* markupsAdded)
//{
//  //MarkupsTool tempMarkupsTool;
//  if(markupsAdded==NULL)
//  {
//    return;
//  }
//
//  this->Markups=markupsAdded;
//  //tempMarkupsTool.tool=markupsAdded;
//  //tempMarkupsTool.label=getToolLabel(markupsAdded->GetName());
//  //tempMarkupsTool.id=markupsAdded->GetID();
//  ////tempMarkupsTool.LastTimeStamp=mrmlNode->GetMTime();
//  //this->Markups.push_back(tempMarkupsTool);
//
//  //return GetNumberOfTools();
//}

void vtkMRMLMarkupsToModelNode::SetAndObserveMarkupsNodeID( const char* markupsId )
{
  // SetAndObserveNodeReferenceID does not handle nicely setting of the same
  // node (it should simply ignore the request, but it adds another observer instead)
  // so check for node equality here.

  const char* currentNodeId=this->GetNodeReferenceID(MARKUPS_ROLE);
  if (markupsId!=NULL && currentNodeId!=NULL)
  {
    if (strcmp(markupsId,currentNodeId)==0)
    {
      // not changed
      return;
    }
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  events->InsertNextValue( vtkMRMLMarkupsNode::PointModifiedEvent ); // PointEndInteractionEvent 
  this->SetAndObserveNodeReferenceID( MARKUPS_ROLE, markupsId, events.GetPointer() );
  this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
}

void vtkMRMLMarkupsToModelNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL ) return;

  if (this->GetMarkupsNode() && this->GetMarkupsNode()==caller)
  {
    this->InvokeCustomModifiedEvent(InputDataModifiedEvent);
  }
}

void vtkMRMLMarkupsToModelNode::RemoveAllMarkups()
{
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( MARKUPS_ROLE ) );
  markupsNode->RemoveAllMarkups();
}

void vtkMRMLMarkupsToModelNode::RemoveLastMarkup()
{
  vtkMRMLMarkupsFiducialNode* markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast( this->GetNodeReference( MARKUPS_ROLE ) );
  if(markupsNode->GetNumberOfFiducials()>0)
  {
    markupsNode->RemoveMarkup(markupsNode->GetNumberOfFiducials()-1);
  }
}

std::string vtkMRMLMarkupsToModelNode::GetModelNodeName()
{
  if(this->ModelNode == NULL)
  {
    return std::string(this->GetID()).append("Model");
  }
}

std::string vtkMRMLMarkupsToModelNode::GetDisplayNodeName()
{
  if(this->ModelNode == NULL)
  {
    return std::string(this->GetID()).append("Display");
  }
}

void vtkMRMLMarkupsToModelNode::SwapTools( int toolA, int toolB )
{

}

bool vtkMRMLMarkupsToModelNode::HasTool(char * toolName)
{
  return false;
}

