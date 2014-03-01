/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLBreachWarningNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLBreachWarningNode_h
#define __vtkMRMLBreachWarningNode_h

#include <ctime>
#include <iostream>
#include <utility>
#include <vector>

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// BreachWarning includes
#include "vtkSlicerBreachWarningModuleMRMLExport.h"


class
VTK_SLICER_BREACHWARNING_MODULE_MRML_EXPORT
vtkMRMLBreachWarningNode
: public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLBreachWarningNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLBreachWarningNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "BreachWarning"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:

  // Constructor/desctructor methods
  vtkMRMLBreachWarningNode();
  virtual ~vtkMRMLBreachWarningNode();
  vtkMRMLBreachWarningNode ( const vtkMRMLBreachWarningNode& );
  void operator=( const vtkMRMLBreachWarningNode& );
 
  
public:
  // Enumerate all the possible modified states
  enum ModifyType
  {
    NeverModify,
    DefaultModify,
    AlwaysModify
  };

  // Use default setters and getters - vtk set macro will cause modified event

  // Watched model defines the risk area that needs to be avoided.
private:
  std::string WatchedModelID;
public:
  void SetWatchedModelID( std::string modelID, int modifyType = DefaultModify );
  std::string GetWatchedModelID();

  // Tool transform is interpreted as ToolTip-to-RAS. The origin of ToolTip 
  // coordinate system is the tip of the surgical tool that needs to avoid the
  // risk area.
private:
  std::string ToolTipTransformID;
public:
  void SetToolTipTransformID( std::string newTransformID, int modifyType = DefaultModify );
  std::string GetToolTipTransformID();
  
  // Old sample.
  void SetProbeTransformID( std::string newProbeTransformID, int modifyType = DefaultModify );
  void SetFromFiducialListID( std::string newFromFiducialListID, int modifyType = DefaultModify );
  void SetToFiducialListID( std::string newToFiducialListID, int modifyType = DefaultModify );
  void SetOutputTransformID( std::string newOutputTransformID, int modifyType = DefaultModify );
  void SetRegistrationMode( std::string newRegistrationMode, int modifyType = DefaultModify );

  std::string GetProbeTransformID();
  std::string GetFromFiducialListID();
  std::string GetToFiducialListID();
  std::string GetOutputTransformID();
  std::string GetRegistrationMode();

  std::string GetNodeReferenceIDString( std::string referenceRole );

  void ObserveAllReferenceNodes();

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  

  // Old sample.
  std::string RegistrationMode;

};  

#endif
