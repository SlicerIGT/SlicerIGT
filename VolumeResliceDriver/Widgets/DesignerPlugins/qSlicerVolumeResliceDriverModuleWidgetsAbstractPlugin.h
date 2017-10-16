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

==============================================================================*/\

#ifndef __qSlicerVolumeResliceDriverModuleWidgetsAbstractPlugin_h
#define __qSlicerVolumeResliceDriverModuleWidgetsAbstractPlugin_h

#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QDesignerCustomWidgetInterface>
#else
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#endif

#include "qSlicerVolumeResliceDriverModuleWidgetsPluginsExport.h"

class Q_SLICER_MODULE_VOLUMERESLICEDRIVER_WIDGETS_PLUGINS_EXPORT qSlicerVolumeResliceDriverModuleWidgetsAbstractPlugin
    : public QDesignerCustomWidgetInterface
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
#endif
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qSlicerVolumeResliceDriverModuleWidgetsAbstractPlugin();
  // Don't re implement this method.
  QString group() const;
  // You can re implement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
