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
option(ENABLE_COLLECT_FIDUCIALS "Enable the building of the CollectFiducials module." OFF)
option(ENABLE_CREATE_MODELS "Enable the building of the CreateModels module." OFF)
option(ENABLE_PERK_EVALUATOR "Enable the building of the PerkEvaluator module." OFF)
option(ENABLE_TRANSFORM_RECORDER "Enable the building of the TransformRecorder module." OFF)
option(ENABLE_ULTRASOUND_SNAPSHOTS "Enable the building of the UltrasoundSnapshots module." OFF)
option(ENABLE_TARGET_FIDUCIALS "Enable the building of the TargetFiducials module." OFF)
option(ENABLE_WORKFLOW_SEGMENTATION "Enable the building of the WorkflowSegmentation module." OFF)
option(ENABLE_SUPPLEMENTAL_DATA "Download the supplemental data (models, test data, etc..)" OFF)
option(ENABLE_DOCUMENTATION "Download the documentation associated with the projec." OFF)

#-----------------------------------------------------------------------------
IF( ENABLE_COLLECT_FIDUCIALS )
	SET (COLLECT_FIDUCIALS_SRC_DIR "${CMAKE_BINARY_DIR}/CollectFiducials")
	SET (COLLECT_FIDUCIALS_DIR "${CMAKE_BINARY_DIR}/CollectFiducials-bin" CACHE INTERNAL "Path to store CollectFiducials binaries")
	ExternalProject_Add( CollectFiducials
    SOURCE_DIR "${COLLECT_FIDUCIALS_SRC_DIR}"
    BINARY_DIR "${COLLECT_FIDUCIALS_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/CollectFiducials.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_COLLECT_FIDUCIALS )

#-----------------------------------------------------------------------------
IF( ENABLE_CREATE_MODELS )
	SET (CREATE_MODELS_SRC_DIR "${CMAKE_BINARY_DIR}/CreateModels")
	SET (CREATE_MODELS_DIR "${CMAKE_BINARY_DIR}/CreateModels-bin" CACHE INTERNAL "Path to store CreateModels binaries")
	ExternalProject_Add( CreateModels
    SOURCE_DIR "${CREATE_MODELS_SRC_DIR}"
    BINARY_DIR "${CREATE_MODELS_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/CreateModels.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_CREATE_MODELS )

#-----------------------------------------------------------------------------
IF( ENABLE_PERK_EVALUATOR )
	SET (PERK_EVALUATOR_SRC_DIR "${CMAKE_BINARY_DIR}/PerkEvaluator")
	SET (PERK_EVALUATOR_DIR "${CMAKE_BINARY_DIR}/PerkEvaluator-bin" CACHE INTERNAL "Path to store PerkEvaluator binaries")
	ExternalProject_Add( PerkEvaluator
    SOURCE_DIR "${PERK_EVALUATOR_SRC_DIR}"
    BINARY_DIR "${PERK_EVALUATOR_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/PerkEvaluator.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_PERK_EVALUATOR )

#-----------------------------------------------------------------------------
IF( ENABLE_TRANSFORM_RECORDER )
	SET (TRANSFORM_RECORDER_SRC_DIR "${CMAKE_BINARY_DIR}/TransformRecorder")
	SET (TRANSFORM_RECORDER_DIR "${CMAKE_BINARY_DIR}/TransformRecorder-bin" CACHE INTERNAL "Path to store TransformRecorder binaries")
	ExternalProject_Add( TransformRecorder
    SOURCE_DIR "${TRANSFORM_RECORDER_SRC_DIR}"
    BINARY_DIR "${TRANSFORM_RECORDER_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/TransformRecorder.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_TRANSFORM_RECORDER )

#-----------------------------------------------------------------------------
IF( ENABLE_ULTRASOUND_SNAPSHOTS )
	SET (ULTRASOUND_SNAPSHOTS_SRC_DIR "${CMAKE_BINARY_DIR}/UltrasoundSnapshots")
	SET (ULTRASOUND_SNAPSHOTS_DIR "${CMAKE_BINARY_DIR}/UltrasoundSnapshots-bin" CACHE INTERNAL "Path to store UltrasoundSnapshots binaries")
	ExternalProject_Add( UltrasoundSnapshots
    SOURCE_DIR "${ULTRASOUND_SNAPSHOTS_SRC_DIR}"
    BINARY_DIR "${ULTRASOUND_SNAPSHOTS_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/UltrasoundSnapshots.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_ULTRASOUND_SNAPSHOTS )

#-----------------------------------------------------------------------------
IF( ENABLE_TARGET_FIDUCIALS )
	SET (TARGET_FIDUCIALS_SRC_DIR "${CMAKE_BINARY_DIR}/TargetFiducials")
	SET (TARGET_FIDUCIALS_DIR "${CMAKE_BINARY_DIR}/TargetFiducials-bin" CACHE INTERNAL "Path to store TargetFiducials binaries")
	ExternalProject_Add( TargetFiducials
    SOURCE_DIR "${TARGET_FIDUCIALS_SRC_DIR}"
    BINARY_DIR "${TARGET_FIDUCIALS_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/TargetFiducials.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_TARGET_FIDUCIALS )

#-----------------------------------------------------------------------------
IF( ENABLE_WORKFLOW_SEGMENTATION )
	SET (WORKFLOW_SEGMENTATION_SRC_DIR "${CMAKE_BINARY_DIR}/WorkflowSegmentation")
	SET (WORKFLOW_SEGMENTATION_DIR "${CMAKE_BINARY_DIR}/WorkflowSegmentation-bin" CACHE INTERNAL "Path to store WorkflowSegmentation binaries")
	ExternalProject_Add( WorkflowSegmentation
    SOURCE_DIR "${WORKFLOW_SEGMENTATION_SRC_DIR}"
    BINARY_DIR "${WORKFLOW_SEGMENTATION_DIR}"
	INSTALL_DIR "${CMAKE_BINARY_DIR}"
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/WorkflowSegmentation.git"
    #--Configure step-------------
    CMAKE_ARGS 
        -DLIBRARY_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DEXECUTABLE_OUTPUT_PATH:STRING=${CMAKE_BINARY_DIR}
        -DBUILD_SHARED_LIBS:BOOL=${CMAKE_BINARY_DIR} 
        -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
        -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
		-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_BINARY_DIR}
		-DSlicer_DIR:STRING=${Slicer_DIR}
    #--Build step-----------------
    #--Install step-----------------
    )
ENDIF( ENABLE_WORKFLOW_SEGMENTATION )

#-----------------------------------------------------------------------------
IF( ENABLE_SUPPLEMENTAL_DATA )
	SET (SUPPLEMENTAL_DATA_SRC_DIR "${CMAKE_BINARY_DIR}/SupplementalData")
	ExternalProject_Add( SupplementalData
    SOURCE_DIR "${SUPPLEMENTAL_DATA_SRC_DIR}"
    BINARY_DIR ""
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/SupplementalData.git"
    #--Configure step-------------
	CONFIGURE_COMMAND ""
    #--Build step-----------------
	BUILD_COMMAND ""
    #--Install step-----------------
    INSTALL_COMMAND ""
    )
ENDIF( ENABLE_SUPPLEMENTAL_DATA )

IF( ENABLE_DOCUMENTATION )
	SET (DOCUMENTATION_DATA_SRC_DIR "${CMAKE_BINARY_DIR}/Documentation")
	ExternalProject_Add( Documentation
    SOURCE_DIR "${DOCUMENTATION_DATA_SRC_DIR}"
    BINARY_DIR ""
    #--Download step--------------
    GIT_REPOSITORY "https://github.com/SlicerIGT/Documentation.git"
    #--Configure step-------------
	CONFIGURE_COMMAND ""
    #--Build step-----------------
	BUILD_COMMAND ""
    #--Install step-----------------
    INSTALL_COMMAND ""
    )
ENDIF( ENABLE_DOCUMENTATION )

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
