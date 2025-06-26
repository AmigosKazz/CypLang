#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser* init_parser (Lexer* lexer) {
    Parser* parser = malloc(sizeof(Parser));
    if (!parser) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }

    parser->lexer = lexer;
    parser->current_token = get_the_next_token(lexer);
    parser->previous_token = NULL;

    return parser;
}

void advance (Parser* parser) {
    if (parser->previous_token) {
        free(parser->previous_token);
    }
    parser-> previous_token = parser->current_token;
    parser->current_token = get_the_next_token(parser->lexer);
}

int match (Parser* parser, Token* type) {
    if (parser->current_token == type) {
        advance(parser);
        return 1;
    }

    return 0;
}

int expect(Parser* parser, Token* type, const char* error_message) {
    if (match(parser, type)) {
        return 1;
    }

    fprintf(stderr, "Syntax error in line %d, column %d: %s\n",
        parser->current_token->line, parser->current_token->column, error_message);

    return 0;
}

void free_parser (Parser* parser) {
    if (parser->current_token) {
        free(parser->current_token);
    }
    if (parser->previous_token) {
        free(parser->previous_token);
    }

    free(parser);
}
