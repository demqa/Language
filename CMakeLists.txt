
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(Language VERSION 2.0 LANGUAGES C CXX)

add_executable( ASTcompilation
    src/Tokens/tokens.cpp
    src/LexemAnalyzer/lex_anal.cpp
    src/BinaryTree/bin_tree.cpp
    src/AST/ast_comp.cpp
    src/AST/ast_main.cpp
)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
