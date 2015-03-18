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

#ifndef __qSlicerOpenIGTLinkRemoteCommandWidget_h
#define __qSlicerOpenIGTLinkRemoteCommandWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerOpenIGTLinkRemoteModuleWidgetsExport.h"

class QTimer;
class qSlicerOpenIGTLinkRemoteCommandWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkSlicerOpenIGTLinkRemoteLogic;
class vtkSlicerOpenIGTLinkIFLogic;


/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_OPENIGTLINKREMOTE_WIDGETS_EXPORT qSlicerOpenIGTLinkRemoteCommandWidget :
  public qSlicerWidget
{
  Q_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerOpenIGTLinkRemoteCommandWidget(QWidget *parent=0);
  virtual ~qSlicerOpenIGTLinkRemoteCommandWidget();

  void setMRMLScene(vtkMRMLScene *scene);
  void setCommandLogic(vtkMRMLAbstractLogic* newCommandLogic);
  void setIFLogic(vtkSlicerOpenIGTLinkIFLogic *logic);


protected slots:

  void OnSendCommandClicked();
  void onQueryResponseReceived();

  virtual void setup();


protected:
  QScopedPointer<qSlicerOpenIGTLinkRemoteCommandWidgetPrivate> d_ptr;
  
private:
  Q_DECLARE_PRIVATE(qSlicerOpenIGTLinkRemoteCommandWidget);
  Q_DISABLE_COPY(qSlicerOpenIGTLinkRemoteCommandWidget);
  
  vtkSlicerOpenIGTLinkRemoteLogic *CommandLogic;
};

#endif
