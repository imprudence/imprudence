# -*- cmake -*-
include(Prebuilt)

SET(TUT_FIND_REQUIRED FALSE)
SET(TUT_FIND_QUIETLY TRUE)

if (STANDALONE)
  if (LL_TESTS)
    SET(TUT_FIND_REQUIRED TRUE)
  endif (LL_TESTS)
  include(FindTut)
  if (TUT_FOUND)
    include_directories(${TUT_INCLUDE_DIR})
  endif (TUT_FOUND)
else (STANDALONE)
  use_prebuilt_binary(tut)
endif(STANDALONE)
