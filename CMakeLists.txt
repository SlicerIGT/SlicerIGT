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
set(EXTENSION_HOMEPAGE "https://www.assembla.com/spaces/slicerigt")
set(EXTENSION_CATEGORY "IGT")
set(EXTENSION_CONTRIBUTORS "Tamas Ungi (Queen's University), Adam Rankin (Queen's University), Junichi Tokuda (Brighams and Women's Hospital)")
set(EXTENSION_DESCRIPTION "Extensions for image-guided interventions")
set(EXTENSION_ICONURL "http://wiki.slicer.org/slicerWiki/images/2/2f/PerkNavLogo.png")
set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/b/bf/PerkNavScreenshot.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated list or 'NA' if any

#-----------------------------------------------------------------------------
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
option(MODULE_COLLECT_FIDUCIALS_ENABLE "Enable the building of the CollectFiducials module." OFF)
option(MODULE_CREATE_MODELS_ENABLE "Enable the building of the CreateModels module." OFF)
option(MODULE_PERK_EVALUATOR_ENABLE "Enable the building of the PerkEvaluator module." OFF)
option(MODULE_TRANSFORM_RECORDER_ENABLE "Enable the building of the TransformRecorder module." OFF)
option(MODULE_ULTRASOUND_SNAPSHOTS_ENABLE "Enable the building of the UltrasoundSnapshots module." OFF)
option(MODULE_TARGET_FIDUCIALS_ENABLE "Enable the building of the TargetFiducials module." OFF)
option(MODULE_WORKFLOW_SEGMENTATION_ENABLE "Enable the building of the WorkflowSegmentation module." OFF)
option(MODULE_SUPPLEMENTAL_DATA_ENABLE "Download the supplemental data (models, test data, etc..)" OFF)
option(MODULE_DOCUMENTATION_ENABLE "Download the documentation associated with the projec." OFF)

#-----------------------------------------------------------------------------
IF( MODULE_COLLECT_FIDUCIALS_ENABLE )
  SET (MODULE_COLLECT_FIDUCIALS_REPOSITORY "https://github.com/SlicerIGT/CollectFiducials.git" CACHE STRING "URL for the CollectFiducials repository")
	SET (COLLECT_FIDUCIALS_SRC_DIR "${CMAKE_BINARY_DIR}/CollectFiducials")
	SET (COLLECT_FIDUCIALS_DIR "${CMAKE_BINARY_DIR}/CollectFiducials-bin" CACHE INTERNAL "Path to store CollectFiducials binaries")
	ExternalProject_Add( CollectFiducials
    SOURCE_DIR "${COLLECT_FIDUCIALS_SRC_DIR}"
    BINARY_DIR "${COLLECT_FIDUCIALS_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_COLLECT_FIDUCIALS_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_COLLECT_FIDUCIALS_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_CREATE_MODELS_ENABLE )
  SET (MODULE_CREATE_MODELS_REPOSITORY "https://github.com/SlicerIGT/CreateModels.git" CACHE STRING "URL for the CreateModels repository")
	SET (CREATE_MODELS_SRC_DIR "${CMAKE_BINARY_DIR}/CreateModels")
	SET (CREATE_MODELS_DIR "${CMAKE_BINARY_DIR}/CreateModels-bin" CACHE INTERNAL "Path to store CreateModels binaries")
	ExternalProject_Add( CreateModels
    SOURCE_DIR "${CREATE_MODELS_SRC_DIR}"
    BINARY_DIR "${CREATE_MODELS_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_CREATE_MODELS_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_CREATE_MODELS_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_PERK_EVALUATOR_ENABLE )
  SET (MODULE_PERK_EVALUATOR_REPOSITORY "https://github.com/SlicerIGT/PerkEvaluator.git" CACHE STRING "URL for the PerkEvaluator repository")
	SET (PERK_EVALUATOR_SRC_DIR "${CMAKE_BINARY_DIR}/PerkEvaluator")
	SET (PERK_EVALUATOR_DIR "${CMAKE_BINARY_DIR}/PerkEvaluator-bin" CACHE INTERNAL "Path to store PerkEvaluator binaries")
	ExternalProject_Add( PerkEvaluator
    SOURCE_DIR "${PERK_EVALUATOR_SRC_DIR}"
    BINARY_DIR "${PERK_EVALUATOR_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_PERK_EVALUATOR_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_PERK_EVALUATOR_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_TRANSFORM_RECORDER_ENABLE )
  SET (MODULE_TRANSFORM_RECORDER_REPOSITORY "https://github.com/SlicerIGT/TransformRecorder.git" CACHE STRING "URL for the TransformRecorder repository")
	SET (TRANSFORM_RECORDER_SRC_DIR "${CMAKE_BINARY_DIR}/TransformRecorder")
	SET (TRANSFORM_RECORDER_DIR "${CMAKE_BINARY_DIR}/TransformRecorder-bin" CACHE INTERNAL "Path to store TransformRecorder binaries")
	ExternalProject_Add( TransformRecorder
    SOURCE_DIR "${TRANSFORM_RECORDER_SRC_DIR}"
    BINARY_DIR "${TRANSFORM_RECORDER_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_TRANSFORM_RECORDER_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_TRANSFORM_RECORDER_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_ULTRASOUND_SNAPSHOTS_ENABLE )
  SET (MODULE_ULTRASOUND_SNAPSHOTS_REPOSITORY "https://github.com/SlicerIGT/UltrasoundSnapshots.git" CACHE STRING "URL for the UltrasoundSnapshots repository")
	SET (ULTRASOUND_SNAPSHOTS_SRC_DIR "${CMAKE_BINARY_DIR}/UltrasoundSnapshots")
	SET (ULTRASOUND_SNAPSHOTS_DIR "${CMAKE_BINARY_DIR}/UltrasoundSnapshots-bin" CACHE INTERNAL "Path to store UltrasoundSnapshots binaries")
	ExternalProject_Add( UltrasoundSnapshots
    SOURCE_DIR "${ULTRASOUND_SNAPSHOTS_SRC_DIR}"
    BINARY_DIR "${ULTRASOUND_SNAPSHOTS_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_ULTRASOUND_SNAPSHOTS_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_ULTRASOUND_SNAPSHOTS_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_TARGET_FIDUCIALS_ENABLE )
  SET (MODULE_TARGET_FIDUCIALS_REPOSITORY "https://github.com/SlicerIGT/TargetFiducials.git" CACHE STRING "URL for the TargetFiducials repository")
	SET (TARGET_FIDUCIALS_SRC_DIR "${CMAKE_BINARY_DIR}/TargetFiducials")
	SET (TARGET_FIDUCIALS_DIR "${CMAKE_BINARY_DIR}/TargetFiducials-bin" CACHE INTERNAL "Path to store TargetFiducials binaries")
	ExternalProject_Add( TargetFiducials
    SOURCE_DIR "${TARGET_FIDUCIALS_SRC_DIR}"
    BINARY_DIR "${TARGET_FIDUCIALS_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_TARGET_FIDUCIALS_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_TARGET_FIDUCIALS_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_WORKFLOW_SEGMENTATION_ENABLE )
  SET (MODULE_WORKFLOW_SEGMENTATION_REPOSITORY "https://github.com/SlicerIGT/WorkflowSegmentation.git" CACHE STRING "URL for the WorkflowSegmentation repository")
	SET (WORKFLOW_SEGMENTATION_SRC_DIR "${CMAKE_BINARY_DIR}/WorkflowSegmentation")
	SET (WORKFLOW_SEGMENTATION_DIR "${CMAKE_BINARY_DIR}/WorkflowSegmentation-bin" CACHE INTERNAL "Path to store WorkflowSegmentation binaries")
	ExternalProject_Add( WorkflowSegmentation
    SOURCE_DIR "${WORKFLOW_SEGMENTATION_SRC_DIR}"
    BINARY_DIR "${WORKFLOW_SEGMENTATION_DIR}"
    INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "${MODULE_WORKFLOW_SEGMENTATION_REPOSITORY}"
    #--Configure step-------------
    CMAKE_ARGS 
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
        -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
        -DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( MODULE_WORKFLOW_SEGMENTATION_ENABLE )

#-----------------------------------------------------------------------------
IF( MODULE_SUPPLEMENTAL_DATA_ENABLE )
  SET (MODULE_SUPPLEMENTAL_DATA_REPOSITORY "https://github.com/SlicerIGT/SupplementalData.git" CACHE STRING "URL for the SupplementalData repository")
	SET (SUPPLEMENTAL_DATA_SRC_DIR "${CMAKE_BINARY_DIR}/SupplementalData")
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
  SET (MODULE_DOCUMENTATION_REPOSITORY "https://github.com/SlicerIGT/Documentation.git" CACHE STRING "URL for the Documentation repository")
	SET (DOCUMENTATION_DATA_SRC_DIR "${CMAKE_BINARY_DIR}/Documentation")
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
include(${Slicer_EXTENSION_CPACK})
