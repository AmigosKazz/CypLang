cmake_minimum_required(VERSION 3.31)
project(CypLang C)

set(CMAKE_C_STANDARD 11)

add_executable(CypLang src/main.c
        src/token/token.h
        src/ast/ast.h
        src/lexer/lexer.h
        src/parser/parser.h
        src/lexer/lexer.c
        src/parser/parser.c
        src/ast/ast.c)

