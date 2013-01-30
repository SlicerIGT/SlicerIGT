
#ifndef __vtkIGTLToMRMLRemoteExec_h
#define __vtkIGTLToMRMLRemoteExec_h

#include "vtkObject.h"
#include "vtkMRMLNode.h"
#include "vtkIGTLToMRMLBase.h"
#include "vtkSlicerOpenIGTLinkRemoteModuleMRMLExport.h"

#include "igtlStringMessage.h"



class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_MRML_EXPORT vtkIGTLToMRMLRemoteExec : public vtkIGTLToMRMLBase
{
 public:

  static vtkIGTLToMRMLRemoteExec *New();
  vtkTypeRevisionMacro(vtkIGTLToMRMLRemoteExec,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int          GetConverterType() { return TYPE_MULTI_IGTL_NAMES; };
  virtual const char*  GetIGTLName() { return "STRING"; };
  virtual const char*  GetMRMLName() { return "RemoteExec"; };
  virtual vtkIntArray* GetNodeEvents();
  virtual vtkMRMLNode* CreateNewNode(vtkMRMLScene* scene, const char* name);

  // for TYPE_MULTI_IGTL_NAMES
  int                  GetNumberOfIGTLNames()   { return this->IGTLNames.size(); };
  const char*          GetIGTLName(int index)   { return this->IGTLNames[index].c_str(); };

  //BTX
  virtual int          IGTLToMRML( igtl::MessageBase::Pointer buffer, vtkMRMLNode* node );
  //ETX
  virtual int          MRMLToIGTL( unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg );

 
 protected:
  vtkIGTLToMRMLRemoteExec();
  ~vtkIGTLToMRMLRemoteExec();

 protected:
  //BTX
  igtl::StringMessage::Pointer StringMsg;
  //ETX
  
};


#endif
