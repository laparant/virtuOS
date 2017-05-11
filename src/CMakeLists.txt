project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

set(HDRS thread.h retval.h define.h)
set(SRCS thread.c retval.c mutex.c)

# Creation of the library libthread.so
add_library(thread SHARED ${HDRS} ${SRCS})

# Install the library and thread.h
INSTALL(TARGETS thread
        DESTINATION lib
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

INSTALL(FILES thread.h
        DESTINATION include
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
