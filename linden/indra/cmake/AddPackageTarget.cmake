# This function adds a custom target named 'package', which runs
# scripts/package.py to create an installer package.
# 
# By default, you must manually build the 'package' target when you
# are ready to create the installer package. But if the global
# AUTOPACKAGE variable is ON ("cmake -D AUTOPACKAGE:BOOL=ON"), the
# 'package' target will be added to the default build target.


set(AUTOPACKAGE OFF CACHE BOOL
    "Automatically build an installer package after compiling.")


function( add_package_target )

  if (AUTOPACKAGE)
    add_custom_target(package ALL)
  else (AUTOPACKAGE)
    add_custom_target(package)
  endif (AUTOPACKAGE)

  add_custom_command(
    TARGET package POST_BUILD
    COMMAND
      ${PYTHON_EXECUTABLE}
      ${SCRIPTS_DIR}/package.py
      --build-dir=${CMAKE_BINARY_DIR}
      --build-type=${CMAKE_BUILD_TYPE}
      --source-dir=${CMAKE_SOURCE_DIR}
  )

endfunction( add_package_target )
