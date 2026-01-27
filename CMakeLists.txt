cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(__STDC_CONSTANT_MACROS)
add_compile_definitions(__STDC_FORMAT_MACROS)
add_compile_definitions(__STDC_LIMIT_MACROS)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -fvisibility-inlines-hidden")

# Platform-specific linker flags
if(UNIX AND NOT APPLE)
  # Linux-specific linker flags
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

project(kscope)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories( src )
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} )
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

set(CMAKE_BUILD_TYPE Debug)

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
