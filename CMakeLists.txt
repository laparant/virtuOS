project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

# Tests
include (CTest)
set( CTEST_MEMORYCHECK_COMMAND valgrind )
  set( CTEST_MEMORYCHECK_COMMAND_OPTIONS "--tool=callgrind -v" )
enable_testing()

set(CMAKE_C_FLAGS "-Wall -g -O0 -lpthread")
option(USE_PTHREAD "USE_PTHREAD" OFF)

if(USE_PTHREAD)
    add_definitions(-DUSE_PTHREAD)
    message("The library pthread will be used instead of the implemented functions.")
endif()

add_subdirectory("src")
add_subdirectory("tst")
add_subdirectory("examples")
add_subdirectory("doc")
