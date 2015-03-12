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

class vtkMRMLTransformNode;
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

  vtkGetMacro( ClosestDistanceToModelFromToolTip, double );
  vtkSetMacro( ClosestDistanceToModelFromToolTip, double );
  bool IsToolTipInsideModel();

  /// Indicates if the warning sound is to be played.
  /// False by default.
  /// \sa SetPlayWarningSound(), GetPlayWarningSound(), PlayWarningSoundOn(), PlayWarningSoundOff()
  vtkGetMacro( PlayWarningSound, bool );
  vtkSetMacro( PlayWarningSound, bool );  
  vtkBooleanMacro( PlayWarningSound, bool );

  /// Indicates if color of the watched model should be changed.
  /// False by default.
  /// \sa SetPlayWarningSound(), GetPlayWarningSound(), PlayWarningSoundOn(), PlayWarningSoundOff()
  vtkGetMacro( DisplayWarningColor, bool );
  vtkSetMacro( DisplayWarningColor, bool );  
  vtkBooleanMacro( DisplayWarningColor, bool );

  vtkSetVector3Macro(WarningColor, double);
  vtkGetVector3Macro(WarningColor, double);

  vtkSetVector3Macro(OriginalColor, double);
  vtkGetVector3Macro(OriginalColor, double);


  // Watched model defines the risk area that needs to be avoided.

  vtkMRMLModelNode* GetWatchedModelNode();
  void SetAndObserveWatchedModelNodeID( const char* modelId );  

  // Tool transform is interpreted as ToolTipToRas. The origin of ToolTip 
  // coordinate system is the tip of the surgical tool that needs to avoid the
  // risk area.
  vtkMRMLTransformNode* GetToolTransformNode();
  void SetAndObserveToolTransformNodeId( const char* nodeId );
  
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:

  double WarningColor[3];
  double OriginalColor[3];
  bool DisplayWarningColor;
  bool PlayWarningSound;
  // It is the closest distance to the model from the tool transform. If the distance is negative
  // the transform is inside the model.
  double ClosestDistanceToModelFromToolTip;

};

#endif
