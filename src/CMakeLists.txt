# 不包含src目录下的.cc文件，因为主要的.cc文件在子目录中
# 主程序入口在main.cc

add_subdirectory(ast)
add_subdirectory(codegen)
add_subdirectory(parsing)

# 收集所有子目录的源文件
set(KSCOPE_SOURCES main.cc errors.cc)

add_executable(kscope ${KSCOPE_SOURCES})

# Platform-specific linking to handle circular dependencies
if(APPLE)
  # macOS: Suppress duplicate library warnings (LLVM has circular dependencies)
  set_target_properties(kscope PROPERTIES
      LINK_FLAGS "-Wl,-dead_strip,-w"
  )
  target_link_libraries(kscope
      kscope_codegen
      parsing
      ast
      ${REQ_LLVM_LIBRARIES}
  )
elseif(UNIX)
  # Linux: Use linker groups to handle circular dependencies
  target_link_libraries(kscope
      -Wl,--start-group
      ast
      kscope_codegen
      parsing
      ${REQ_LLVM_LIBRARIES}
      -Wl,--end-group
  )
else()
  # Other platforms: use standard linking
  target_link_libraries(kscope
      kscope_codegen
      parsing
      ast
      ${REQ_LLVM_LIBRARIES}
  )
endif()
