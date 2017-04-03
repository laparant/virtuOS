project(VirtuOS LANGUAGES C)
cmake_minimum_required(VERSION 3.1)

set(HDRS thread.h)

set(SRCS thread.c)

add_library(thread ${HDRS} ${SRCS})
