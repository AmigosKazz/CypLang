cmake_minimum_required(VERSION 3.31)
project(CypLang C)

set(CMAKE_C_STANDARD 11)

add_executable(CypLang src/main.c
        include/token.h
        include/ast.h
        include/lexer.h
        include/parser.h
        include/interpreter.h
        include/common.h
        src/lexer.c
        src/parser.c
        src/interpreter.c)
