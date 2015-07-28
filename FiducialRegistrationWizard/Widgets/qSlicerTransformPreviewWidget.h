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

#ifndef __qSlicerTransformPreviewWidget_h
#define __qSlicerTransformPreviewWidget_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerFiducialRegistrationWizardModuleWidgetsExport.h"
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
class Q_SLICER_MODULE_FIDUCIALREGISTRATIONWIZARD_WIDGETS_EXPORT 
qSlicerTransformPreviewWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTransformPreviewWidget(QWidget *parent=0);
  virtual ~qSlicerTransformPreviewWidget();

  vtkMRMLNode* currentNode();
  void setCurrentNode( vtkMRMLNode* currentNode );

  void setMRMLScene(vtkMRMLScene* scene);

protected slots:

  void onCheckedNodesChanged();
  void onApplyButtonClicked();
  void onHardenButtonClicked();

//  void updateTransformableNodesList();
//  void updateHiddenNodes();

  void updateWidget();

protected:
  QScopedPointer<qSlicerTransformPreviewWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  void createAndAddPreviewNode( vtkMRMLNode* baseNode );
  void clearPreviewNodes();

private:
  Q_DECLARE_PRIVATE(qSlicerTransformPreviewWidget);
  Q_DISABLE_COPY(qSlicerTransformPreviewWidget);

};

#endif
