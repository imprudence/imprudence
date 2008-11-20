# -*- cmake -*-

set(OPENAL ON CACHE BOOL "Enable OpenAL")

if (OPENAL)
  include(FindPkgConfig)
  pkg_check_modules(OPENAL_LIB REQUIRED openal)
  pkg_check_modules(FREEAULT_LIB REQUIRED freealut)
  set(OPENAL_LIBRARIES 
    openal
    alut
    )
  
endif (OPENAL)
