# -*- cmake -*-

add_subdirectory(base)

add_subdirectory(webkit)

if (LINUX OR DARWIN)
    add_subdirectory(gstreamer010)
endif (LINUX OR DARWIN)

if (WINDOWS)
    add_subdirectory(quicktime)
endif (WINDOWS)

add_subdirectory(example)
