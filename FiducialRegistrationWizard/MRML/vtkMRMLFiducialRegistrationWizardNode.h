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
    UPDATE_MODE_AUTOMATIC,
    UPDATE_MODE_LAST // do not set to this type, insert valid types above this line
  };

  enum
  {
    POINT_MATCHING_MANUAL = 0,
    POINT_MATCHING_AUTOMATIC,
    POINT_MATCHING_LAST // do not set to this type, insert valid types above this line
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
  // Transform 'from' these points
  vtkMRMLMarkupsFiducialNode* GetFromFiducialListNode();
  void SetAndObserveFromFiducialListNodeId( const char* nodeId );

  // Transform 'to' these points
  vtkMRMLMarkupsFiducialNode* GetToFiducialListNode();
  void SetAndObserveToFiducialListNodeId( const char* nodeId );

  // Where to store the output registration
  vtkMRMLTransformNode* GetOutputTransformNode();
  void SetOutputTransformNodeId( const char* nodeId );

  // Transform to record 'From' Points
  vtkMRMLTransformNode* GetProbeTransformFromNode();
  void SetProbeTransformFromNodeId( const char* nodeId );

  // Transform to record 'To' Points
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
  void SetUpdateModeToAuto() { this->SetUpdateMode( UPDATE_MODE_AUTOMATIC ); }
  static std::string UpdateModeAsString( int );
  static int UpdateModeFromString( std::string );

  vtkGetMacro( PointMatching, int );
  void SetPointMatching( int );
  void SetPointMatchingToInputOrder() { this->SetPointMatching( POINT_MATCHING_MANUAL ); }
  void SetPointMatchingToComputed() { this->SetPointMatching( POINT_MATCHING_AUTOMATIC ); }
  static std::string PointMatchingAsString( int );
  static int PointMatchingFromString( std::string );

  vtkSetMacro( CalibrationStatusMessage, std::string );
  vtkGetMacro( CalibrationStatusMessage, std::string );
  void AddToCalibrationStatusMessage( std::string text );
  void ClearCalibrationStatusMessage();

  vtkSetMacro( CalibrationError, double );
  vtkGetMacro( CalibrationError, double );

  /// Get/Set forward transform direction for warping transform.
  /// Forward transforms are applied much faster than inverse transforms.
  /// If WarpingTransformFromParent is set to true (this is the default) then images are transformed fast.
  /// If WarpingTransformFromParent is set to false then models and markups are transformed fast.
  /// \sa WarpingTransformFromParent, SetWarpingTransformFromParent(), WarpingTransformFromParentOn(), WarpingTransformFromParentOff()
  void SetWarpingTransformFromParent(bool warpingTransformFromParent);
  vtkGetMacro(WarpingTransformFromParent, bool);
  vtkBooleanMacro(WarpingTransformFromParent, bool);

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

private:
  // Three modes:
  // - Rigid (translation, orientation)
  // - Similarity (translation, orientation, scale)
  // - Warping (non-uniform, cannot be stored in a vtkMRMLLinearTransformNode)
  int RegistrationMode;

  // There are two modes:
  // - Manual will only update the registration when the UpdateCalibration()
  //   method in the logic is called
  // - Automatic will update automatically whenever one of the observed lists
  //   is modified.
  int UpdateMode;

  // Point matching can be either manual or automatic.
  // - Manual assumes that there are two equally-sized lists of points,
  //   And that they are organized (ordered) in pairs.
  // - Automatic makes neither of the above assumptions. It does assume
  //   that there are at least three points in both lists, that there
  //   is no symmetry among said points, and that a rigid registration
  //   is being performed (similarity and warping are not yet supported).
  int PointMatching;

  /// If true then transformation speedi is optimized for images, otherwise
  /// transformation speed is optimized for models and markups.
  bool WarpingTransformFromParent;

  // The Calibration status message reports the RMS error,
  // as well as any warnings about how the registration
  // was set up.
  // TODO: add these to the output transform as custom node attributes
  std::string CalibrationStatusMessage;
  double CalibrationError;

};

#endif
