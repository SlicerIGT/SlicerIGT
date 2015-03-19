/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLWatchdogNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLWatchdogNode_h
#define __vtkMRMLWatchdogNode_h

#include <ctime>
#include <iostream>
#include <utility>
#include <list>

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// Watchdog includes
#include "vtkSlicerWatchdogModuleMRMLExport.h"

class vtkMRMLDisplayableNode;

struct WatchedTool{
  vtkMRMLDisplayableNode* tool;
  int status;
  int playSound;
  unsigned long lastTimeStamp;
  unsigned long lastElapsedTimeStamp;
  std::string label;
  std::string id;

  WatchedTool()
  {
    tool=NULL;
    status=0;
    lastTimeStamp=0;
    playSound=0;
    label = "label";
    id = "";
    lastElapsedTimeStamp=0;
  }
};

class
VTK_SLICER_WATCHDOG_MODULE_MRML_EXPORT
vtkMRMLWatchdogNode
: public vtkMRMLNode
{
public:
  
  vtkTypeMacro( vtkMRMLWatchdogNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLWatchdogNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "Watchdog"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );


protected:

  // Constructor/destructor methods
  vtkMRMLWatchdogNode();
  virtual ~vtkMRMLWatchdogNode();
  vtkMRMLWatchdogNode ( const vtkMRMLWatchdogNode& );
  void operator=( const vtkMRMLWatchdogNode& );
  
public:

  // Tool is interpreted as displayable node. The tool's time stamp is checked with a frequency determined
  // by the QTimer in the toolBarManagerWidget class. The tool's status is set to 1 if the time stamp has changed compared
  // to the last time stamp saved.

  /// Gets the specified tool watched from the tools' list
  WatchedTool* GetToolNode(int currentRow);
  /// Gets the list of tools 
  std::list<WatchedTool>* GetToolNodes();
  /// Adds the displayble node into the tools list, adding label, status,id, and last time stamp information.
  int AddToolNode( vtkMRMLDisplayableNode *mrmlNode);
  /// Removes the specified tool watched from the tools' list
  void RemoveTool(int row);
  /// Swaps the specified tools watched from the tools' list
  void SwapTools( int toolA, int toolB );
  /// Returns True if the list of tools already contains the tool name
  bool HasTool(char * toolName);
  /// Returns the size of the list of tools
  int GetNumberOfTools();

private:
  std::list< WatchedTool > WatchedTools;

};

#endif
