/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialRegistrationWizardNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLOpenIGTLinkRemoteNode_h
#define __vtkMRMLOpenIGTLinkRemoteNode_h

#include "vtkMRMLNode.h"

// OpenIGTLinkRemote includes
#include "vtkSlicerOpenIGTLinkRemoteModuleMRMLExport.h"

class VTK_SLICER_OPENIGTLINKREMOTE_MODULE_MRML_EXPORT vtkMRMLOpenIGTLinkRemoteNode
: public vtkMRMLNode
{
public:
  static vtkMRMLOpenIGTLinkRemoteNode *New();
  vtkTypeMacro( vtkMRMLOpenIGTLinkRemoteNode, vtkMRMLNode );
  void PrintSelf( ostream& os, vtkIndent indent );
  
  // Standard MRML node methods
  virtual vtkMRMLNode* CreateNodeInstance();  
  virtual const char* GetNodeTagName() { return "OpenIGTLinkRemote"; };
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:
  vtkMRMLOpenIGTLinkRemoteNode();
  virtual ~vtkMRMLOpenIGTLinkRemoteNode();
  vtkMRMLOpenIGTLinkRemoteNode ( const vtkMRMLOpenIGTLinkRemoteNode& );
  void operator=( const vtkMRMLOpenIGTLinkRemoteNode& );

private:

};  

#endif
