# -*- cmake -*-

include(GStreamer)

set(LLMEDIA_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/llmedia
    )

set(LLMEDIA_LIBRARIES
    llmedia
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_PLUGINS_BASE_LIBRARIES}
    )
