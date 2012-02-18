# -*- cmake -*-

set(ZLIB_FIND_QUIETLY ON)
set(ZLIB_FIND_REQUIRED ON)

include(Prebuilt)

if (STANDALONE)
  include(FindZLIB)
else (STANDALONE)
  use_prebuilt_binary(zlib)
  if (WINDOWS)
    set(ZLIB_LIBRARIES 
      debug zlibd
      optimized zlib)
  else (WINDOWS)
    set(ZLIB_LIBRARIES z)
  endif (WINDOWS)
  if (WINDOWS OR LINUX)
    if(LINUX AND ${ARCH} STREQUAL "x86_64")
      set(ZLIB_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include)
    else(LINUX AND ${ARCH} STREQUAL "x86_64")
      set(ZLIB_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include/zlib)
    endif(LINUX AND ${ARCH} STREQUAL "x86_64")
  endif (WINDOWS OR LINUX)
endif (STANDALONE)
