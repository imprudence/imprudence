# -*- cmake -*-
include(Prebuilt)

set(ELFIO_FIND_QUIETLY ON)

if (STANDALONE)
  include(FindELFIO)
elseif (LINUX)
#     if (${ARCH} STREQUAL "x86_64")
#        set(ELFIO_FOUND "NO")
#     else (${ARCH} STREQUAL "x86_64")
        use_prebuilt_binary(elfio)
        set(ELFIO_LIBRARIES ELFIO)
        set(ELFIO_INCLUDE_DIR ${LIBS_PREBUILT_DIR}/include)
        set(ELFIO_FOUND "YES")
#     endif (${ARCH} STREQUAL "x86_64")
endif (STANDALONE)

if (ELFIO_FOUND)
  add_definitions(-DLL_ELFBIN=1)
else (ELFIO_FOUND)
  set(ELFIO_INCLUDE_DIR "")
endif (ELFIO_FOUND)
