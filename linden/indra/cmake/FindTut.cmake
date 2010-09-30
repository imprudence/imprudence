# -*- cmake -*-

# - Find Tut
# Find the Tut unit test framework includes and library
# This module defines
#  TUT_INCLUDE_DIR, where to find tut/tut.hpp, etc.
#  TUT_FOUND, If false, do not try to use Tut.

find_path(TUT_INCLUDE_DIR tut/tut.hpp
  NO_SYSTEM_ENVIRONMENT_PATH
)

IF (TUT_INCLUDE_DIR)
    SET(TUT_FOUND "YES")
ELSE (TUT_INCLUDE_DIR)
  SET(TUT_FOUND "NO")
ENDIF (TUT_INCLUDE_DIR)

IF (TUT_FOUND)
   IF (NOT TUT_FIND_QUIETLY)
      MESSAGE(STATUS "Found Tut: ${TUT_INCLUDE_DIR}")
	SET(TUT_FIND_QUIETLY TRUE) # Only alert us the first time
   ENDIF (NOT TUT_FIND_QUIETLY)
ELSE (TUT_FOUND)
   IF (TUT_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Tut")
   ENDIF (TUT_FIND_REQUIRED)
ENDIF (TUT_FOUND)

MARK_AS_ADVANCED(
    TUT_INCLUDE_DIR
)

