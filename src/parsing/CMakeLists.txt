find_package(BISON)
find_package(FLEX)

# Generate into the build tree, never into the source tree. The previous
# version wrote bison_parser.{cc,hh,output} and flex_lexer.cc next to the
# grammar, which pollutes the source directory, breaks read-only source
# checkouts, and leaves stale copies that shadow the freshly generated ones
# (the source dir comes first on the include path).
set(BISON_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bison_parser.cc)
set(FLEX_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/flex_lexer.cc)

BISON_TARGET(parser
  ${CMAKE_CURRENT_SOURCE_DIR}/grammar.yy
  ${BISON_OUTPUT})

FLEX_TARGET(lexer
  ${CMAKE_CURRENT_SOURCE_DIR}/lexer.ll
  ${FLEX_OUTPUT})

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

# The generated parser/lexer now live in the build tree, so the sources they
# include by a bare name (tree.h, ast.h, ...) must be reachable through -I
# rather than the "same directory as the including file" lookup.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PARSING_SOURCES
    tree.cc
    lexer.cc)

add_library(parsing
  ${PARSING_SOURCES}
  ${BISON_parser_OUTPUTS}
  ${FLEX_lexer_OUTPUTS})

target_link_libraries(parsing PRIVATE ast)

# Disable unused-but-set-variable warning for Bison-generated file
set_source_files_properties(${BISON_OUTPUT} PROPERTIES
  COMPILE_FLAGS "-Wno-unused-but-set-variable")

# Disable sign-compare warning for Flex-generated file
set_source_files_properties(${FLEX_OUTPUT} PROPERTIES
  COMPILE_FLAGS "-Wno-sign-compare")
