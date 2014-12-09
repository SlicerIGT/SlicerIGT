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
#include <list>

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// ToolWatchdog includes
#include "vtkSlicerToolWatchdogModuleMRMLExport.h"

#include <QString>

//class vtkMRMLLinearTransformNode;
class vtkMRMLDisplayableNode;

struct WatchedTool{
  vtkMRMLDisplayableNode* tool;
  //vtkMRMLVolumeNode* volume;
  int status;
  unsigned long lastTimeStamp;
  unsigned long lastElapsedTimeStamp;
  std::string label;

  WatchedTool()
  {
    tool=NULL;
    status=0;
    lastTimeStamp=0;
    label = "label";
    lastElapsedTimeStamp=0;
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

  WatchedTool* GetToolNode(int currentRow);
  std::list<WatchedTool>* GetToolNodes();
  void AddToolNode( vtkMRMLDisplayableNode *mrmlNode);
  void RemoveTool(int row);
  void SwapMarkups( int trasformA, int trasformB );
  void SetAndObserveToolNodeId( const char* nodeId );

  bool HasTool(char * transformName);
  int GetNumberOfTools();
  
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  std::list< WatchedTool > WatchedTools;


};  

#endif
