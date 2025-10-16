# Ccache toolchain configuration for Phoenix
# This file configures ccache as a compiler launcher for C and C++ compilation

option(ENABLE_CCACHE "Enable ccache launcher" ON)

# Ensure ccache is found
find_program(CCACHE_EXECUTABLE ccache)
if(ENABLE_CCACHE AND CCACHE_EXECUTABLE)
    set(CMAKE_C_COMPILER_LAUNCHER ccache CACHE STRING "")
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache CACHE STRING "")
    message(STATUS "Using ccache for compiler caching: ${CCACHE_EXECUTABLE}")
elseif(ENABLE_CCACHE)
    message(WARNING "ccache not found - compiler caching disabled")
else()
    message(STATUS "ccache disabled by ENABLE_CCACHE=OFF")
endif()
