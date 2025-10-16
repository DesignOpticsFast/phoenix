# Ccache toolchain configuration for Phoenix
# This file configures ccache as a compiler launcher for C and C++ compilation

set(CMAKE_C_COMPILER_LAUNCHER ccache CACHE STRING "")
set(CMAKE_CXX_COMPILER_LAUNCHER ccache CACHE STRING "")

# Ensure ccache is found
find_program(CCACHE_EXECUTABLE ccache)
if(NOT CCACHE_EXECUTABLE)
    message(WARNING "ccache not found - compiler caching disabled")
else()
    message(STATUS "Using ccache for compiler caching: ${CCACHE_EXECUTABLE}")
endif()
