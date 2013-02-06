cmake_minimum_required( VERSION 2.8.7 )

set( EXTENSION_NAME SlicerIGT )

set( EXTENSION_HOMEPAGE "http://slicerigt.github.com/" )
set( EXTENSION_CATEGORY "IGT" )
set( EXTENSION_CONTRIBUTORS "Tamas Ungi, Andras Lasso, Adam Rankin (Queen's University), Junichi Tokuda (Brigham and Women's Hospital)" )
set( EXTENSION_DESCRIPTION "Extensions for image-guided interventions" )
set( EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/2b/SlicerIGTLogo.png" )
set( EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/7/78/SlicerIGTScreenshot.png" )
set( EXTENSION_DEPENDS "NA") # Specified as a space separated list or 'NA' if any
set( EXTENSION_BUILD_SUBDIRECTORY inner-build )

find_package( Slicer REQUIRED )
include( ${Slicer_USE_FILE} )

option( MODULE_CollectFiducials_ENABLE "Enable the building of the CollectFiducials module." ON )

# Run superbuild in the first round of configuration.
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

set( MODULE_NAME ${EXTENSION_NAME} )
set( MODULE_TITLE ${MODULE_NAME} )
string( TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER )

add_subdirectory( ${CollectFiducials_SOURCE_DIR} ${CollectFiducials_BINARY_DIR} )
add_subdirectory( ${CreateModels_SOURCE_DIR} ${CreateModels_BINARY_DIR} )
add_subdirectory( ${UltrasoundSnapshots_SOURCE_DIR} ${UltrasoundSnapshots_BINARY_DIR} )
add_subdirectory( ${VolumeResliceDriver_SOURCE_DIR} ${VolumeResliceDriver_BINARY_DIR} )

set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")

include(${Slicer_EXTENSION_CPACK})
