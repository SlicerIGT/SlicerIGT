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
class vtkMRMLMarkupsFiducialNode;

class
VTK_SLICER_BREACHWARNING_MODULE_MRML_EXPORT
vtkMRMLBreachWarningNode
: public vtkMRMLNode
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

  /// Computed parameter
  vtkGetMacro( ClosestDistanceToModelFromToolTip, double );
  vtkSetMacro( ClosestDistanceToModelFromToolTip, double );

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

  /// Indicates if the trajectory should be displayed.
  /// True by default.
  /// \sa SetDisplayTrajectory(), GetDisplayTrajectory(), DisplayTrajectoryOn(), DisplayTrajectoryOff()
  vtkGetMacro( DisplayTrajectory, bool );
  virtual void SetDisplayTrajectory(bool _arg);

  vtkGetVector3Macro(TrajectoryColor, double);
  virtual void SetTrajectoryColor(double _arg1, double _arg2, double _arg3);
  virtual void SetTrajectoryColor(double _arg[3]);

  /// Indicates if the cross-hair should be displayed.
  /// True by default.
  /// \sa SetDisplayCrossHair(), GetDisplayCrossHair(), DisplayCrossHairOn(), DisplayCrossHairOff()
  vtkGetMacro( DisplayCrossHair, bool );
  virtual void SetDisplayCrossHair(bool _arg);

  vtkGetVector3Macro(CrossHairColor, double);
  virtual void SetCrossHairColor(double _arg1, double _arg2, double _arg3);
  virtual void SetCrossHairColor(double _arg[3]);

  /// Indicates if the distance (mm) should be displayed.
  /// True by default.
  /// \sa SetDisplayDistance(), GetDisplayDistance(), DisplayDistanceOn(), DisplayDistanceOff()
  vtkGetMacro( DisplayDistance, bool );
  virtual void SetDisplayDistance(bool _arg);

  vtkGetVector3Macro(DistanceColor, double);
  virtual void SetDistanceColor(double _arg1, double _arg2, double _arg3);
  virtual void SetDistanceColor(double _arg[3]);

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
  double TrajectoryColor[3];
  double DistanceColor[3];
  double CrossHairColor[3];

  bool DisplayWarningColor;
  bool DisplayTrajectory;
  bool DisplayCrossHair;
  bool DisplayDistance;
  bool PlayWarningSound;

  // It is the closest distance to the model from the tool transform. If the distance is negative
  // the transform is inside the model.
  double ClosestDistanceToModelFromToolTip;

  vtkMRMLModelNode* TrajectoryModelNode;

  vtkMRMLMarkupsFiducialNode* CrossHairFiducialNode;

  vtkMRMLMarkupsFiducialNode* TipFiducialNode;
};

#endif
