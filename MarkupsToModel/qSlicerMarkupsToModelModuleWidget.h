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

==============================================================================*/

#ifndef __qSlicerMarkupsToModelModuleWidget_h
#define __qSlicerMarkupsToModelModuleWidget_h

// Slicer MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsToModelNode.h"

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMarkupsToModelModuleExport.h"

class qSlicerMarkupsToModelModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_MARKUPSTOMODEL_EXPORT qSlicerMarkupsToModelModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMarkupsToModelModuleWidget(QWidget *parent=0);
  virtual ~qSlicerMarkupsToModelModuleWidget();

public slots:
  void setMRMLScene( vtkMRMLScene* scene );
  void onSceneImportedEvent();

  void updateFromMRMLNode();
  void updateParametersToMRMLNode();
  void updateModelToMRMLNode();
  void updateMarkupsToMRMLNode();
  void updateFromRenderedNodes();
  void updateToRenderedNodes();

  void blockAllSignals();
  void unblockAllSignals();
  void enableAllWidgets();
  void disableAllWidgets();
  
  void UpdateOutputModel();

protected slots:
  void onUpdateButtonClicked();
  void onUpdateButtonCheckboxToggled(bool);

protected:
  QScopedPointer<qSlicerMarkupsToModelModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();
  virtual void exit();

  // functions for manipulating other MRML nodes
  vtkMRMLModelNode* GetModelNode( );
  vtkMRMLModelDisplayNode* GetModelDisplayNode( );
  vtkMRMLMarkupsFiducialNode* GetMarkupsNode( );
  vtkMRMLMarkupsDisplayNode* GetMarkupsDisplayNode( );

  void SetOutputIntersectionVisibility( bool outputIntersectionVisibility );
  void SetOutputVisibility( bool outputVisibility );
  void SetOutputOpacity( double outputOpacity );
  virtual void SetOutputColor( double redComponent, double greenComponent, double blueComponent );
  void SetMarkupsTextScale( double scale );

  bool GetOutputIntersectionVisibility( );
  bool GetOutputVisibility( );
  double GetOutputOpacity( );
  void GetOutputColor( double outputColor[3] );
  double GetMarkupsTextScale( );


private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsToModelModuleWidget);
  Q_DISABLE_COPY(qSlicerMarkupsToModelModuleWidget);

  int previouslyPersistent;
};

#endif
