cmake_minimum_required(VERSION 2.8.9)

project(ExampleGuidelet)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "www.YOURWEBSITE.org")
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_CONTRIBUTORS "YOUR NAME")
set(EXTENSION_DESCRIPTION "Just an example for Guidelet-based modules")
set(EXTENSION_ICONURL "")
set(EXTENSION_SCREENSHOTURLS "")
set(EXTENSION_DEPENDS "SlicerIGT")

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(ExampleGuidelet)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
