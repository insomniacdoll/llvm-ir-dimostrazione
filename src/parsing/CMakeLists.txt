find_package(BISON)
find_package(FLEX)

BISON_TARGET(parser
  ${CMAKE_CURRENT_SOURCE_DIR}/grammar.yy
  ${CMAKE_CURRENT_SOURCE_DIR}/bison_parser.cc)

FLEX_TARGET(lexer
  ${CMAKE_CURRENT_SOURCE_DIR}/lexer.ll
  ${CMAKE_CURRENT_SOURCE_DIR}/flex_lexer.cc)

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PARSING_SOURCES
    tree.cc
    lexer.cc)

add_library(parsing
  ${PARSING_SOURCES}
  ${BISON_parser_OUTPUTS}
  ${FLEX_lexer_OUTPUTS})

target_link_libraries(parsing ast)

# Disable unused-but-set-variable warning for Bison-generated file
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/bison_parser.cc PROPERTIES
  COMPILE_FLAGS "-Wno-unused-but-set-variable")

# Disable sign-compare warning for Flex-generated file
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/flex_lexer.cc PROPERTIES
  COMPILE_FLAGS "-Wno-sign-compare")
