project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

if(USE_PTHREAD)
    add_definitions(-DUSE_PTHREAD)
endif()

add_subdirectory("src")
add_subdirectory("tst")
add_subdirectory("examples")

enable_testing()
