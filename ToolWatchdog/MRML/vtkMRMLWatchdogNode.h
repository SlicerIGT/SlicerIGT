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

class vtkMRMLDisplayableNode;

struct WatchedTool{
  vtkMRMLDisplayableNode* tool;
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

  // Tool is interpreted as displayable node. It check the time stamp with a frequency determined
  // by the QTimer in the widget and set the status to 1 if the time stamp has changed compared
  // to the last time stamp saved.

  /// Gets the specified tool watched from the tools' list
  WatchedTool* GetToolNode(int currentRow);
  /// Gets the list of tools 
  std::list<WatchedTool>* GetToolNodes();
  /// Adds the displayble node into the tools list, adding label, status, and last time stamp information.
  void AddToolNode( vtkMRMLDisplayableNode *mrmlNode);
  /// Removes the specified tool watched from the tools' list
  void RemoveTool(int row);
  /// Swaps the specified tools watched from the tools' list
  void SwapTools( int toolA, int toolB );
  /// Returns True if the list of tools already contains the tool name
  bool HasTool(char * toolName);
  /// Returns the size of the list of tools
  int GetNumberOfTools();

  //void SetAndObserveToolNodeId( const char* nodeId );
  //void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  std::list< WatchedTool > WatchedTools;
};  

#endif
