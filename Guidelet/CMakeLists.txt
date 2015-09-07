#-----------------------------------------------------------------------------
set(MODULE_NAME Guidelet)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  GuideletLoadable.py
  ${MODULE_NAME}Lib/__init__.py
  ${MODULE_NAME}Lib/UltraSound.py
  )

slicerFunctionAddPythonQtResources(MODULE_PYTHON_RESOURCES
  ${MODULE_NAME}Lib/${MODULE_NAME}.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS} ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  WITH_SUBDIR
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)

  # Register the unittest subclass in the main script as a ctest.
  # Note that the test will also be available at runtime.
  slicer_add_python_unittest(SCRIPT ${MODULE_NAME}.py)

  # Additional build-time testing
  add_subdirectory(Testing)
endif()
