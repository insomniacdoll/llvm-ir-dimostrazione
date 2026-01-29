file(GLOB CODEGEN_FILES *.cc)

add_library(kscope_codegen ${CODEGEN_FILES})

target_link_libraries(kscope_codegen PUBLIC ast)
