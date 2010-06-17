# -*- cmake -*-

include(APR)
include(Boost)
include(EXPAT)
include(ZLIB)

set(LLCOMMON_INCLUDE_DIRS
    ${LIBS_OPEN_DIR}/llcommon
    ${APRUTIL_INCLUDE_DIR}
    ${APR_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )

# Files that need PIC code (pluginAPI) need to set REQUIRE_PIC on 64bit systems
# this will link against a llcommon built with Position Independent Code
# this is a requirment to link a static library (.a) to a DSO on 64 bit systems

if(REQUIRE_PIC)
	set(LLCOMMON_LIBRARIES llcommonPIC)
else(REQUIRE_PIC)
	set(LLCOMMON_LIBRARIES llcommon)
endif(REQUIRE_PIC)

#force clear the flag, files that need this must explicity set it themselves
set(REQUIRE_PIC 0)