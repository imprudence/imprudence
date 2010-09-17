# -*- cmake -*-
include(Prebuilt)

include(Linking)
set(JPEG_FIND_QUIETLY ON)
set(JPEG_FIND_REQUIRED ON)

if (STANDALONE)
  include(FindJPEG)
else (STANDALONE)
  use_prebuilt_binary(jpeglib)
  if (LINUX)
    set(JPEG_LIBRARIES jpeg)
  elseif (DARWIN)
    find_library(JPEG_LIBRARIES
      NAMES jpeg.62
      PATHS ${ARCH_PREBUILT_DIRS_RELEASE}
      )
    if (NOT JPEG_LIBRARIES)
      message(STATUS "WARNING: libjpeg.62.dylib not found! Falling back to -ljpeg. This might potentially link to the wrong libjpeg.")
      set(JPEG_LIBRARIES jpeg)
    endif (NOT JPEG_LIBRARIES)
  elseif (WINDOWS)
    set(JPEG_LIBRARIES jpeglib)
  endif (LINUX)
  set(JPEG_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include)
endif (STANDALONE)
