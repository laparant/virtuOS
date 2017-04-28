project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-Wall -g -O0 -lpthread")
option(USE_PTHREAD "USE_PTHREAD" OFF)

if(USE_PTHREAD)
    add_definitions(-DUSE_PTHREAD)
    message("The library pthread will be used instead of the implemented functions.")
endif()

add_subdirectory("src")
add_subdirectory("tst")
add_subdirectory("examples")

enable_testing()
