#ifndef __vtkMRMLTextNode_h
#define __vtkMRMLTextNode_h

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>

#include <string>

#include "vtkSlicerOpenIGTLinkRemoteModuleMRMLExport.h"

class vtkMRMLSequenceNode;
class vtkMRMLDisplayNode;

class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_MRML_EXPORT vtkMRMLTextNode : public vtkMRMLNode
{
public:
  static vtkMRMLTextNode *New();
  vtkTypeMacro(vtkMRMLTextNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Create instance of a GAD node. 
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes from name/value pairs 
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format. 
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object 
  virtual void Copy(vtkMRMLNode *node);

  /// Get unique node XML tag name (like Volume, Model) 
  virtual const char* GetNodeTagName() {return "Text";};
  
  void SetText( std::string newText );
  void SetText( const char* newText );
  std::string GetText();

  
protected:
  vtkMRMLTextNode();
  ~vtkMRMLTextNode();
  vtkMRMLTextNode(const vtkMRMLTextNode&);
  void operator=(const vtkMRMLTextNode&);

  std::string GenerateVirtualNodePostfix();
  std::string GetVirtualNodePostfixFromRoot(vtkMRMLSequenceNode* rootNode);

protected:
  std::string Text;

};

#endif
