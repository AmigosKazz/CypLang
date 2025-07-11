cmake_minimum_required(VERSION 3.31)
project(CypLang C)

set(CMAKE_C_STANDARD 11)

add_executable(CypLang src/main.c
        src/frontend/token/token.h
        src/frontend/ast/ast.h
        src/frontend/lexer/lexer.h
        src/frontend/parser/parser.h
        src/frontend/lexer/lexer.c
        src/frontend/parser/parser.c
        src/frontend/ast/ast.c
        src/middle/ir_generator.c
        src/backend/llvm_emitter.c
        src/middle/ir_generator.h)

