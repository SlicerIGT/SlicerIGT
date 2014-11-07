cmake_minimum_required( VERSION 2.8.7 )

set( EXTENSION_NAME SlicerIGT )

set( EXTENSION_HOMEPAGE "http://slicerigt.github.com/" )
set( EXTENSION_CATEGORY "IGT" )
set( EXTENSION_CONTRIBUTORS "Tamas Ungi (Queen's University), Junichi Tokuda (Brigham and Women's Hospital)" )
set( EXTENSION_DESCRIPTION "This extension contains modules that enable rapid prototyping of applications for image-guided interventions. Intended users should have real-time imaging and/or tracking hardware (e.g. tracked ultrasound) connected to 3D Slicer through OpenIGTLink network. Specific modules allow patient registration to the navigation coordinate system in 3D Slicer, and real-time update of tracked models and images." )
set( EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/2b/SlicerIGTLogo.png" )
set( EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/7/78/SlicerIGTScreenshot.png" )
set( EXTENSION_DEPENDS "NA") # Specified as a space separated list or 'NA' if any
set( EXTENSION_BUILD_SUBDIRECTORY inner-build )

project( SlicerIGT ) 
find_package( Slicer REQUIRED )
include( ${Slicer_USE_FILE} )

# Enable building of experimental modules
option( SLICERIGT_ENABLE_EXPERIMENTAL_MODULES "Enable building of work-in-progress, experimental modules." OFF )

set( SlicerIGT_Modules
  BreachWarning
  CollectFiducials
  CreateModels
  FiducialRegistrationWizard
  OpenIGTLinkRemote
  PathExplorer
  PivotCalibration
  PlusRemote
  TransformFusion
  UltrasoundSnapshots
  VolumeResliceDriver
)

if( SLICERIGT_ENABLE_EXPERIMENTAL_MODULES )
  set( SlicerIGT_Modules
    ${SlicerIGT_Modules}
    ToolWatchdog
    # Put experimental modules names here
  )
endif( SLICERIGT_ENABLE_EXPERIMENTAL_MODULES )

# Run superbuild in the first round of configuration.
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)

# Ensure that the modules' repositories are correctly set
foreach( proj ${SlicerIGT_Modules} )
  set( SLICERIGT_REPOSITORY_URL_${proj} "https://github.com/SlicerIGT/${proj}.git" CACHE STRING "Set the ${proj} repository" )
  set( SLICERIGT_REPOSITORY_TAG_${proj} "" CACHE STRING "Set the ${proj} repository tag" )
endforeach( proj )

if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

set( MODULE_NAME ${EXTENSION_NAME} )
set( MODULE_TITLE ${MODULE_NAME} )
string( TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER )

foreach( proj ${SlicerIGT_Modules} )
  add_subdirectory( ${${proj}_SOURCE_DIR} ${${proj}_BINARY_DIR} )
endforeach( proj )

set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")

include(${Slicer_EXTENSION_CPACK})
