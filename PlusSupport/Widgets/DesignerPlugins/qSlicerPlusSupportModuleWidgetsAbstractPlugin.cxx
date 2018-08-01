/*=auto=========================================================================

Portions (c) Copyright 2018 Robarts Research Institute. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: qSlicerPlusSupportModuleWidgetsAbstractPlugin.cxx,v $
Date:      $Date: 2018/6/16 10:54:09 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

#include "qSlicerPlusSupportModuleWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerPlusSupportModuleWidgetsAbstractPlugin::qSlicerPlusSupportModuleWidgetsAbstractPlugin()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPlusSupportModuleWidgetsAbstractPlugin::group() const
{
  return "Slicer [PlusSupport Widgets]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerPlusSupportModuleWidgetsAbstractPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qSlicerPlusSupportModuleWidgetsAbstractPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerPlusSupportModuleWidgetsAbstractPlugin::whatsThis() const
{
  return QString();
}