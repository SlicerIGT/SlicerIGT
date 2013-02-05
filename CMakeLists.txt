#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------
include(ExternalProject)
set(ep_base        "${CMAKE_BINARY_DIR}")

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME SlicerIGT)

#-----------------------------------------------------------------------------
set(EXTENSION_HOMEPAGE "http://slicerigt.github.com/")
set(EXTENSION_CATEGORY "IGT")
set(EXTENSION_CONTRIBUTORS "Tamas Ungi (Queen's University), Junichi Tokuda (Brigham and Women's Hospital), Adam Rankin (Queen's University)")
set(EXTENSION_DESCRIPTION "Extensions for image-guided interventions")
set(EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/2b/SlicerIGTLogo.png")
set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/b/bf/PerkNavScreenshot.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated list or 'NA' if any

#-----------------------------------------------------------------------------
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
option(MODULE_COLLECT_FIDUCIALS_ENABLE "Enable the building of the CollectFiducials module." ON)
option(MODULE_CREATE_MODELS_ENABLE "Enable the building of the CreateModels module." ON)
option(MODULE_PERK_EVALUATOR_ENABLE "Enable the building of the PerkEvaluator module." OFF)
option(MODULE_TRANSFORM_RECORDER_ENABLE "Enable the building of the TransformRecorder module." OFF)
option(MODULE_ULTRASOUND_SNAPSHOTS_ENABLE "Enable the building of the UltrasoundSnapshots module." ON)
option(MODULE_TARGET_FIDUCIALS_ENABLE "Enable the building of the TargetFiducials module." OFF)
option(MODULE_WORKFLOW_SEGMENTATION_ENABLE "Enable the building of the WorkflowSegmentation module." OFF)
option(MODULE_SUPPLEMENTAL_DATA_ENABLE "Download the supplemental data (models, test data, etc..)" OFF)
option(MODULE_DOCUMENTATION_ENABLE "Download the documentation associated with the projec." OFF)
option(MODULE_VOLUME_RESLICE_DRIVER_ENABLE "Enable the building of the VolumeResliceDriver module." ON)

#-----------------------------------------------------------------------------
IF( MODULE_COLLECT_FIDUCIALS_ENABLE )
  set (MODULE_COLLECT_FIDUCIALS_REPOSITORY "https://github.com/SlicerIGT/CollectFiducials.git" CACHE STRING "URL for the CollectFiducials repository")
  set (COLLECT_FIDUCIALS_SRC_DIR "${CMAKE_BINARY_DIR}/CollectFiducials")
  set (COLLECT_FIDUCIALS_DIR "${CMAKE_BINARY_DIR}/CollectFiducials-bin" CACHE INTERNAL "Path to store CollectFiducials binaries")
  ExternalProject_Add( CollectFiducials
    SOURCE_DIR "${COLLECT_FIDUCIALS_SRC_DIR}"
    BINARY_DIR "${COLLECT_FIDUCIALS_DIR}"
    GIT_REPOSITORY "${MODULE_COLLECT_FIDUCIALS_REPOSITORY}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "" 
    )
  add_subdirectory( ${COLLECT_FIDUCIALS_SRC_DIR} ${COLLECT_FIDUCIALS_DIR} )
ENDIF( MODULE_COLLECT_FIDUCIALS_ENABLE )



IF( MODULE_CREATE_MODELS_ENABLE )
  set (MODULE_CREATE_MODELS_REPOSITORY "https://github.com/SlicerIGT/CreateModels.git" CACHE STRING "URL for the CreateModels repository")
  set (CREATE_MODELS_SRC_DIR "${CMAKE_BINARY_DIR}/CreateModels")
  set (CREATE_MODELS_DIR "${CMAKE_BINARY_DIR}/CreateModels-bin" CACHE INTERNAL "Path to store CreateModels binaries")
  ExternalProject_Add( CreateModels
    SOURCE_DIR "${CREATE_MODELS_SRC_DIR}"
    BINARY_DIR "${CREATE_MODELS_DIR}"
    GIT_REPOSITORY "${MODULE_CREATE_MODELS_REPOSITORY}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "" 
    )
  add_subdirectory( ${CREATE_MODELS_SRC_DIR} ${CREATE_MODELS_DIR} )
ENDIF( MODULE_CREATE_MODELS_ENABLE )



IF( MODULE_PERK_EVALUATOR_ENABLE )
  set (MODULE_PERK_EVALUATOR_REPOSITORY "https://github.com/SlicerIGT/PerkEvaluator.git" CACHE STRING "URL for the PerkEvaluator repository")
  set (PERK_EVALUATOR_SRC_DIR "${CMAKE_BINARY_DIR}/PerkEvaluator")
  set (PERK_EVALUATOR_DIR "${CMAKE_BINARY_DIR}/PerkEvaluator-bin" CACHE INTERNAL "Path to store PerkEvaluator binaries")
  ExternalProject_Add( PerkEvaluator
    SOURCE_DIR "${PERK_EVALUATOR_SRC_DIR}"
    BINARY_DIR "${PERK_EVALUATOR_DIR}"
    GIT_REPOSITORY "${MODULE_PERK_EVALUATOR_REPOSITORY}"
    CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
  add_subdirectory( ${PERK_EVALUATOR_SRC_DIR} ${PERK_EVALUATOR_DIR} )
ENDIF( MODULE_PERK_EVALUATOR_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_TRANSFORM_RECORDER_ENABLE )
  set (MODULE_TRANSFORM_RECORDER_REPOSITORY "https://github.com/SlicerIGT/TransformRecorder.git" CACHE STRING "URL for the TransformRecorder repository")
  set (TRANSFORM_RECORDER_SRC_DIR "${CMAKE_BINARY_DIR}/TransformRecorder")
  set (TRANSFORM_RECORDER_DIR "${CMAKE_BINARY_DIR}/TransformRecorder-bin" CACHE INTERNAL "Path to store TransformRecorder binaries")
  ExternalProject_Add( TransformRecorder
    SOURCE_DIR "${TRANSFORM_RECORDER_SRC_DIR}"
    BINARY_DIR "${TRANSFORM_RECORDER_DIR}"
    GIT_REPOSITORY "${MODULE_TRANSFORM_RECORDER_REPOSITORY}"
    CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
  add_subdirectory( ${TRANSFORM_RECORDER_SRC_DIR} ${TRANSFORM_RECORDER_DIR} )
ENDIF( MODULE_TRANSFORM_RECORDER_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_ULTRASOUND_SNAPSHOTS_ENABLE )
  set (MODULE_ULTRASOUND_SNAPSHOTS_REPOSITORY "https://github.com/SlicerIGT/UltrasoundSnapshots.git" CACHE STRING "URL for the UltrasoundSnapshots repository")
  set (ULTRASOUND_SNAPSHOTS_SRC_DIR "${CMAKE_BINARY_DIR}/UltrasoundSnapshots")
  set (ULTRASOUND_SNAPSHOTS_DIR "${CMAKE_BINARY_DIR}/UltrasoundSnapshots-bin" CACHE INTERNAL "Path to store UltrasoundSnapshots binaries")
  ExternalProject_Add( UltrasoundSnapshots
    SOURCE_DIR "${ULTRASOUND_SNAPSHOTS_SRC_DIR}"
    BINARY_DIR "${ULTRASOUND_SNAPSHOTS_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    GIT_REPOSITORY "${MODULE_ULTRASOUND_SNAPSHOTS_REPOSITORY}"
    CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
  add_subdirectory( ${ULTRASOUND_SNAPSHOTS_SRC_DIR} ${ULTRASOUND_SNAPSHOTS_DIR} )
ENDIF( MODULE_ULTRASOUND_SNAPSHOTS_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_TARGET_FIDUCIALS_ENABLE )
  set (MODULE_TARGET_FIDUCIALS_REPOSITORY "https://github.com/SlicerIGT/TargetFiducials.git" CACHE STRING "URL for the TargetFiducials repository")
  set (TARGET_FIDUCIALS_SRC_DIR "${CMAKE_BINARY_DIR}/TargetFiducials")
  set (TARGET_FIDUCIALS_DIR "${CMAKE_BINARY_DIR}/TargetFiducials-bin" CACHE INTERNAL "Path to store TargetFiducials binaries")
  ExternalProject_Add( TargetFiducials
    SOURCE_DIR "${TARGET_FIDUCIALS_SRC_DIR}"
    BINARY_DIR "${TARGET_FIDUCIALS_DIR}"
    GIT_REPOSITORY "${MODULE_TARGET_FIDUCIALS_REPOSITORY}"
    CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
    INSTALL_COMMAND ""
	  )
  add_subdirectory( ${TARGET_FIDUCIALS_SRC_DIR} ${TARGET_FIDUCIALS_DIR} )
ENDIF( MODULE_TARGET_FIDUCIALS_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_WORKFLOW_SEGMENTATION_ENABLE )
  set (MODULE_WORKFLOW_SEGMENTATION_REPOSITORY "https://github.com/SlicerIGT/WorkflowSegmentation.git" CACHE STRING "URL for the WorkflowSegmentation repository")
  set (WORKFLOW_SEGMENTATION_SRC_DIR "${CMAKE_BINARY_DIR}/WorkflowSegmentation")
  set (WORKFLOW_SEGMENTATION_DIR "${CMAKE_BINARY_DIR}/WorkflowSegmentation-bin" CACHE INTERNAL "Path to store WorkflowSegmentation binaries")
  ExternalProject_Add( WorkflowSegmentation
    SOURCE_DIR "${WORKFLOW_SEGMENTATION_SRC_DIR}"
    BINARY_DIR "${WORKFLOW_SEGMENTATION_DIR}"
    GIT_REPOSITORY "${MODULE_WORKFLOW_SEGMENTATION_REPOSITORY}"
    CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
    INSTALL_COMMAND ""
	  )
  add_subdirectory( ${WORKFLOW_SEGMENTATION_SRC_DIR} ${WORKFLOW_SEGMENTATION_DIR} )
ENDIF( MODULE_WORKFLOW_SEGMENTATION_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_SUPPLEMENTAL_DATA_ENABLE )
  set (MODULE_SUPPLEMENTAL_DATA_REPOSITORY "https://github.com/SlicerIGT/SupplementalData.git" CACHE STRING "URL for the SupplementalData repository")
  set (SUPPLEMENTAL_DATA_SRC_DIR "${CMAKE_BINARY_DIR}/SupplementalData")
  ExternalProject_Add( SupplementalData
    SOURCE_DIR "${SUPPLEMENTAL_DATA_SRC_DIR}"
    BINARY_DIR ""
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_SUPPLEMENTAL_DATA_REPOSITORY}"
    #--Configure step-------------
    CONFIGURE_COMMAND ""
    #--Build step-----------------
  BUILD_COMMAND ""
    #--Install step-----------------
    INSTALL_COMMAND ""
    )
ENDIF( MODULE_SUPPLEMENTAL_DATA_ENABLE )

IF( MODULE_DOCUMENTATION_ENABLE )
  set (MODULE_DOCUMENTATION_REPOSITORY "https://github.com/SlicerIGT/Documentation.git" CACHE STRING "URL for the Documentation repository")
  set (DOCUMENTATION_DATA_SRC_DIR "${CMAKE_BINARY_DIR}/Documentation")
  ExternalProject_Add( Documentation
    SOURCE_DIR "${DOCUMENTATION_DATA_SRC_DIR}"
    BINARY_DIR ""
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_DOCUMENTATION_REPOSITORY}"
    #--Configure step-------------
    CONFIGURE_COMMAND ""
    #--Build step-----------------
  BUILD_COMMAND ""
    #--Install step-----------------
    INSTALL_COMMAND ""
    )
ENDIF( MODULE_DOCUMENTATION_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_VOLUME_RESLICE_DRIVER_ENABLE )
  set (MODULE_VOLUME_RESLICE_DRIVER_REPOSITORY "https://github.com/SlicerIGT/VolumeResliceDriver.git" CACHE STRING "URL for the VolumeResliceDriver repository")
  set (VOLUME_RESLICE_DRIVER_SRC_DIR "${CMAKE_BINARY_DIR}/VolumeResliceDriver")
  set (VOLUME_RESLICE_DRIVER_DIR "${CMAKE_BINARY_DIR}/VolumeResliceDriver-bin" CACHE INTERNAL "Path to store VolumeResliceDriver binaries")
  ExternalProject_Add( VolumeResliceDriver
    SOURCE_DIR "${VOLUME_RESLICE_DRIVER_SRC_DIR}"
    BINARY_DIR "${VOLUME_RESLICE_DRIVER_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_VOLUME_RESLICE_DRIVER_REPOSITORY}"
    CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
    INSTALL_COMMAND ""
	)
  add_subdirectory( ${VOLUME_RESLICE_DRIVER_SRC_DIR} ${VOLUME_RESLICE_DRIVER_DIR} )
ENDIF( MODULE_VOLUME_RESLICE_DRIVER_ENABLE )

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})

set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
