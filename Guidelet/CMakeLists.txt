#-----------------------------------------------------------------------------
set(MODULE_NAME Guidelet)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  GuideletLoadable.py
  __init__.py
  ${MODULE_NAME}Lib/__init__.py
  ${MODULE_NAME}Lib/UltraSound.py
  )

set(MODULE_PYTHON_RESOURCES
  ${MODULE_NAME}Lib/Resources/Icons/${MODULE_NAME}.png
  ${MODULE_NAME}Lib/Resources/Icons/icon_Record.png
  ${MODULE_NAME}Lib/Resources/Icons/icon_Stop.png
  ${MODULE_NAME}Lib/Resources/StyleSheets/DefaultStyle.qss
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
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
