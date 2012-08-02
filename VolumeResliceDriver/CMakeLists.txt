cmake_minimum_required(VERSION 2.8.8)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME MyExtension)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.1/Extensions/${EXTENSION_NAME}")
  set(EXTENSION_CATEGORY "IGT")
  set(EXTENSION_CONTRIBUTORS "Junichi Tokuda (BWH)")
  set(EXTENSION_DESCRIPTION "This is a loadable module to reslice volumes by linear transform.")
  set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/LoadableExtensionTemplate/Resources/Icons/LoadableExtensionTemplate.png?revision=19437&view=co")
endif()

# Module name (usually identique to the directory name)
set(qt_module_name "VolumeResliceDriver")
set(qt_module_title "Volume Reslice Driver")

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR) 
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

# Build module sub libraries
add_subdirectory(Logic)

set(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMERESLICEDRIVER_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

# Source files
set(qt_module_SRCS
  qSlicerVolumeResliceDriverModule.cxx
  qSlicerVolumeResliceDriverModule.h
  qSlicerVolumeResliceDriverModuleWidget.cxx
  qSlicerVolumeResliceDriverModuleWidget.h
  qSlicerReslicePropertyWidget.cxx
  qSlicerReslicePropertyWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerVolumeResliceDriverModule.h
  qSlicerVolumeResliceDriverModuleWidget.h
  qSlicerReslicePropertyWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerVolumeResliceDriverModule.ui
  Resources/UI/qSlicerReslicePropertyWidget.ui
  )

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerVolumeResliceDriverModuleLogic
  )

# Resources
set(qt_module_resources
  Resources/qSlicerVolumeResliceDriverModule.qrc
  )

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  TITLE ${qt_module_title}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )


if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif()


#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
