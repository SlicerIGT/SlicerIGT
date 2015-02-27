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

#include "vtkCommand.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// BreachWarning includes
#include "vtkSlicerBreachWarningModuleMRMLExport.h"

class vtkMRMLLinearTransformNode;
class vtkMRMLModelNode;


class
VTK_SLICER_BREACHWARNING_MODULE_MRML_EXPORT
vtkMRMLBreachWarningNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    // vtkCommand::UserEvent + 555 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 555
  };
  
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
    
  vtkGetMacro( ToolInsideModel, bool );
  vtkGetMacro( ClosestDistanceToModelFromToolTransform, double );
  vtkSetMacro( ClosestDistanceToModelFromToolTransform, double );

  void SetWarningColor( double r, double g, double b, double a );
  double GetWarningColorComponent( int c );
  
  void SetOriginalColor( double r, double g, double b, double a );
  double GetOriginalColorComponent( int c );

  void SetDisplayWarningColor(int displayWarningColor );
  int GetDisplayWarningColor();

  // Watched model defines the risk area that needs to be avoided.

  vtkMRMLModelNode* GetWatchedModelNode();
  void SetAndObserveWatchedModelNodeID( const char* modelId );
  

  // Tool transform is interpreted as ToolTip-to-RAS. The origin of ToolTip 
  // coordinate system is the tip of the surgical tool that needs to avoid the
  // risk area.

  vtkMRMLLinearTransformNode* GetToolTransformNode();
  void SetAndObserveToolTransformNodeId( const char* nodeId );

  
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:

  double WarningColor[ 4 ];
  double OriginalColor[ 4 ];
  bool ToolInsideModel;
  int DisplayWarningColor;
  // It is the closest distance to the model from the tool transform. If the distance is negative
  // the transform is inside the model.
  double ClosestDistanceToModelFromToolTransform;

};  

#endif
