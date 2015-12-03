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

class vtkMRMLAnnotationRulerNode;
class vtkMRMLTransformNode;
class vtkMRMLModelNode;


class VTK_SLICER_BREACHWARNING_MODULE_MRML_EXPORT vtkMRMLBreachWarningNode: public vtkMRMLNode
{
public:

  enum Events
  {
    /// The node stores both inputs (e.g., tooltip position, model, colors, etc.) and computed parameters.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
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

  /// Distance of the closest point on the model to the tooltip. Computed parameter.
  vtkGetMacro( ClosestDistanceToModelFromToolTip, double );
  vtkSetMacro( ClosestDistanceToModelFromToolTip, double );

  /// Position of the closest point on the model to the tooltip in RAS coordinate system. Computed parameter.
  vtkGetVector3Macro( ClosestPointOnModel, double );
  vtkSetVector3Macro( ClosestPointOnModel, double );

  /// Computed parameter
  bool IsToolTipInsideModel();

  /// Indicates if the warning sound is to be played.
  /// False by default.
  /// \sa SetPlayWarningSound(), GetPlayWarningSound(), PlayWarningSoundOn(), PlayWarningSoundOff()
  vtkGetMacro( PlayWarningSound, bool );
  virtual void SetPlayWarningSound(bool _arg);

  /// Indicates if color of the watched model should be changed.
  /// True by default.
  /// \sa SetDisplayWarningColor(), GetDisplayWarningColor(), DisplayWarningColorOn(), DisplayWarningColorOff()
  vtkGetMacro( DisplayWarningColor, bool );
  virtual void SetDisplayWarningColor(bool _arg);

  vtkGetVector3Macro(WarningColor, double);
  virtual void SetWarningColor(double _arg1, double _arg2, double _arg3);
  virtual void SetWarningColor(double _arg[3]);

  vtkGetVector3Macro(OriginalColor, double);
  virtual void SetOriginalColor(double _arg1, double _arg2, double _arg3);
  virtual void SetOriginalColor(double _arg[3]);

  /// Watched model defines the area that may breached.
  vtkMRMLModelNode* GetWatchedModelNode();
  void SetAndObserveWatchedModelNodeID( const char* modelId );

  // Tool transform is interpreted as ToolTipToRas. The origin of ToolTip 
  // coordinate system is the tip of the surgical tool that needs to avoid the
  // risk area.
  vtkMRMLTransformNode* GetToolTransformNode();
  void SetAndObserveToolTransformNodeId( const char* nodeId );

  /// Node that displays the line from the tooltip to the closest point on the watched model
  vtkMRMLAnnotationRulerNode* GetLineToClosestPointNode();
  const char* GetLineToClosestPointNodeID();
  void SetLineToClosestPointNodeID( const char* lineToClosestPointNodeId );
  
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:

  double WarningColor[3];
  double OriginalColor[3];
  bool DisplayWarningColor;
  bool PlayWarningSound;
  // It is the closest distance to the model from the tool transform. If the distance is negative
  // the transform is inside the model.
  double ClosestDistanceToModelFromToolTip;
  double ClosestPointOnModel[3];

};
#endif
