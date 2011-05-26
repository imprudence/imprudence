# 
# Generate the Info.plist file from the template.
# Only @-style "@VARIABLES@" are substituted in the template (not "${VARIABLES}").
# 
# This script is needed because CMake has no other way to perform
# configure_file() as a build-time custom command. :(
# 
# When running this script, you must define (-D) SOURCE_DIR and
# BINARY_DIR to refer to indra and the build directory respectively.
# (Equivalent to CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR in
# CMakeLists.txt )
# 

if (NOT SOURCE_DIR)
  message( FATAL_ERROR "You forgot to define SOURCE_DIR!" )
endif (NOT SOURCE_DIR)

if (NOT BINARY_DIR)
  message( FATAL_ERROR "You forgot to define BINARY_DIR!" )
endif (NOT BINARY_DIR)

set(SCRIPTS_DIR "${SOURCE_DIR}/../scripts")
set(CMAKE_MODULE_PATH "${SOURCE_DIR}/cmake/" "${CMAKE_ROOT/Modules}")

include(BuildVersion)
build_version(viewer)

SET( BUNDLE_NAME           "${viewer_NAME}"                   )
SET( EXECUTABLE            "${viewer_NAME}"                   )
set( BUNDLE_VERSION        "${viewer_VERSION}"                )
set( SHORT_VERSION_STRING  "${viewer_NAME} ${viewer_VERSION}" )
set( ICON_FILE             "viewer.icns"                      )
set( IDENTIFIER            "${viewer_BUNDLE_ID}"              )
set( SIGNATURE             "impr"                             )

configure_file(
  ${SOURCE_DIR}/newview/packaging/mac/Info.plist.in
  ${BINARY_DIR}/newview/packaging/mac/Info.plist 
  @ONLY)
