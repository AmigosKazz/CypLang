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
static char* default_output_path(const char* source_path);
static void print_usage(const char* prog);

int main(int argc, char* argv[]) {
    // CLI shapes:
    //   cyplang FILE.cyp                        → debug mode (dump source/AST/IR/LLVM to stdout)
    //   cyplang compile FILE.cyp                → emit FILE.ll next to source
    //   cyplang compile FILE.cyp -o OUT.ll      → emit to OUT.ll
    int compile_mode = 0;
    const char* input_path = NULL;
    const char* output_path = NULL;
    char* output_path_owned = NULL; // free on exit if we allocated a default

    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int argi = 1;
    if (strcmp(argv[argi], "compile") == 0) {
        compile_mode = 1;
        argi++;
        if (argi >= argc) {
            fprintf(stderr, "compile: missing source file\n");
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        input_path = argv[argi++];
        while (argi < argc) {
            if (strcmp(argv[argi], "-o") == 0) {
                argi++;
                if (argi >= argc) {
                    fprintf(stderr, "compile: -o requires an argument\n");
                    return EXIT_FAILURE;
                }
                output_path = argv[argi++];
            } else {
                fprintf(stderr, "compile: unknown argument '%s'\n", argv[argi]);
                return EXIT_FAILURE;
            }
        }
        if (!output_path) {
            output_path_owned = default_output_path(input_path);
            output_path = output_path_owned;
        }
    } else {
        input_path = argv[argi];
    }

    char* source = readFile(input_path);
    if (!source) {
        fprintf(stderr, "Error: Could not read file %s\n", input_path);
        free(output_path_owned);
        return EXIT_FAILURE;
    }

    if (!compile_mode) {
        printf("=== Source (%s) ===\n%s\n", input_path, source);
    }

    // 1. Lexer
    Lexer* lexer = init_lexer(source);
    if (!lexer) {
        fprintf(stderr, "Failed to initialize lexer\n");
        free(source);
        free(output_path_owned);
        return EXIT_FAILURE;
    }

    // 2. Parser → AST
    Parser* parser = init_parser(lexer);
    if (!parser) {
        fprintf(stderr, "Failed to initialize parser\n");
        free_lexer(lexer);
        free(source);
        free(output_path_owned);
        return EXIT_FAILURE;
    }

    AstNode* ast = parse(parser);
    if (!ast) {
        fprintf(stderr, "Parsing failed\n");
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        free(output_path_owned);
        return EXIT_FAILURE;
    }

    if (!compile_mode) {
        printf("\n=== AST ===\n");
        print_ast(ast, 0);
    }

    // 3. IR generation
    IRProgram* ir = generate_ir(ast);
    if (!ir) {
        fprintf(stderr, "IR generation failed\n");
        free_ast_node(ast);
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        free(output_path_owned);
        return EXIT_FAILURE;
    }

    if (!compile_mode) {
        printf("\n");
        ir_print_program(ir);
        printf("\n");
    }

    // 4. LLVM emission
    int emit_rc = emit_llvm(ir, "cyplang_module", compile_mode ? output_path : NULL);

    ir_free_program(ir);
    free_ast_node(ast);
    free_parser(parser);
    free_lexer(lexer);
    free(source);
    free(output_path_owned);

    return emit_rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

// Given "path/to/foo.cyp", returns a malloc'd "path/to/foo.ll".
// If no .cyp extension, just appends ".ll".
static char* default_output_path(const char* source_path) {
    size_t len = strlen(source_path);
    const char* dot = strrchr(source_path, '.');
    size_t base_len = (dot && strcmp(dot, ".cyp") == 0) ? (size_t)(dot - source_path) : len;
    char* out = malloc(base_len + 4); // ".ll" + '\0'
    memcpy(out, source_path, base_len);
    memcpy(out + base_len, ".ll", 4);
    return out;
}

static void print_usage(const char* prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s FILE.cyp                          dump source/AST/IR/LLVM to stdout\n"
        "  %s compile FILE.cyp [-o OUT.ll]      emit LLVM IR to a file\n",
        prog, prog);
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
