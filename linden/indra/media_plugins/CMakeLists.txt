# -*- cmake -*-

add_subdirectory(base)

add_subdirectory(webkit)

if (LINUX)
    add_subdirectory(gstreamer010)
endif (LINUX)

if (WINDOWS OR DARWIN)
    add_subdirectory(quicktime)
endif (WINDOWS OR DARWIN)

add_subdirectory(example)
