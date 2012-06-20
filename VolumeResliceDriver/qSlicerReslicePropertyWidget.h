/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2011 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerReslicePropertyWidget_h
#define __qSlicerReslicePropertyWidget_h

// Qt includes
//#include <QGroupBox>
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

#include "qMRMLViewControllerBar.h"

// OpenIGTLinkIF GUI includes
#include "qSlicerRealTimeImagingModuleExport.h"

class qSlicerReslicePropertyWidgetPrivate;
class vtkMRMLScene;
class vtkMRMLNode;
class vtkMRMLSliceNode;
class vtkObject;


/// \ingroup Slicer_QtModules_OpenIGTLinkIF
class Q_SLICER_QTMODULES_REALTIMEIMAGING_EXPORT qSlicerReslicePropertyWidget : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qMRMLViewControllerBar Superclass;
  qSlicerReslicePropertyWidget(QWidget *parent = 0);
  virtual ~qSlicerReslicePropertyWidget();

public slots:
  ///// Set the MRML node of interest
  //void setMRMLIGTLConnectorNode(vtkMRMLIGTLConnectorNode * connectorNode);
  //
  ///// Utility function that calls setMRMLIGTLConnectorNode(vtkMRMLIGTLConnectorNode*)
  ///// It's useful to connect to vtkMRMLNode* signals when you are sure of
  ///// the type
  //void setMRMLIGTLConnectorNode(vtkMRMLNode* node);
  void setSliceViewName(const QString& newSliceViewName);

  void setSliceViewColor(const QColor& newSliceViewColor);

  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);

  /// Return a reference to the current MRML scene
  //vtkMRMLScene * getMRMLScene()const;
  /// Set and observe the MRMLScene
  void setMRMLScene(vtkMRMLScene * newScene);

  void setDriverNode(vtkMRMLNode * newNode);

protected slots:
  /// Internal function to update the widgets based on the IGTLConnector node
  void onMRMLNodeModified();

  //void startCurrentIGTLConnector(bool enabled);

  /// Internal function to update the IGTLConnector node based on the property widget
  //void updateIGTLConnectorNode();


private:
  Q_DECLARE_PRIVATE(qSlicerReslicePropertyWidget);
  Q_DISABLE_COPY(qSlicerReslicePropertyWidget);
};

#endif

