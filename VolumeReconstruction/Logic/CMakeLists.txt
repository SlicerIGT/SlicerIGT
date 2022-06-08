project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT "${PROJECT_NAME}")

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")
set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSequencesModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerAnnotationModuleMRML_INCLUDE_DIRS}
  ${VTKVOLUMERECONSTRUCTION_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${VTK_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerMarkupsModuleMRML
  vtkVolumeReconstruction
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )


foreach(p IN LISTS ${KIT}_INCLUDE_DIRECTORIES)
  target_include_directories(${KIT} PUBLIC $<BUILD_INTERFACE:${p}>)
endforeach()
foreach(p IN LISTS ${KIT}_INCLUDE_DIRS)
  target_include_directories(${KIT} PUBLIC $<BUILD_INTERFACE:${p}>)
endforeach()
target_include_directories(${KIT} PUBLIC $<INSTALL_INTERFACE:${${KIT}_INSTALL_INCLUDE_DIR}>)
