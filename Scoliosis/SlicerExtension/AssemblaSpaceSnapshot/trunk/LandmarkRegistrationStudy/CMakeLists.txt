cmake_minimum_required(VERSION 2.8.9)

project(LandmarkRegistrationStudy)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/LandmarkRegistrationStudy")
set(EXTENSION_CATEGORY "Landmark Registration")
set(EXTENSION_CONTRIBUTORS "Ben Church (Queen's University, PerkLab)")
set(EXTENSION_DESCRIPTION "This extension provides several tools for interacting with skeletal landmarks used for scoliosis assessment with ultrasound")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/LandmarkRegistrationStudy.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/LandmarkRegistrationStudy/Screenshots/1.png")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(ModelToPatientRegistration)
add_subdirectory(PreProcessLandmarks)
add_subdirectory(StandardizeLandmarks)
add_subdirectory(AssessLandmarks)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
