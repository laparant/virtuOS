project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

set(SRCS test_01_main.c)

set(HDRS ../src/thread.h)

add_library(tests ${HDRS} ${SRCS})
add_executable(test test_01_main.c)
target_link_libraries(test tests)
add_test(NAME test_01 COMMAND test)
