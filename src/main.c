#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser/parser.h"
#include "ast/ast.h"
#include "lexer/lexer.h"

char* read_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier '%s'\n", file_path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Erreur: Impossible de se déplacer à la fin du fichier '%s'\n", file_path);
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1L) {
        fprintf(stderr, "Erreur: Impossible d'obtenir la taille du fichier '%s'\n", file_path);
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Erreur: Impossible de se déplacer au début du fichier '%s'\n", file_path);
        fclose(file);
        return NULL;
    }
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Erreur: Impossible d'allouer de la mémoire pour le contenu du fichier\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "Erreur: Lecture incomplète du fichier '%s'. Bytes lus: %zu, attendu: %ld\n", file_path, bytes_read, file_size);
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    printf("Bienvenue dans le compilateur CypLang !\n");

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_source>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* source = read_file(argv[1]);
    if (!source) {
        return EXIT_FAILURE;
    }

    Lexer* lexer = init_lexer(source);
    if (!lexer) {
        free(source);
        return EXIT_FAILURE;
    }

    Parser* parser = init_parser(lexer);
    if (!parser) {
        free_lexer(lexer);
        free(source);
        return EXIT_FAILURE;
    }

    // Parser le programme
    AstNode* ast = parse(parser);
    if (!ast) {
        fprintf(stderr, "Erreur: Échec de l'analyse syntaxique.\n");
        free_parser(parser);
        free_lexer(lexer);
        free(source);
        return EXIT_FAILURE;
    }

    printf("Analyse syntaxique terminée avec succès !\n");

    printf("\n==== STRUCTURE DE L'AST ====\n");
    print_ast(ast, 0);
    printf("===========================\n\n");

    free_ast_node(ast);
    free_parser(parser);
    free_lexer(lexer);
    free(source);

    return EXIT_SUCCESS;
}