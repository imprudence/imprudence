# -*- cmake -*-
include(Prebuilt)

if (STANDALONE)
  include(FindPkgConfig)
  pkg_check_modules(OGG REQUIRED ogg)
  pkg_check_modules(VORBIS REQUIRED vorbis)
  pkg_check_modules(VORBISENC REQUIRED vorbisenc)
  pkg_check_modules(VORBISFILE REQUIRED vorbisfile)
else (STANDALONE)
  use_prebuilt_binary(ogg-vorbis)
  set(VORBIS_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include)
  set(VORBISENC_INCLUDE_DIRS ${VORBIS_INCLUDE_DIRS})
  set(VORBISFILE_INCLUDE_DIRS ${VORBIS_INCLUDE_DIRS})

  if (WINDOWS)
    set(OGG_LIBRARIES
        optimized ogg_static
        debug ogg_static_d)
    set(VORBIS_LIBRARIES
        optimized vorbis_static
        debug vorbis_static_d)
    set(VORBISENC_LIBRARIES
        optimized vorbisenc_static
        debug vorbisenc_static_d)
    set(VORBISFILE_LIBRARIES
        optimized vorbisfile_static
        debug vorbisfile_static_d)
  else (WINDOWS)
    set(OGG_LIBRARIES ogg)
    set(VORBIS_LIBRARIES vorbis)
    set(VORBISENC_LIBRARIES vorbisenc)
    set(VORBISFILE_LIBRARIES vorbisfile)
  endif (WINDOWS)
  if(LINUX AND ${ARCH} STREQUAL "x86_64")
    set(VORBIS_LIBRARY_DIRS ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/lib)
    set(VORBISENC_LIBRARY_DIRS ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/lib)
    set(VORBISFILE_LIBRARY_DIRS ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/lib)
    set(OGG_LIBRARY_DIRS ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/lib)
  endif(LINUX AND ${ARCH} STREQUAL "x86_64")
endif (STANDALONE)

link_directories(
    ${VORBIS_LIBRARY_DIRS}
    ${VORBISENC_LIBRARY_DIRS}
    ${VORBISFILE_LIBRARY_DIRS}
    ${OGG_LIBRARY_DIRS}
    )

if(NOT vorbis_link_msg)
  set(vorbis_link_msg ON CACHE BOOL "ogg vorbis linked from:\n")
  message("ogg vorbis linked from:\n"
    ${VORBIS_LIBRARY_DIRS} "\n"
    ${VORBISENC_LIBRARY_DIRS} "\n"
    ${VORBISFILE_LIBRARY_DIRS} "\n"
    ${OGG_LIBRARY_DIRS} "\n"
  )
endif(NOT vorbis_link_msg)
