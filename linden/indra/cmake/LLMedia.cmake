# -*- cmake -*-

include(GStreamer)
include(QuickTime)

set(LLMEDIA_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/llmedia
    )

set(LLMEDIA_LIBRARIES
    llmedia
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_PLUGINS_BASE_LIBRARIES}
    ${QUICKTIME_LIBRARY}
    )
