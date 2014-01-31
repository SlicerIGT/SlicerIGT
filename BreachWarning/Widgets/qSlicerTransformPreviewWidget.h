/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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

#ifndef __qSlicerTransformPreviewWidget_h
#define __qSlicerTransformPreviewWidget_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerBreachWarningModuleWidgetsExport.h"
#include "ui_qSlicerTransformPreviewWidget.h"

#include "vtkSmartPointer.h"
#include "vtkGeneralTransform.h"
#include "vtkCollection.h"

#include "vtkMRMLScene.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"


class qSlicerTransformPreviewWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_BREACHWARNING_WIDGETS_EXPORT 
qSlicerTransformPreviewWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTransformPreviewWidget(QWidget *parent=0);
  virtual ~qSlicerTransformPreviewWidget();

  static qSlicerTransformPreviewWidget* New( vtkMRMLScene* scene );

  vtkMRMLNode* GetCurrentNode();
  void SetCurrentNode( vtkMRMLNode* currentNode );

protected slots:

  void onCheckedNodesChanged();
  void onApplyButtonClicked();
  void onHardenButtonClicked();

  void ObserveAllTransformableNodes();
  void UpdateHiddenNodes();

  void updateWidget();

protected:
  QScopedPointer<qSlicerTransformPreviewWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  void CreateAndAddPreviewNode( vtkMRMLNode* baseNode );
  void ClearPreviewNodes();

  vtkMRMLLinearTransformNode* CurrentTransformNode;
  std::vector< vtkSmartPointer< vtkMRMLTransformableNode > > PreviewNodes;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformPreviewWidget);
  Q_DISABLE_COPY(qSlicerTransformPreviewWidget);

};

#endif
