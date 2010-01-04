# -*- cmake -*-
#
# Download viewer artwork even when using standalone

include(Variables)
include(Prebuilt)

if (NOT STANDALONE)
  use_prebuilt_binary(artwork)
else (NOT STANDALONE)
  set(STANDALONE OFF)
    use_prebuilt_binary(artwork)
  set(STANDALONE ON)
endif (NOT STANDALONE)
