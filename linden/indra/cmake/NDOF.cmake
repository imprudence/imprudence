# -*- cmake -*-
include(Prebuilt)

if (STANDALONE)
  include(FindNDOF)
  if(NOT NDOF_FOUND)
    message(STATUS "Building without N-DoF joystick support")
  endif(NOT NDOF_FOUND)
else (STANDALONE)
  use_prebuilt_binary(ndofdev)

  if (WINDOWS)
    set(NDOF_LIBRARY libndofdev)
  elseif (DARWIN OR LINUX)
    set(NDOF_LIBRARY ndofdev)
  endif (WINDOWS)

  set(NDOF_INCLUDE_DIR ${ARCH_PREBUILT_DIRS}/include/ndofdev)
  set(NDOF_FOUND 1)
endif (STANDALONE)

if (NDOF_FOUND)
  add_definitions(-DLIB_NDOF=1)
  include_directories(${NDOF_INCLUDE_DIR})
else (NDOF_FOUND)
  set(NDOF_INCLUDE_DIR "")
  set(NDOF_LIBRARY "")
endif (NDOF_FOUND)
