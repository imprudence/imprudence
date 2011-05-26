# -*- cmake -*-

include(Python)

if (NOT SCRIPTS_DIR)
  set( SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/../scripts" )
endif (NOT SCRIPTS_DIR)

macro (build_version _target)
  execute_process(
      COMMAND ${PYTHON_EXECUTABLE} ${SCRIPTS_DIR}/viewer_info.py --version
      OUTPUT_VARIABLE ${_target}_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

  execute_process(
      COMMAND ${PYTHON_EXECUTABLE} ${SCRIPTS_DIR}/viewer_info.py --name
      OUTPUT_VARIABLE ${_target}_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

  execute_process(
      COMMAND ${PYTHON_EXECUTABLE} ${SCRIPTS_DIR}/viewer_info.py --bundle-id
      OUTPUT_VARIABLE ${_target}_BUNDLE_ID
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )

  if ("${_target}_VERSION" AND "${_target}_NAME")
    message(STATUS "Version of ${_target} is ${${_target}_NAME} ${${_target}_VERSION}")
  else (${_target}_VERSION)
    message(SEND_ERROR "Could not determine ${_target} version")
  endif ("${_target}_VERSION" AND "${_target}_NAME")
endmacro (build_version)
