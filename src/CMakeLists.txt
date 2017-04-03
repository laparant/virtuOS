project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

set(HDRS thread.h)

set(SRCS thread.c)

add_library(thread ${HDRS} ${SRCS})
