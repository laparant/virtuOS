project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

# test01
add_executable(test01 test_01_main.c)
target_link_libraries (test01 thread)
include_directories ("${PROJECT_SOURCE_DIR}/src")
