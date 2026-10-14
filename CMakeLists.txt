cmake_minimum_required(VERSION 3.15)

# project() has to come before the compiler settings below: passing them
# beforehand is unreliable, as project() may reset CMAKE_CXX_STANDARD.
project(kscope)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Append to the existing flags instead of replacing them. The previous
# set(CMAKE_CXX_FLAGS "...") silently discarded anything the user passed on
# the command line or through the environment.
add_compile_options(-Wall -Wextra -fvisibility-inlines-hidden)

add_compile_definitions(__STDC_CONSTANT_MACROS)
add_compile_definitions(__STDC_FORMAT_MACROS)
add_compile_definitions(__STDC_LIMIT_MACROS)

# Platform-specific linker flags
if(UNIX AND NOT APPLE)
  # Linux-specific linker flags
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories( src )
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} )
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Default to Debug (LLVM's assertions stay enabled) but let the user or CI
# override it with -DCMAKE_BUILD_TYPE=Release.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES
  orcjit
  native
  core
  support
  analysis
  executionengine
  target
  passes
)

add_subdirectory(src)
