#This cmake script is meant to be run as a build time custom command.
#The script is run using cmake w/ the -P option.
# parameters are passed to this scripts execution with the -D option. 
# A full command line would look like this:
# cmake -D LD_LIBRARY_PATH=~/checkout/libraries -D TEST_CMD=./llunit_test -D ARGS=--touch=llunit_test_ok.txt -P RunBuildTest.cmake

# Parameters:
# LD_LIBRARY_PATH: string, What to set the LD_LIBRARY_PATH env var.
# TEST_CMD: string list, command to run the unit test with, followed by its args.  
set(ENV{LD_LIBRARY_PATH} ${LD_LIBRARY_PATH})
#message("Running: ${TEST_CMD}")
separate_arguments(TEST_CMD)
#message("Running: ${TEST_CMD}")
execute_process(
  COMMAND ${TEST_CMD}
  RESULT_VARIABLE RES
  )

if(NOT ${RES} STREQUAL 0)
  message(STATUS "Failure running: ${TEST_CMD}") 
  message(FATAL_ERROR "Error: ${RES}")
endif(NOT ${RES} STREQUAL 0)