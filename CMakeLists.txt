cmake_minimum_required(VERSION 3.10)
project(CypLang C)

set(CMAKE_C_STANDARD 11)

# Locate LLVM (Homebrew keg-only path on macOS, or whatever is in CMAKE_PREFIX_PATH)
if(NOT LLVM_DIR AND EXISTS "/opt/homebrew/opt/llvm/lib/cmake/llvm")
    set(LLVM_DIR "/opt/homebrew/opt/llvm/lib/cmake/llvm")
endif()
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} at ${LLVM_DIR}")

add_executable(CypLang
        src/main.c
        src/frontend/token/token.h
        src/frontend/ast/ast.h
        src/frontend/ast/ast.c
        src/frontend/lexer/lexer.h
        src/frontend/lexer/lexer.c
        src/frontend/parser/parser.h
        src/frontend/parser/parser.c
        src/middle/ir_generator.h
        src/middle/ir_generator.c
        src/backend/llvm_emitter.h
        src/backend/llvm_emitter.c)

target_include_directories(CypLang PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(CypLang PRIVATE ${LLVM_DEFINITIONS})

# Link against the LLVM Core component (sufficient for Phase 2.x — IR builder, module, context).
llvm_map_components_to_libnames(LLVM_LIBS core)
target_link_libraries(CypLang PRIVATE ${LLVM_LIBS})

