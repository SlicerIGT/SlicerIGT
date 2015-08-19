/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

#ifndef __qSlicerReslicePropertyWidgetPlugin_h
#define __qSlicerReslicePropertyWidgetPlugin_h

#include "qSlicerVolumeResliceDriverModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_VOLUMERESLICEDRIVER_WIDGETS_PLUGINS_EXPORT
qSlicerReslicePropertyWidgetPlugin
  : public QObject, public qSlicerVolumeResliceDriverModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerReslicePropertyWidgetPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;

};

#endif
