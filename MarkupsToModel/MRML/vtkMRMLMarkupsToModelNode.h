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
    Curve,
    ModelType_Last // insert valid types above this line
  };


  enum InterpolationType
  {
    Linear =0,
    CardinalSpline,
    KochanekSpline,
    Polynomial,
    InterpolationType_Last // insert valid types above this line
  };

  enum PointParameterType
  {
    RawIndices = 0,
    MinimumSpanningTree,
    PointParameterType_Last // insert valid types above this line
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

  vtkGetMacro( KochanekTension, double );
  vtkSetMacro( KochanekTension, double );
  vtkGetMacro( KochanekBias, double );
  vtkSetMacro( KochanekBias, double );
  vtkGetMacro( KochanekContinuity, double );
  vtkSetMacro( KochanekContinuity, double );

  vtkGetMacro( PolynomialOrder, int );
  vtkSetMacro( PolynomialOrder, int );

  vtkGetMacro( ModelType, int );
  vtkSetMacro( ModelType, int );
  vtkGetMacro( InterpolationType, int );
  vtkSetMacro( InterpolationType, int );
  vtkGetMacro( PointParameterType, int );
  vtkSetMacro( PointParameterType, int );
  vtkGetMacro( TubeRadius, double );
  vtkSetMacro( TubeRadius, double );
  vtkGetMacro( TubeSegmentsBetweenControlPoints, int );
  vtkSetMacro( TubeSegmentsBetweenControlPoints, int );
  vtkGetMacro( TubeNumberOfSides, int );
  vtkSetMacro( TubeNumberOfSides, int );
  vtkGetMacro( TubeLoop, bool );
  vtkSetMacro( TubeLoop, bool );
  
  vtkGetMacro( AutoUpdateOutput, bool );
  vtkSetMacro( AutoUpdateOutput, bool );
  vtkGetMacro( CleanMarkups, bool );
  vtkSetMacro( CleanMarkups, bool );
  vtkGetMacro( ButterflySubdivision, bool );
  vtkSetMacro( ButterflySubdivision, bool );
  vtkGetMacro( DelaunayAlpha, double );
  vtkSetMacro( DelaunayAlpha, double );
  vtkGetMacro( ConvexHull, bool );
  vtkSetMacro( ConvexHull, bool );

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
  void SetAndObserveModelNodeID( const char* modelId );
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

  vtkMRMLMarkupsFiducialNode * GetMarkupsNode( );
  vtkMRMLModelNode* GetModelNode( );

  // Convert between model and interpolation types IDs and names.
  static const char* GetModelTypeAsString( int id );
  static const char* GetInterpolationTypeAsString( int id );
  static const char* GetPointParameterTypeAsString( int id );
  static int GetModelTypeFromString( const char* name );
  static int GetInterpolationTypeFromString( const char* name );
  static int GetPointParameterTypeFromString( const char* name );

private:
  int    ModelType;
  int    InterpolationType; // Rename to CurveType? Can now be approximating.
  int    PointParameterType;
  bool   AutoUpdateOutput;
  bool   CleanMarkups;
  bool   ButterflySubdivision;
  double DelaunayAlpha;
  bool   ConvexHull;
  double TubeRadius;
  int    TubeSegmentsBetweenControlPoints;
  int    TubeNumberOfSides;
  bool   TubeLoop;
  double KochanekTension;
  double KochanekBias; 
  double KochanekContinuity;
  int    PolynomialOrder;
};

#endif
