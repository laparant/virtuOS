project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

set(HDRS thread.h retval.h define.h)
set(SRCS thread.c retval.c)
add_library(thread ${HDRS} ${SRCS})
