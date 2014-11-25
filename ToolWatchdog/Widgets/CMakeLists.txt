project(qMRML${MODULE_NAME}ModuleWidgets) #project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "QMRML_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

##
## CTK
##
#set(CTK_LIBRARIES CTKVisualizationVTKWidgets)
#if(Slicer_USE_PYTHONQT)
#  list(APPEND CTK_LIBRARIES
#    CTKScriptingPythonWidgets
#    )
#endif()
#if(Slicer_USE_QtTesting)
#  list(APPEND CTK_LIBRARIES
#    CTKQtTesting
#    )
#endif()




set(${KIT}_INCLUDE_DIRECTORIES
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLDisplayableManager_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  #${CMAKE_CURRENT_SOURCE_DIR}
  #${CMAKE_CURRENT_BINARY_DIR}
  )

set(${KIT}_SRCS
  #qSlicer${MODULE_NAME}FooBarWidget.cxx
  #qSlicer${MODULE_NAME}FooBarWidget.h
  qMRML${MODULE_NAME}ToolBar.cxx
  qMRML${MODULE_NAME}ToolBar.h
  )

set(${KIT}_MOC_SRCS
  #qSlicer${MODULE_NAME}FooBarWidget.h
  qMRML${MODULE_NAME}Toolbar.h
  )

#set(${KIT}_UI_SRCS
#  ../Resources/UI/qSlicer${MODULE_NAME}FooBarWidget.ui
#  )
#
#set(${KIT}_RESOURCES
#  ../Resources/UI/qSlicer${MODULE_NAME}FooBarWidget.ui
#  )
MESSAGE("name up " ${${KIT}_EXPORT_DIRECTIVE})

Message("CTK libs" ${CTK_LIBRARIES})
Message("QT libs " ${QT_LIBRARIES})

set(${KIT}_TARGET_LIBRARIES
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  #vtkSlicer${MODULE_NAME}ModuleLogic
  qMRMLWidgets
  MRMLLogic
  MRMLDisplayableManager
  )

#if(${VTK_VERSION_MAJOR} GREATER 5)
#  list(APPEND ${KIT}_TARGET_LIBRARIES
#    vtkRenderingQt
#    )
#endif()


SET (${KIT}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" FORCE)

#-----------------------------------------------------------------------------
#ctkMacroBuildQtDesignerPlugin(
#  NAME ${PROJECT_NAME}
#  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
#  SRCS ${${KIT}_SRCS}
#  MOC_SRCS ${${KIT}_MOC_SRCS}
#  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
#  )


SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  #UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  #RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )
