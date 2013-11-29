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

  // Since the setters and getters will only set and get for now, the attributes may as well be public
  std::string ProbeTransformID;
  std::string FromFiducialListID;
  std::string ToFiducialListID;
  std::string ActiveFiducialListID;
  std::string OutputTransformID;
  std::string RegistrationMode;

};  

#endif
