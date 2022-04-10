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

#include <iostream>

#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkTimerLog.h"
#include "vtkWeakPointer.h"

// Watchdog includes
#include "vtkSlicerWatchdogModuleMRMLExport.h"

class VTK_SLICER_WATCHDOG_MODULE_MRML_EXPORT vtkMRMLWatchdogNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLWatchdogNode* New();
  vtkTypeMacro(vtkMRMLWatchdogNode, vtkMRMLDisplayableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name
  virtual const char* GetNodeTagName() override { return "Watchdog"; };

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) override;

  /// Create and observe default vtkMRMLWatchdogDisplayNode display node
  virtual void CreateDefaultDisplayNodes() override;

  //--------------------------------------------------------------------------
  /// Watchdog-specific methods
  //--------------------------------------------------------------------------

  /// Returns the number of tools that this node watches
  int GetNumberOfWatchedNodes();

  /// Get the warning message that is displayed if the chosen watched node is outdated
  const char* GetWatchedNodeWarningMessage(int watchedNodeIndex);
  /// Set the warning message that is displayed if the chosen watched node is outdated
  void SetWatchedNodeWarningMessage(int watchedNodeIndex, const char* warningMessage);

  /// Get the maximum allowed elapsed time since the last update of the watched node.
  /// If the node is not updated within this time then the node becomes outdated.
  double GetWatchedNodeUpdateTimeToleranceSec(int watchedNodeIndex);
  /// Set the maximum allowed elapsed time since the last update of the watched node.
  void SetWatchedNodeUpdateTimeToleranceSec(int watchedNodeIndex, double updateTimeToleranceSec);

  /// Returns the status computed in the last call UpdateWatchedNodesStatus method call
  bool GetWatchedNodeUpToDate(int watchedNodeIndex);

  /// Get time elapsed since the last update of the selected watched node
  double GetWatchedNodeElapsedTimeSinceLastUpdateSec(int watchedNodeIndex);

  /// Get true if sound should be played when the watched node becomes outdated
  bool GetWatchedNodePlaySound(int watchedNodeIndex);
  /// Enable/disable playing a warning sound when the watched node becomes outdated
  void SetWatchedNodePlaySound(int watchedNodeIndex, bool playSound);

  /// Add a node to be watched. Returns the watched node's index.
  int AddWatchedNode(vtkMRMLNode *watchedNode, const char* warningMessage=NULL, double updateTimeToleranceSec=-1, bool playSound=false);

  /// Remove the specified watched nodes from the list
  void RemoveWatchedNode(int watchedNodeIndex);

  /// Remove all the watched nodes
  void RemoveAllWatchedNodes();

  /// Get the index of the watched node
  /// Returns -1 if the node is not watched.
  int GetWatchedNodeIndex(vtkMRMLNode* watchedNode);

  /// Get the N-th watched node
  /// Returns NULL if the index is not in a valid range.
  vtkMRMLNode* GetWatchedNode(int watchedNodeIndex);

  /// Get notification about updates of watched nodes
  virtual void ProcessMRMLEvents ( vtkObject * caller, unsigned long event, void * callData ) override;

  /// Updates the up-to-date status of all watched nodes.
  /// If any of the statuses change then a Modified event is invoked.
  /// A watched node's status is valid if the last update of the node happened not longer time than the update time tolerance.
  void UpdateWatchedNodesStatus(bool &watchedNodeBecomeUpToDateSound, bool &watchedNodeBecomeOutdatedSound);

  vtkSetMacro(WatchTransformModifiedEvents, bool);
  vtkGetMacro(WatchTransformModifiedEvents, bool);
  vtkBooleanMacro(WatchTransformModifiedEvents, bool);

protected:

  ///
  /// Called after a node reference ID is removed (list size decreased).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) override;

  // Constructor/destructor methods
  vtkMRMLWatchdogNode();
  virtual ~vtkMRMLWatchdogNode();
  vtkMRMLWatchdogNode ( const vtkMRMLWatchdogNode& );
  void operator=( const vtkMRMLWatchdogNode& );

  bool WatchTransformModifiedEvents{ true };

private:
  class vtkInternal;
  vtkInternal* Internal;
};

#endif
