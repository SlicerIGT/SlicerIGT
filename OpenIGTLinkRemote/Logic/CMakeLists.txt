project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

if(SlicerOpenIGTLink_FOUND)
  set(${KIT}_INCLUDE_DIRECTORIES ${vtkSlicerOpenIGTLinkIFModuleLogic_INCLUDE_DIRS})
else()
  find_package(OpenIGTLink REQUIRED)
  set(${KIT}_TARGET_LIBRARIES OpenIGTLink)
  set(${KIT}_INCLUDE_DIRECTORIES "")
endif()

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_SRCS
  vtkSlicerOpenIGTLinkCommand.cxx
  vtkSlicerOpenIGTLinkCommand.h
  vtkSlicerOpenIGTLinkRemoteLogic.cxx
  vtkSlicerOpenIGTLinkRemoteLogic.h
  )

LIST(APPEND ${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerOpenIGTLinkIFModuleLogic
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  ${DISABLE_WRAP_PYTHON}
  )
