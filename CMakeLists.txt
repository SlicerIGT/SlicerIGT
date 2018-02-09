cmake_minimum_required(VERSION 3.5)

project(SlicerIGT)

# Experimental modules are not included in the official SlicerIGT extension package.
# They are kept in the repository to allow testing but not stable enough to be made available to users.
option(SLICERIGT_ENABLE_EXPERIMENTAL_MODULES "Enable building experimental modules" OFF)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.slicerigt.org")
set(EXTENSION_CATEGORY "IGT")
set(EXTENSION_CONTRIBUTORS "Tamas Ungi (Queen's University), Junichi Tokuda (Brigham and Women's Hospital), Andras Lasso (Queen's University), Isaiah Norton (Brigham and Women's Hospital), Matthew Holden (Queen's University), Laurent Chauvin (SNR), Atsushi Yamada (SNR), Franklin King (Queen's University), Jaime Garcia-Guevara (Queen's University), Amelie Meyer (Queen's University), Mikael Brudfors (UCL), Adam Rankin (Robarts Research Institute)")
set(EXTENSION_DESCRIPTION "This extension contains modules that enable rapid prototyping of applications for image-guided interventions. Intended users should have real-time imaging and/or tracking hardware (e.g. tracked ultrasound) connected to 3D Slicer through OpenIGTLink network. Specific modules allow patient registration to the navigation coordinate system in 3D Slicer, and real-time update of tracked models and images." )
set(EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/2b/SlicerIGTLogo.png" )
set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/7/78/SlicerIGTScreenshot.png" )
set(EXTENSION_DEPENDS "SlicerOpenIGTLink")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(BreachWarning)
add_subdirectory(CollectPoints)
add_subdirectory(CreateModels)
add_subdirectory(FiducialRegistrationWizard)
add_subdirectory(FiducialsToModelRegistration)
add_subdirectory(Guidelet)
add_subdirectory(ModelRegistration)
add_subdirectory(OpenIGTLinkRemote)
add_subdirectory(PathExplorer)
add_subdirectory(PivotCalibration)
add_subdirectory(PlusRemote)
add_subdirectory(TextureModel)
add_subdirectory(ToolWatchdog)
add_subdirectory(TransformProcessor)
add_subdirectory(UltrasoundSnapshots)
add_subdirectory(VolumeResliceDriver)
add_subdirectory(PlusModelCatalogBrowser)
add_subdirectory(Viewpoint)
## NEXT_MODULE
if(SLICERIGT_ENABLE_EXPERIMENTAL_MODULES)
  add_subdirectory(Experimental)
endif(SLICERIGT_ENABLE_EXPERIMENTAL_MODULES)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
