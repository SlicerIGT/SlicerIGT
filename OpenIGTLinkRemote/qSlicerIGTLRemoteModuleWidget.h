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

#ifndef __qSlicerIGTLRemoteModuleWidget_h
#define __qSlicerIGTLRemoteModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerIGTLRemoteModuleExport.h"

class qSlicerIGTLRemoteModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_IGTLRemote
class Q_SLICER_QTMODULES_IGTLREMOTE_EXPORT qSlicerIGTLRemoteModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerIGTLRemoteModuleWidget(QWidget *parent=0);
  virtual ~qSlicerIGTLRemoteModuleWidget();

public slots:
  virtual void setMRMLScene(vtkMRMLScene *newScene);
  void setConnectorNode(vtkMRMLNode* node);
  void queryRemoteList();
  void querySelectedItem();
  void onQueryResponseReceived();

protected:
  QScopedPointer<qSlicerIGTLRemoteModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerIGTLRemoteModuleWidget);
  Q_DISABLE_COPY(qSlicerIGTLRemoteModuleWidget);
};

#endif
