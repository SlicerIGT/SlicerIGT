cmake_minimum_required(VERSION 2.8.9)

project(PostureAssessmentToolkit)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/PostureAssessmentToolkit")
set(EXTENSION_CATEGORY "Scoliosis")
set(EXTENSION_CONTRIBUTORS "Ben Church (Perk Lab - Queen's University)")
set(EXTENSION_DESCRIPTION "The modules in this extension provide tools for assessing posture from bilaterally symmetric spinal landmarks")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/PostureAssessmentToolkit.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/PostureAssessmentToolkit/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(PreProcessLandmarks)
add_subdirectory(StandardizeLandmarks)
add_subdirectory(AssessLandmarks)
add_subdirectory(UsSurfaceToLandmarks)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
