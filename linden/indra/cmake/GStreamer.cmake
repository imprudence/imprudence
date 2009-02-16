# -*- cmake -*-
include(Prebuilt)

  # possible libxml should have its own .cmake file instead
  use_prebuilt_binary(libxml)
  set(GSTREAMER_FOUND ON FORCE BOOL)
  set(GSTREAMER_PLUGINS_BASE_FOUND ON FORCE BOOL)
  use_prebuilt_binary(gstreamer)

if (WINDOWS)

  use_prebuilt_binary(libxml)
  use_prebuilt_binary(iconv)
  use_prebuilt_binary(gstreamer-plugins)
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
      libgstvideo-0.10
      libgsttag-0.10
      libgstsdp-0.10
      libgstrtsp-0.10
      libgstrtp-0.10
      libgstriff-0.10
      libgstreamer-0.10
      libgstpbutils-0.10
      libgstnetbuffer-0.10
      libgstnet-0.10
      libgstinterfaces-0.10
      libgstdshow-0.10
      libgstdataprotocol-0.10
      libgstcontroller-0.10
      libgstbase-0.10
      libgstaudio-0.10
      libgstapp-0.10
      libxml2
      libxml2_a
      libxml2_a_dll
      iconv
      iconv_a
      )

else (WINDOWS)

  include(FindPkgConfig)

  pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
  pkg_check_modules(GSTREAMER_PLUGINS_BASE REQUIRED gstreamer-plugins-base-0.10)
  set(GSTREAMER_INCLUDE_DIRS
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/gstreamer-0.10
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0/glib
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/glib-2.0/gobject
      ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/libxml2
      )

  # We don't need to explicitly link against gstreamer itself, because
  # LLMediaImplGStreamer probes for the system's copy at runtime.
  set(GSTREAMER_LIBRARIES
      gobject-2.0
      gmodule-2.0
      dl
      gthread-2.0
      rt
      glib-2.0
      )

endif (WINDOWS)

if (GSTREAMER_FOUND AND GSTREAMER_PLUGINS_BASE_FOUND)
  set(GSTREAMER ON CACHE BOOL "Build with GStreamer streaming media support.")
endif (GSTREAMER_FOUND AND GSTREAMER_PLUGINS_BASE_FOUND)

if (GSTREAMER)
  add_definitions(-DLL_GSTREAMER_ENABLED=1)
endif (GSTREAMER)
