# -*- cmake -*-
include(Linking)
include(Prebuilt)

if (STANDALONE)
    set(MOZLIB OFF CACHE BOOL 
        "Enable Mozilla support in the viewer (requires llmozlib library).")
else (STANDALONE)
    use_prebuilt_binary(llmozlib)
    set(MOZLIB ON CACHE BOOL
        "Enable Mozilla support in the viewer (requires llmozlib library).")
endif (STANDALONE)

if (MOZLIB)
    add_definitions(-DLL_LLMOZLIB_ENABLED=1)

    if (LINUX)
        link_directories(${CMAKE_SOURCE_DIR}/newview/app_settings/mozilla-runtime-linux-${ARCH})
        set(MOZLIB_LIBRARIES
            llmozlib2
            mozjs
            nspr4
            plc4
            plds4
            xpcom
            xul
            profdirserviceprovider_s
            )
    elseif (WINDOWS)
        if (MSVC71)
            set(MOZLIB_LIBRARIES 
                debug llmozlib2d
                optimized llmozlib2)
        elseif (MSVC80 OR MSVC90)
            set(MOZLIB_LIBRARIES 
                debug llmozlib2d-vc80
                optimized llmozlib2-vc80)
        endif (MSVC71)
    else (LINUX)
        set(MOZLIB_LIBRARIES
          optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libllmozlib2.dylib
          debug ${ARCH_PREBUILT_DIRS_DEBUG}/libllmozlib2.dylib
          )
    endif (LINUX)
else (MOZLIB)
    add_definitions(-DLL_LLMOZLIB_ENABLED=0)
endif (MOZLIB)
