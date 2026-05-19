#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast.h"
#include "middle/ir_generator.h"
#include "backend/llvm_emitter.h"

#define MAX_FILE_SIZE (1024 * 1024) // 1MB

static char* readFile(const char* filename);

int main(int argc, char* argv[]) {
    const char* defaultFile = "input.cyp";
    const char* filename;

    if (argc > 1) {
        filename = argv[1];
    } else {
        filename = defaultFile;
        printf("No input file specified, using default: %s\n", defaultFile);
    }

    char* source = readFile(filename);
    if (!source) {
        fprintf(stderr, "Error: Could not read file %s\n", filename);
        return EXIT_FAILURE;
    }

    printf("=== Source (%s) ===\n%s\n", filename, source);

    // 1. Lexer
    Lexer* lexer = init_lexer(source);
    if (!lexer) {
        fprintf(stderr, "Failed to initialize lexer\n");
        free(source);
        return EXIT_FAILURE;
    }

    // 2. Parser → AST
    Parser* parser = init_parser(lexer);
    if (!parser) {
        fprintf(stderr, "Failed to initialize parser\n");
        free_lexer(lexer);
        free(source);
        return EXIT_FAILURE;
    }

    AstNode* ast = parse(parser);
    if (!ast) {
        fprintf(stderr, "Parsing failed\n");
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return EXIT_FAILURE;
    }

    printf("\n=== AST ===\n");
    print_ast(ast, 0);

    // 3. IR generation
    IRProgram* ir = generate_ir(ast);
    if (!ir) {
        fprintf(stderr, "IR generation failed\n");
        free_ast_node(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return EXIT_FAILURE;
    }

    printf("\n");
    ir_print_program(ir);

    // 4. LLVM emission (Phase 2.2: empty module skeleton)
    printf("\n");
    emit_llvm(ir, "cyplang_module");

    ir_free_program(ir);
    free_ast_node(ast);
    free_parser(parser);
    free_lexer(lexer);
    free(source);

    return EXIT_SUCCESS;
}

static char* readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large (max: %d bytes)\n", MAX_FILE_SIZE);
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, size, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    return buffer;
}
