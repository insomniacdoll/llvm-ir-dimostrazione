# 不包含src目录下的.cc文件，因为主要的.cc文件在子目录中
# 主程序入口在main.cc

add_subdirectory(ast)
add_subdirectory(codegen)
add_subdirectory(parsing)

# 收集所有子目录的源文件
set(KSCOPE_SOURCES main.cc errors.cc)

add_executable(kscope ${KSCOPE_SOURCES})

# 确保库之间的依赖关系正确
target_link_libraries(kscope_codegen ast)
target_link_libraries(parsing ast)

# 使用链接器组确保相互依赖的库正确链接
target_link_libraries(kscope 
    -Wl,--start-group
    ast 
    kscope_codegen 
    parsing 
    ${REQ_LLVM_LIBRARIES}
    -Wl,--end-group
)
