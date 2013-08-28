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

#ifndef __qSlicerOpenIGTLinkRemoteModuleQueryWidget_h
#define __qSlicerOpenIGTLinkRemoteModuleQueryWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerOpenIGTLinkRemoteModuleWidgetsExport.h"

class qSlicerOpenIGTLinkRemoteQueryWidgetPrivate;
class QAbstractButton;
class vtkSlicerOpenIGTLinkIFLogic;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_IGTLRemote
class Q_SLICER_QTMODULES_OPENIGTLINKREMOTE_WIDGETS_EXPORT qSlicerOpenIGTLinkRemoteQueryWidget :
  public qSlicerWidget
{
  Q_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerOpenIGTLinkRemoteQueryWidget(QWidget *parent=0);
  virtual ~qSlicerOpenIGTLinkRemoteQueryWidget();

  void setMRMLScene(vtkMRMLScene *scene);
  void setIFLogic(vtkSlicerOpenIGTLinkIFLogic *ifLogic);

public slots:
  void setConnectorNode(vtkMRMLNode* node);
  void queryRemoteList();
  void querySelectedItem();
  void onQueryResponseReceived();
  void onQueryTypeChanged(int id);
  void startTracking();
  void stopTracking();

  void getImage(std::string id);
  void getPointList(std::string id);

protected:
  QScopedPointer<qSlicerOpenIGTLinkRemoteQueryWidgetPrivate> d_ptr;

  void init();

private:
  Q_DECLARE_PRIVATE(qSlicerOpenIGTLinkRemoteQueryWidget);
  Q_DISABLE_COPY(qSlicerOpenIGTLinkRemoteQueryWidget);

};

#endif
