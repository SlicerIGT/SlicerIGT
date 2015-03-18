// OpenIGTLinkRemote MRML includes
#include "vtkMRMLOpenIGTLinkRemoteNode.h"

// VTK includes
#include <vtkObjectFactory.h>

vtkMRMLNodeNewMacro(vtkMRMLOpenIGTLinkRemoteNode);

// ----------------------------------------------------------------------------
vtkMRMLOpenIGTLinkRemoteNode::vtkMRMLOpenIGTLinkRemoteNode()
{
}

// ----------------------------------------------------------------------------
vtkMRMLOpenIGTLinkRemoteNode::~vtkMRMLOpenIGTLinkRemoteNode()
{
}

// ----------------------------------------------------------------------------
void vtkMRMLOpenIGTLinkRemoteNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent); // This will take care of referenced nodes
}

// ----------------------------------------------------------------------------
void vtkMRMLOpenIGTLinkRemoteNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes
}

// ----------------------------------------------------------------------------
void vtkMRMLOpenIGTLinkRemoteNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
}

// ----------------------------------------------------------------------------
void vtkMRMLOpenIGTLinkRemoteNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}
