#ifndef __qSlicerOpenIGTLinkRemoteModuleWidget_h
#define __qSlicerOpenIGTLinkRemoteModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerOpenIGTLinkRemoteModuleWidgetsExport.h"

// MRML includes
#include "vtkMRMLAbstractLogic.h"

class qSlicerOpenIGTLinkRemoteModuleWidgetPrivate;
class vtkSlicerOpenIGTLinkIFLogic;
class vtkMRMLNode;


/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_OPENIGTLINKREMOTE_WIDGETS_EXPORT qSlicerOpenIGTLinkRemoteModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  void setMRMLScene(vtkMRMLScene* scene);
  void setIFLogic(vtkSlicerOpenIGTLinkIFLogic* ifLogic);

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerOpenIGTLinkRemoteModuleWidget(QWidget *parent=0);
  virtual ~qSlicerOpenIGTLinkRemoteModuleWidget();

protected slots:

protected:
  QScopedPointer<qSlicerOpenIGTLinkRemoteModuleWidgetPrivate> d_ptr;
  
  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerOpenIGTLinkRemoteModuleWidget);
  Q_DISABLE_COPY(qSlicerOpenIGTLinkRemoteModuleWidget);
  
  QTimer* Timer;
  int LastCommandId;  // Id of the last command sent from this widget.
};

#endif
