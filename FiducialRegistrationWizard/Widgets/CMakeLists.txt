project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  qSlicerSimpleMarkupsWidget.h
  qSlicerSimpleMarkupsWidget.cxx
  qSlicerTransformPreviewWidget.h
  qSlicerTransformPreviewWidget.cxx
  )

set(${KIT}_MOC_SRCS
  qSlicerSimpleMarkupsWidget.h
  qSlicerTransformPreviewWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerSimpleMarkupsWidget.ui
  Resources/UI/qSlicerTransformPreviewWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/UI/qSlicerSimpleMarkupsWidget.ui
  Resources/UI/qSlicerTransformPreviewWidget.ui
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
