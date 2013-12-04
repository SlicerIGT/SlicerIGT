/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialRegistrationWizardNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLFiducialRegistrationWizardNode_h
#define __vtkMRMLFiducialRegistrationWizardNode_h

#include <ctime>
#include <iostream>
#include <utility>
#include <vector>

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"

// FiducialRegistrationWizard includes
#include "vtkSlicerFiducialRegistrationWizardModuleMRMLExport.h"


class
VTK_SLICER_FIDUCIALREGISTRATIONWIZARD_MODULE_MRML_EXPORT
vtkMRMLFiducialRegistrationWizardNode
: public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLFiducialRegistrationWizardNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLFiducialRegistrationWizardNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "FiducialRegistrationWizard"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:

  // Constructor/desctructor methods
  vtkMRMLFiducialRegistrationWizardNode();
  virtual ~vtkMRMLFiducialRegistrationWizardNode();
  vtkMRMLFiducialRegistrationWizardNode ( const vtkMRMLFiducialRegistrationWizardNode& );
  void operator=( const vtkMRMLFiducialRegistrationWizardNode& );
 
  
public:
  // Enumerate all the possible modified states
  enum ModifyType
  {
    NeverModify,
    DefaultModify,
    AlwaysModify
  };

  // Use default setters and getters - vtk set macro will cause modified event
  void SetProbeTransformID( std::string newProbeTransformID, int modifyType = DefaultModify );
  void SetFromFiducialListID( std::string newFromFiducialListID, int modifyType = DefaultModify );
  void SetToFiducialListID( std::string newToFiducialListID, int modifyType = DefaultModify );
  void SetActiveFiducialListID( std::string newActiveFiducialListID, int modifyType = DefaultModify );
  void SetOutputTransformID( std::string newOutputTransformID, int modifyType = DefaultModify );
  void SetRegistrationMode( std::string newRegistrationMode, int modifyType = DefaultModify );

  std::string GetProbeTransformID();
  std::string GetFromFiducialListID();
  std::string GetToFiducialListID();
  std::string GetActiveFiducialListID();
  std::string GetOutputTransformID();
  std::string GetRegistrationMode();

  std::string GetNodeReferenceIDString( std::string referenceRole );

  void ObserveAllReferenceNodes();

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  std::string RegistrationMode;

};  

#endif
