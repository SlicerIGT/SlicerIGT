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

#-----------------------------------------------------------------------------
# Project dependencies
#-----------------------------------------------------------------------------

set( inner_DEPENDENCIES "" )

  
foreach( proj ${SlicerIGT_Modules} )
  set( inner_DEPENDENCIES ${proj}Download ${inner_DEPENDENCIES} )
  set( ${proj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj} )
  set( ${proj}_BINARY_DIR ${${proj}_SOURCE_DIR}-build )
  
  set( MODULE_COMMANDS
    ${MODULE_COMMANDS}
    -D${proj}_SOURCE_DIR:PATH=${${proj}_SOURCE_DIR}
    -D${proj}_BINARY_DIR:PATH=${${proj}_BINARY_DIR}
  )    
  
  ExternalProject_Add(
    ${proj}Download
    GIT_REPOSITORY ${SLICERIGT_REPOSITORY_URL_${proj}}
    GIT_TAG ${SLICERIGT_REPOSITORY_TAG_${proj}}
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
    ${MODULE_COMMANDS}
    -DSLICERIGT_ENABLE_EXPERIMENTAL_MODULES:BOOL=${SLICERIGT_ENABLE_EXPERIMENTAL_MODULES} 
    -DSlicer_DIR:PATH=${Slicer_DIR}
    -DMIDAS_PACKAGE_EMAIL:STRING=${MIDAS_PACKAGE_EMAIL}
    -DMIDAS_PACKAGE_API_KEY:STRING=${MIDAS_PACKAGE_API_KEY}    
    ${ep_cmake_args}
  DEPENDS ${${proj}_DEPENDENCIES}
  )
