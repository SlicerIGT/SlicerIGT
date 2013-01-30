project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

# message( WARNING, ${${KIT}_EXPORT_DIRECTIVE} )

set(${KIT}_INCLUDE_DIRECTORIES
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerOpenIGTLinkIFModuleMRML_SOURCE_DIR}
  ${vtkSlicerOpenIGTLinkIFModuleMRML_BINARY_DIR}
  )

set(${KIT}_SRCS
  vtkMRMLRemoteExecNode.cxx
  vtkMRMLRemoteExecNode.h
  vtkIGTLToMRMLRemoteExec.cxx
  vtkIGTLToMRMLRemoteExec.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${OpenIGTLink_LIBRARIES}
  ${MRML_LIBRARIES}
  vtkSlicerOpenIGTLinkIFModuleMRML
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
