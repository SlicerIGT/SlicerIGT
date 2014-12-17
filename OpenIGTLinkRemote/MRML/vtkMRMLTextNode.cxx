#include "vtkMRMLTextNode.h"
#include "vtkMRMLOpenIGTLinkRemoteNode.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>
#include <algorithm> // for std::find

// First reference is the master root node, subsequent references are the synchronized root nodes
static const char* ROOT_NODE_REFERENCE_ROLE_BASE = "rootNodeRef";
static const char* DATA_NODE_REFERENCE_ROLE_BASE = "dataNodeRef";
static const char* DISPLAY_NODES_REFERENCE_ROLE_BASE = "displayNodesRef";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTextNode);

//----------------------------------------------------------------------------
vtkMRMLTextNode::vtkMRMLTextNode()
{
  this->Text = "";
}

//----------------------------------------------------------------------------
vtkMRMLTextNode::~vtkMRMLTextNode()
{
 
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " Text=\"" << this->Text << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Text")) 
    {
      this->SetText(attValue);
    }
  }
}


// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTextNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTextNode* node=vtkMRMLTextNode::SafeDownCast(anode);
  if (node==NULL)
  {
    vtkErrorMacro("Node copy failed: not a vtkMRMLSequenceNode");
    return;
  }
  this->Text=node->Text;
}


void vtkMRMLTextNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "Text: " << this->Text << std::endl;
}


void vtkMRMLTextNode::SetText( std::string newText )
{
  this->Text = newText;
}


void vtkMRMLTextNode::SetText( const char* newText )
{
  this->Text = std::string( newText );
}


std::string vtkMRMLTextNode::GetText()
{
  return this->Text;
}

  
  