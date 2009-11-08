# -*- cmake -*-
include(Prebuilt)

  # Maybe libxml and glib should have their own .cmake files
  use_prebuilt_binary(libxml)
  use_prebuilt_binary(glib)

  set(GSTREAMER_FOUND ON FORCE BOOL)
  set(GSTREAMER_PLUGINS_BASE_FOUND ON FORCE BOOL)

  use_prebuilt_binary(gstreamer)
  use_prebuilt_binary(gstreamer-plugins)

if (WINDOWS)

  use_prebuilt_binary(iconv)
  set(GSTREAMER_FOUND ON FORCE BOOL)
  set(GSTREAMER_INCLUDE_DIRS
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gio
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gobject
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/libxml2
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/iconv
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gst
      )

  set(GSTREAMER_LIBRARIES
      glib-2.0
      gio-2.0
      gmodule-2.0
      gobject-2.0
      gthread-2.0
      libgstvideo.lib
      libgsttag.lib
      libgstsdp.lib
      libgstrtsp.lib
      libgstrtp.lib
      libgstriff.lib
      libgstreamer-0.10.lib
      libgstpbutils.lib
      libgstnetbuffer.lib
      libgstnet-0.10.lib
      libgstinterfaces.lib
      libgstdshow.lib
      libgstdataprotocol-0.10.lib
      libgstcontroller-0.10.lib
      libgstbase-0.10.lib
      libgstaudio.lib
      libgstapp.lib
      libxml2
      libxml2_a
      libxml2_a_dll
      iconv
      iconv_a
      )

else (WINDOWS)

  set(GSTREAMER_INCLUDE_DIRS
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gstreamer-0.10
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0/glib
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0/gobject
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/libxml2
      )

  if (DARWIN) # Mac

    use_prebuilt_binary(flac)
    use_prebuilt_binary(liboil)
    use_prebuilt_binary(neon)
    use_prebuilt_binary(theora)

    find_library( XML2_LIB
      NAMES xml2.2
      PATHS ${ARCH_PREBUILT_DIRS_RELEASE}
      NO_DEFAULT_PATH
      DOC "libxml2 dynamic library / shared object" )

    if (NOT XML2_LIB)
      message(FATAL_ERROR "libxml2 not found!")
    else (NOT XML2_LIB)
      #message(STATUS "libxml2 found: ${XML2_LIB}")
    endif (NOT XML2_LIB)

    set(GSTREAMER_LIBRARIES
        gstvideo-0.10
        gstaudio-0.10
        gstbase-0.10
        gstreamer-0.10
        gobject-2.0
        gmodule-2.0
        gthread-2.0
        glib-2.0
        ${XML2_LIB}
        )

  else (DARWIN) # Linux

    use_prebuilt_binary(theora)

    set(GSTREAMER_LIBRARIES
        gstvideo-0.10
        gstaudio-0.10
        gstbase-0.10
        gstreamer-0.10
        gobject-2.0
        gmodule-2.0
        dl
        gthread-2.0
        rt
        glib-2.0
        )

  endif (DARWIN)

endif (WINDOWS)

if (GSTREAMER_FOUND AND GSTREAMER_PLUGINS_BASE_FOUND)
  set(GSTREAMER ON CACHE BOOL "Build with GStreamer streaming media support.")
endif (GSTREAMER_FOUND AND GSTREAMER_PLUGINS_BASE_FOUND)

if (GSTREAMER)
  add_definitions(-DLL_GSTREAMER_ENABLED=1)
endif (GSTREAMER)
