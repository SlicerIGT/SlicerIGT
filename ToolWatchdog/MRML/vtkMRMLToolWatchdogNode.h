/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLToolWatchdogNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLToolWatchdogNode_h
#define __vtkMRMLToolWatchdogNode_h

#include <ctime>
#include <iostream>
#include <utility>
#include <vector>

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// ToolWatchdog includes
#include "vtkSlicerToolWatchdogModuleMRMLExport.h"

#include <QString>

class vtkMRMLLinearTransformNode;

struct WatchedTransform{
  vtkMRMLLinearTransformNode* transform;
  int status;
  unsigned long LastTimeStamp;

  WatchedTransform()
  {
    transform=NULL;
    status=0;
    LastTimeStamp=0;
  }

};

class
VTK_SLICER_TOOLWATCHDOG_MODULE_MRML_EXPORT
vtkMRMLToolWatchdogNode
: public vtkMRMLNode
{
public:
  
  vtkTypeMacro( vtkMRMLToolWatchdogNode, vtkMRMLNode );
  
  // Standard MRML node methods  

  static vtkMRMLToolWatchdogNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "ToolWatchdog"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );


  
protected:

  // Constructor/destructor methods

  vtkMRMLToolWatchdogNode();
  virtual ~vtkMRMLToolWatchdogNode();
  vtkMRMLToolWatchdogNode ( const vtkMRMLToolWatchdogNode& );
  void operator=( const vtkMRMLToolWatchdogNode& );
  
public:

  // Tool transform is interpreted as ToolTip-to-RAS. The origin of ToolTip 
  // coordinate system is the tip of the surgical tool that needs to avoid the
  // risk area.

  vtkMRMLLinearTransformNode* GetTransformNode();
  std::vector<WatchedTransform>* GetTransformNodes();
  void addTransformNode( vtkMRMLNode *mrmlNode);
  void RemoveTransform(QString transformName);
  void SetAndObserveToolTransformNodeId( const char* nodeId );
  
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  std::vector< WatchedTransform > WatchedTransfroms;


};  

#endif
