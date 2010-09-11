# -*- cmake -*-
include(Prebuilt)

if (NOT STANDALONE)
  use_prebuilt_binary(libuuid)
  use_prebuilt_binary(vivox)
   if(LINUX AND ${ARCH} STREQUAL "x86_64")
     use_prebuilt_binary(32bitcompatibilitylibs)
   endif(LINUX AND ${ARCH} STREQUAL "x86_64")
  use_prebuilt_binary(fontconfig)
endif(NOT STANDALONE)

if (WINDOWS)
  use_prebuilt_binary(dbghelp)
endif (WINDOWS)

