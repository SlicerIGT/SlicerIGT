/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLMarkupsToModelNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLMarkupsToModelNode_h
#define __vtkMRMLMarkupsToModelNode_h

//#include <ctime>
#include <iostream>
//#include <utility>
#include <list>

#include "vtkCommand.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"


// MarkupsToModel includes
#include "vtkSlicerMarkupsToModelModuleMRMLExport.h"

static const char* MARKUPS_ROLE = "markupsToModelRoleNode";

class vtkMRMLModelNode;
class vtkMRMLMarkupsFiducialNode;

struct MarkupsTool{
  //vtkMRMLDisplayableNode* tool;
  int status;
  int playSound;
  unsigned long lastTimeStamp;
  unsigned long lastElapsedTimeStamp;
  std::string label;
  std::string id;

  MarkupsTool()
  {
    //tool=NULL;
    status=0;
    lastTimeStamp=0;
    playSound=0;
    label = "label";
    id = "";
    lastElapsedTimeStamp=0;
  }
};

class
VTK_SLICER_MARKUPSTOMODEL_MODULE_MRML_EXPORT
vtkMRMLMarkupsToModelNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    /// The node stores both inputs (e.g., markups, etc.) and computed parameters.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 777 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 777
  };

  enum ModelType
  {
    ClosedSurface =0,
    Curve
  };
  
  vtkTypeMacro( vtkMRMLMarkupsToModelNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLMarkupsToModelNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "MarkupsToModel"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  //vtkGetMacro( ModelNodeName, char* );
  vtkSetMacro( ModelNodeName, char* );

  vtkGetMacro( ModelNode, vtkMRMLModelNode * );
  vtkSetMacro( ModelNode, vtkMRMLModelNode * );

  vtkGetMacro( TubeRadius, double );
  vtkSetMacro( TubeRadius, double );
  vtkGetMacro( ModelType, int );
  vtkSetMacro( ModelType, int );
  vtkGetMacro( AutoUpdateOutput, bool );
  vtkSetMacro( AutoUpdateOutput, bool );
  vtkGetMacro( CleanMarkups, bool );
  vtkSetMacro( CleanMarkups, bool );
  vtkGetMacro( ButterflySubdivision, bool );
  vtkSetMacro( ButterflySubdivision, bool );
  vtkGetMacro( DelaunayAlpha, double );
  vtkSetMacro( DelaunayAlpha, double );

protected:

  // Constructor/destructor methods
  vtkMRMLMarkupsToModelNode();
  virtual ~vtkMRMLMarkupsToModelNode();
  vtkMRMLMarkupsToModelNode ( const vtkMRMLMarkupsToModelNode& );
  void operator=( const vtkMRMLMarkupsToModelNode& );
  
public:

  // Tool is interpreted as displayable node. The tool's time stamp is checked with a frequency determined
  // by the QTimer in the toolBarManagerWidget class. The tool's status is set to 1 if the time stamp has changed compared
  // to the last time stamp saved.

  // 
  void SetAndObserveMarkupsNodeID( const char* markupsId );
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

  /// Gets the specified tool watched from the tools' list
  vtkMRMLMarkupsFiducialNode * GetMarkupsNode();
  /// Gets the list of tools 
  //std::list<MarkupsTool>* GetToolNodes();
  /// Adds the displayble node into the tools list, adding label, status,id, and last time stamp information.
  //void AddMarkupsNode( vtkMRMLMarkupsFiducialNode* markupsAdded);
  /// Removes the specified tool watched from the tools' list
  void RemoveAllMarkups();
  void RemoveLastMarkup();
  /// Swaps the specified tools watched from the tools' list
  void SwapTools( int toolA, int toolB );
  /// Returns True if the list of tools already contains the tool name
  bool HasTool(char * toolName);
  /// Returns the size of the list of tools
  //int GetNumberOfMarkups();
  std::string GetModelNodeName();
  std::string GetDisplayNodeName();


  void SetOutputIntersectionVisibility(bool outputIntersectionVisibility);
  void SetOutputVisibility(bool outputVisibility);
  void SetOutputOpacity(double outputOpacity);
  //vtkGetVector3Macro(WarningColor, double);
  virtual void SetOutputColor(double redComponent, double greenComponent, double blueComponent);
  //virtual void SetWarningColor(double _arg[3]);

  bool GetOutputIntersectionVisibility( );
  bool GetOutputVisibility( );
  double GetOutputOpacity( );
  //vtkGetVector3Macro(WarningColor, double);
  void GetOutputColor( double outputColor[3]  );

private:
  vtkMRMLMarkupsFiducialNode * Markups;
  vtkMRMLModelNode * ModelNode;
  char* MarkupsNodeID;
  char* ModelNodeName;
  int ModelType;
  bool AutoUpdateOutput;
  bool CleanMarkups;
  bool ButterflySubdivision;
  double DelaunayAlpha;
  double TubeRadius;
};

#endif
