# 不包含src目录下的.cc文件，因为主要的.cc文件在子目录中
# 主程序入口在main.cc

add_subdirectory(ast)
add_subdirectory(codegen)
add_subdirectory(parsing)

# 收集所有子目录的源文件
set(KSCOPE_SOURCES main.cc errors.cc)

add_executable(kscope ${KSCOPE_SOURCES})

# Platform-specific linking to handle circular dependencies
if(UNIX AND NOT APPLE)
  # Use linker groups for Linux to handle circular dependencies
  target_link_libraries(kscope
      -Wl,--start-group
      ast
      kscope_codegen
      parsing
      ${REQ_LLVM_LIBRARIES}
      -Wl,--end-group
  )
else()
  # For macOS and other platforms, use standard linking
  # The order may need adjustment based on actual dependencies
  target_link_libraries(kscope
      ast
      kscope_codegen
      parsing
      ${REQ_LLVM_LIBRARIES}
  )
endif()
