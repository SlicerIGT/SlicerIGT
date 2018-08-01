/*=auto=========================================================================

Portions (c) Copyright 2018 Robarts Research Institute. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: qMRMLPlusSupportTransformsWidgetPlugin.cxx,v $
Date:      $Date: 2018/6/16 10:54:09 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

#include "qMRMLPlusSupportTransformWidgetPlugin.h"
#include "qSlicerPlusSupportTransformWidget.h"

//------------------------------------------------------------------------------
qMRMLPlusSupportTransformWidgetPlugin::qMRMLPlusSupportTransformWidgetPlugin(QObject* _parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget* qMRMLPlusSupportTransformWidgetPlugin::createWidget(QWidget* _parent)
{
  qSlicerPlusSupportTransformWidget* _widget = new qSlicerPlusSupportTransformWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLPlusSupportTransformWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerPlusSupportTransformWidget\" \
          name=\"PlusSupportTransformWidget\">\n"
         "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLPlusSupportTransformWidgetPlugin::includeFile() const
{
  return "qSlicerPlusSupportTransformWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLPlusSupportTransformWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLPlusSupportTransformWidgetPlugin::name() const
{
  return "qSlicerPlusSupportTransformWidget";
}
