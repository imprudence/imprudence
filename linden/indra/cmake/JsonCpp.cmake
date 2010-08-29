# -*- cmake -*-

include(Prebuilt)

set(JSONCPP_FIND_QUIETLY ON)
set(JSONCPP_FIND_REQUIRED ON)

if (STANDALONE)
  include(FindJsonCpp)
else (STANDALONE)
  use_prebuilt_binary(jsoncpp)
  if (WINDOWS)
    if (MSVC80)
      set(JSONCPP_LIBRARIES 
        debug json_vc80d
        optimized json_vc80)
    elseif (MSVC90)
      set(JSONCPP_LIBRARIES 
        debug json_vc90d
        optimized json_vc90)
	endif(MSVC80)
  elseif (DARWIN)
    set(JSONCPP_LIBRARIES json_mac-universal-gcc_libmt)
  elseif (LINUX)
    set(JSONCPP_LIBRARIES jsoncpp)
  endif (WINDOWS)
  set(JSONCPP_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include/jsoncpp)
endif (STANDALONE)
