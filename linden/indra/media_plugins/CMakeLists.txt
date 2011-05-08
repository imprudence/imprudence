# -*- cmake -*-

add_subdirectory(base)

add_subdirectory(webkit)

if (LINUX)
    add_subdirectory(gstreamer010)
endif (LINUX)

# We use gstreamer for audio, quicktime for media on win-- MC
if (WINDOWS)
    add_subdirectory(gstreamer010)
    add_subdirectory(quicktime)
endif (WINDOWS)

if (DARWIN)
    add_subdirectory(quicktime)
endif (DARWIN)

add_subdirectory(example)
