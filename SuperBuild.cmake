#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------
include(ExternalProject)
set(ep_base "${CMAKE_BINARY_DIR}")

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#-----------------------------------------------------------------------------
# Build options
#-----------------------------------------------------------------------------

OPTION(SLICERIGT_ENABLE_EXPERIMENTAL_MODULES "Enable the building of work-in-progress, experimental modules." OFF)

#-----------------------------------------------------------------------------
# Project dependencies
#-----------------------------------------------------------------------------

set( inner_DEPENDENCIES "" )
  
set( SlicerIGT_Modules
  CollectFiducials
  CreateModels
  UltrasoundSnapshots
  VolumeResliceDriver
  OpenIGTLinkRemote
  PivotCalibration
  FiducialRegistrationWizard
  )

  
foreach( proj ${SlicerIGT_Modules} )
  set( inner_DEPENDENCIES ${proj}Download ${inner_DEPENDENCIES} )
  set( ${proj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj} )
  set( ${proj}_BINARY_DIR ${${proj}_SOURCE_DIR}-build )
  
  message( STATUS " *** " )
  message( STATUS "Adding new module to superbulid:" )
  message( STATUS ${proj}_SOURCE_DIR" = "${${proj}_SOURCE_DIR} )
  message( STATUS ${proj}_BINARY_DIR" = "${${proj}_BINARY_DIR} )
  
  ExternalProject_Add(
    ${proj}Download
    GIT_REPOSITORY "https://github.com/SlicerIGT/${proj}.git"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND ""
    SOURCE_DIR ${${proj}_SOURCE_DIR}
    BINARY_DIR ${${proj}_BINARY_DIR}
    )
endforeach( proj )


#SlicerMacroCheckExternalProjectDependency(inner)


set(proj inner)
ExternalProject_Add( ${proj}
  DOWNLOAD_COMMAND ""
  INSTALL_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${EXTENSION_BUILD_SUBDIRECTORY}
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
    -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE}
    -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
    -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
    -D${EXTENSION_NAME}_SUPERBUILD:BOOL=OFF
    -DEXTENSION_SUPERBUILD_BINARY_DIR:PATH=${${EXTENSION_NAME}_BINARY_DIR}
    -DSLICERRT_ENABLE_EXPERIMENTAL_MODULES:BOOL=${SLICERRT_ENABLE_EXPERIMENTAL_MODULES} 
    -DCollectFiducials_SOURCE_DIR:PATH=${CollectFiducials_SOURCE_DIR}
    -DCollectFiducials_BINARY_DIR:PATH=${CollectFiducials_BINARY_DIR}
    -DCreateModels_SOURCE_DIR:PATH=${CreateModels_SOURCE_DIR}
    -DCreateModels_BINARY_DIR:PATH=${CreateModels_BINARY_DIR}
    -DOpenIGTLinkRemote_SOURCE_DIR:PATH=${OpenIGTLinkRemote_SOURCE_DIR}
    -DOpenIGTLinkRemote_BINARY_DIR:PATH=${OpenIGTLinkRemote_BINARY_DIR}
    -DUltrasoundSnapshots_SOURCE_DIR:PATH=${UltrasoundSnapshots_SOURCE_DIR}
    -DUltrasoundSnapshots_BINARY_DIR:PATH=${UltrasoundSnapshots_BINARY_DIR}
    -DVolumeResliceDriver_SOURCE_DIR:PATH=${VolumeResliceDriver_SOURCE_DIR}
    -DVolumeResliceDriver_BINARY_DIR:PATH=${VolumeResliceDriver_BINARY_DIR}
    -DPivotCalibration_SOURCE_DIR:PATH=${PivotCalibration_SOURCE_DIR}
    -DPivotCalibration_BINARY_DIR:PATH=${PivotCalibration_BINARY_DIR}
    -DFiducialRegistrationWizard_SOURCE_DIR:PATH=${FiducialRegistrationWizard_SOURCE_DIR}
    -DFiducialRegistrationWizard_BINARY_DIR:PATH=${FiducialRegistrationWizard_BINARY_DIR}
    -DSlicer_DIR:PATH=${Slicer_DIR}
    -DMIDAS_PACKAGE_EMAIL:STRING=${MIDAS_PACKAGE_EMAIL}
    -DMIDAS_PACKAGE_API_KEY:STRING=${MIDAS_PACKAGE_API_KEY}    
    ${ep_cmake_args}
  DEPENDS ${${proj}_DEPENDENCIES}
  )
