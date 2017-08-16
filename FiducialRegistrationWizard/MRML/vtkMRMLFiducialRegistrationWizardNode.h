/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkMRMLFiducialRegistrationWizardNode_h
#define __vtkMRMLFiducialRegistrationWizardNode_h

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

// FiducialRegistrationWizard includes
#include "vtkSlicerFiducialRegistrationWizardModuleMRMLExport.h"

class vtkMRMLMarkupsFiducialNode;
class vtkMRMLTransformNode;

class
VTK_SLICER_FIDUCIALREGISTRATIONWIZARD_MODULE_MRML_EXPORT
vtkMRMLFiducialRegistrationWizardNode
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

  enum
  {
    REGISTRATION_MODE_RIGID = 0,
    REGISTRATION_MODE_SIMILARITY,
    REGISTRATION_MODE_WARPING,
    REGISTRATION_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    UPDATE_MODE_MANUAL = 0,
    UPDATE_MODE_AUTO,
    UPDATE_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    INPUT_FORMAT_ORDERED_PAIRS = 0,
    INPUT_FORMAT_UNORDERED_PAIRS,
    INPUT_FORMAT_LAST // do not set to this type, insert valid types above this line
  };

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

  vtkMRMLFiducialRegistrationWizardNode();
  virtual ~vtkMRMLFiducialRegistrationWizardNode();
  vtkMRMLFiducialRegistrationWizardNode ( const vtkMRMLFiducialRegistrationWizardNode& );
  void operator=( const vtkMRMLFiducialRegistrationWizardNode& );

public:
  vtkMRMLMarkupsFiducialNode* GetFromFiducialListNode();
  void SetAndObserveFromFiducialListNodeId( const char* nodeId );

  vtkMRMLMarkupsFiducialNode* GetToFiducialListNode();
  void SetAndObserveToFiducialListNodeId( const char* nodeId );

  vtkMRMLTransformNode* GetOutputTransformNode();
  void SetOutputTransformNodeId( const char* nodeId );

  vtkMRMLTransformNode* GetProbeTransformFromNode();
  void SetProbeTransformFromNodeId( const char* nodeId );

  vtkMRMLTransformNode* GetProbeTransformToNode();
  void SetProbeTransformToNodeId( const char* nodeId );

  vtkGetMacro( RegistrationMode, int );
  void SetRegistrationMode( int );
  void SetRegistrationModeToRigid() { this->SetRegistrationMode( REGISTRATION_MODE_RIGID ); }
  void SetRegistrationModeToSimilarity() { this->SetRegistrationMode( REGISTRATION_MODE_SIMILARITY ); }
  void SetRegistrationModeToWarping() { this->SetRegistrationMode( REGISTRATION_MODE_WARPING ); }
  static std::string RegistrationModeAsString( int );
  static int RegistrationModeFromString( std::string );

  vtkGetMacro( UpdateMode, int );
  void SetUpdateMode( int newUpdateMode);
  void SetUpdateModeToManual() { this->SetUpdateMode( UPDATE_MODE_MANUAL ); }
  void SetUpdateModeToAuto() { this->SetUpdateMode( UPDATE_MODE_AUTO ); }
  static std::string UpdateModeAsString( int );
  static int UpdateModeFromString( std::string );

  vtkGetMacro( InputFormat, int );
  void SetInputFormat( int newInputFormat );
  void SetInputFormatToOrderedPairs() { this->SetInputFormat( INPUT_FORMAT_ORDERED_PAIRS ); }
  void SetInputFormatToUnorderedPairs() { this->SetInputFormat( INPUT_FORMAT_UNORDERED_PAIRS ); }
  static std::string InputFormatAsString( int );
  static int InputFormatFromString( std::string );

  vtkSetMacro( CalibrationStatusMessage, std::string );
  vtkGetMacro( CalibrationStatusMessage, std::string );

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  int RegistrationMode;
  int UpdateMode;
  int InputFormat;
  std::string CalibrationStatusMessage; // TODO: add this to the ouput transform as a custom node attribute

};  

#endif
