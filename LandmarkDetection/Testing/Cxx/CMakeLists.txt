set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkLandmarkDetectionTest.cxx
  )
set(KIT_TEST_NAMES
  vtkLandmarkDetectionTest
  )
set(KIT_TEST_NAMES_CXX
  vtkLandmarkDetectionTest
  )

SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

#-----------------------------------------------------------------------------
#set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  # Add source of your tests after this line.
  #EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

#-----------------------------------------------------------------------------
add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

#-----------------------------------------------------------------------------
set(PATH_STRING "$ENV{PATH}")
STRING(REPLACE "\\;" ";" PATH_STRING "${PATH_STRING}")
STRING(REPLACE ";" "\\;" PATH_STRING "${PATH_STRING}")
foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
  SET_TESTS_PROPERTIES(${testname}
    PROPERTIES ENVIRONMENT "PATH=${PATH_STRING}")
endforeach()
