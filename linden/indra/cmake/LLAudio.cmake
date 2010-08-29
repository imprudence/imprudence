# -*- cmake -*-

include(Audio)
include(OPENAL)

set(LLAUDIO_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/llaudio
    ${OPENAL_INCLUDE_DIRS}
    )

set(LLAUDIO_LIBRARIES llaudio ${OPENAL_LIBRARIES})
