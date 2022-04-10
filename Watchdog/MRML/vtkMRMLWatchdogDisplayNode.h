/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#ifndef __vtkMRMLWatchdogDisplayNode_h
#define __vtkMRMLWatchdogDisplayNode_h

#include "vtkMRMLDisplayNode.h"

// Watchdog includes
#include "vtkSlicerWatchdogModuleMRMLExport.h" 

/// \brief MRML node to represent display properties for watchdog visualization in the slice and 3D viewers.
///
/// vtkMRMLWatchdogDisplayNode nodes store display properties of watchdogs.
class VTK_SLICER_WATCHDOG_MODULE_MRML_EXPORT vtkMRMLWatchdogDisplayNode : public vtkMRMLDisplayNode
{
 public:
  static vtkMRMLWatchdogDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLWatchdogDisplayNode,vtkMRMLDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent ) override;

  enum Position
    {
    POSITION_TOP_LEFT,
    POSITION_BOTTOM_LEFT,
    POSITION_TOP_RIGHT,
    POSITION_BOTTOM_RIGHT,
    POSITION_LAST // this should be the last type of position
    };

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts ) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent ) override;

  ///
  /// Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node ) override;

  ///
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  virtual const char* GetNodeTagName() override {return "WatchdogDisplayNode";};

  //--------------------------------------------------------------------------
  /// Display options
  //--------------------------------------------------------------------------

  vtkSetMacro(Position, int);
  vtkGetMacro(Position, int);
  /// Convert position index to a string for serialization.
  /// Returns an empty string if the index is unknown.
  static const char* ConvertPositionToString(int positionIndex);
  /// Convert position string to an index that can be set in Position.
  /// Returns -1 if the string is unknown.
  static int ConvertPositionFromString(const char* positionString);

  vtkSetMacro(FontSize, int);
  vtkGetMacro(FontSize, int);

protected:

  int Position;

  int FontSize;

 protected:
  vtkMRMLWatchdogDisplayNode ( );
  ~vtkMRMLWatchdogDisplayNode ( );
  vtkMRMLWatchdogDisplayNode ( const vtkMRMLWatchdogDisplayNode& );
  void operator= ( const vtkMRMLWatchdogDisplayNode& );

};

#endif
