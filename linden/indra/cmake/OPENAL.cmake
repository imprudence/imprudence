# -*- cmake -*-

include(Variables)
include(Linking)
include(FindPkgConfig)

set(OPENAL ON CACHE BOOL "Enable OpenAL")

# If STANDALONE but NOT PKG_CONFIG_FOUND we should fail,
# but why try to find it as prebuilt?
if (OPENAL AND STANDALONE AND PKG_CONFIG_FOUND)

  # This module defines
  # OPENAL_INCLUDE_DIRS
  # OPENAL_LIBRARIES
  # OPENAL_FOUND
  pkg_check_modules(OPENAL REQUIRED freealut)	# freealut links with openal.

elseif (OPENAL)

  # message(STATUS "Building with OpenAL audio support")

  # OPENAL_LIB
  use_prebuilt_binary(openal)
  
  if (WINDOWS)
    set(OPENAL_LIB
	  optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libOpenAL32.dll.a.lib
	  debug ${ARCH_PREBUILT_DIRS_DEBUG}/libOpenAL32.dll.a.lib
      )
  
  elseif (DARWIN)
    # Look for for system's OpenAL.framework
    # Nemu: This code has never looked for the system's OpenAL.framework
    # Nemu: should it?
    set(OPENAL_LIB ${ARCH_PREBUILT_DIRS_RELEASE}/libopenal.1.dylib)
  else (WINDOWS)
    set(OPENAL_LIB openal)
  endif (WINDOWS)
  
  if (NOT OPENAL_LIB)
    message(FATAL_ERROR "OpenAL not found!")
  else (NOT OPENAL_LIB)
    # message(STATUS "OpenAL found: ${OPENAL_LIB}")
  endif (NOT OPENAL_LIB)



  # OPENAL_INCLUDE_DIR

  if (DARWIN)
    set(OPENAL_INCLUDE_DIR ${LIBS_PREBUILT_DIR}/${LL_ARCH_DIR}/include/AL)
  else (DARWIN)
    find_path(OPENAL_INCLUDE_DIR
      NAMES al.h
      PATHS ${LIBS_PREBUILT_DIR}/include/AL
      )
  endif (DARWIN)

  if (NOT OPENAL_INCLUDE_DIR)
    message(FATAL_ERROR "al.h not found!")
  else (NOT OPENAL_INCLUDE_DIR)
    # message(STATUS "al.h found in: ${OPENAL_INCLUDE_DIR}")
  endif (NOT OPENAL_INCLUDE_DIR)



  # ALUT_LIB

  if (WINDOWS)
   set(ALUT_LIB
	 optimized ${ARCH_PREBUILT_DIRS_RELEASE}/alut.lib
	 debug ${ARCH_PREBUILT_DIRS_DEBUG}/alut.lib
     )
  elseif (DARWIN)
    find_library( ALUT_LIB
      NAMES alut.0
      PATHS ${ARCH_PREBUILT_DIRS_RELEASE}
      NO_DEFAULT_PATH
      )
  else (WINDOWS)
    set(ALUT_LIB alut)
  endif (WINDOWS)

  if (NOT ALUT_LIB)
    message(FATAL_ERROR "ALUT not found!")
  else (NOT ALUT_LIB)
    # message(STATUS "ALUT found: ${ALUT_LIB}")
  endif (NOT ALUT_LIB)



  # ALUT_INCLUDE_DIR

  find_path(ALUT_INCLUDE_DIR
    NAMES alut.h
    PATHS ${OPENAL_INCLUDE_DIR}
    )

  if (NOT ALUT_INCLUDE_DIR)
    message(FATAL_ERROR "alut.h not found!")
  else (NOT ALUT_INCLUDE_DIR)
    # message(STATUS "alut.h found in: ${ALUT_INCLUDE_DIR}")
  endif (NOT ALUT_INCLUDE_DIR)



  set(OPENAL_LIBRARIES
    ${OPENAL_LIB}
    ${ALUT_LIB}
    )

  set(OPENAL_INCLUDE_DIRS
    ${OPENAL_INCLUDE_DIR}
    ${ALUT_INCLUDE_DIR}
    )
  

  set(OPENAL_FOUND TRUE CACHE BOOL
    "Found OpenAL and ALUT libraries successfully"
    )

endif (OPENAL AND STANDALONE AND PKG_CONFIG_FOUND)
