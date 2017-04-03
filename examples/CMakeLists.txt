project(VirtuOS LANGUAGES C)
cmake_minimum_required(VERSION 2.8)

#add_library(thr_example ../src/thread.h example.c)
add_library(thr_cont ../src/thread.h contextes.c)

#add_executable(examples example.c)
add_executable(context contextes.c)

#target_link_libraries(examples thr_example)
target_link_libraries(context thr_cont)
