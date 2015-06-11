
#-----------------------------------------------------------------------------
set(MODULE_NAME "Watchdog")
set(MODULE_TITLE "Watchdog")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(MRMLDM)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicer${MODULE_NAME}ModuleWidget.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  ${QT_LIBRARIES}
  qMRMLWidgets
  MRMLLogic
  MRMLDisplayableManager
  qSlicerBaseQTCore
  )

set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Resources/WatchedNodeOutdated.wav
  ${SlicerIGT_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/WatchedNodeOutdated.wav
  COPYONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Resources/WatchedNodeUpToDate.wav 
  ${SlicerIGT_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/WatchedNodeUpToDate.wav 
  COPYONLY)

install(
  FILES
    ${SlicerIGT_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/WatchedNodeOutdated.wav
    ${SlicerIGT_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/WatchedNodeUpToDate.wav
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME} COMPONENT Runtime)
