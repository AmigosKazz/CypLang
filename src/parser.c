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

int match (Parser* parser, TokenType type) {
    if (parser->current_token->type == type) {
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

AstNode* parse(Parser* parser) {
    return parse_program(parser);
}

AstNode* parse_program (Parser* parser) {
    AstProgram* program = (AstProgram*)create_program_node();

    int capacity = 10;
    program->declarations = malloc(sizeof(AstNode*) * capacity);
    program->declaration_count = 0;

    while (parser->current_token->type != TOKEN_EOF) {
        AstNode* declaration = parse_declaration(parser);

        if (declaration) {
            if (program->declaration_count >= capacity) {
                capacity *= 2;
                program->declarations = realloc(program->declarations, sizeof(AstNode*) * capacity);
            }
            program->declarations[program->declaration_count] = declaration;
            program->declaration_count++;
        }
    }

    return (AstNode*)program;
}

AstNode* parse_declaration(Parser* parser) {
    if (parser->current_token->type == TOKEN_DEBFONC) {
        return parse_function_declaration(parser);
    }
    else if (parser->current_token->type == TOKEN_ENTIER ||
             parser->current_token->type == TOKEN_REEL ||
             parser->current_token->type == TOKEN_CHAINE ||
             parser->current_token->type == TOKEN_CHARACTER ||
             parser->current_token->type == TOKEN_BOOLEEN) {
        return parse_variable_declaration(parser);
             }

    fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: déclaration attendue\n",
            parser->current_token->line, parser->current_token->column);
    advance(parser);

    return NULL;
}

AstNode*  parse_expression(Parser* parser) {
    return parse_equality(parser);
}

AstNode* parse_equality (Parser* parser) {
    AstNode* left = parse_comparison(parser);

    while (parser->current_token->type == TOKEN_EQUAL || parser->current_token->type == TOKEN_BANG_EQUAL) {
        TokenType operator = parser->current_token->type;
        advance(parser);
        AstNode* right = parse_comparison(parser);
        left = create_binary_expr_node(left, operator, right);
    }

    return left;
}

// (<, >, <=, >=)
AstNode* parse_comparison(Parser* parser) {
    AstNode* left = parse_term(parser);

    while (parser->current_token->type == TOKEN_LESS ||
           parser->current_token->type == TOKEN_GREATER ||
           parser->current_token->type == TOKEN_LESS_EQUAL ||
           parser->current_token->type == TOKEN_GREATER_EQUAL) {
        TokenType operator = parser->current_token->type;
        advance(parser);
        AstNode* right = parse_term(parser);
        left = create_binary_expr_node(left, operator, right);
           }

    return left;
}

AstNode* parse_term(Parser* parser) {
    AstNode* left = parse_factor(parser);

    while (parser->current_token->type == TOKEN_PLUS ||
           parser->current_token->type == TOKEN_MINUS ||
           parser->current_token->type == TOKEN_OU) {  // Opérateur logique 'ou'
        TokenType operator = parser->current_token->type;
        advance(parser);
        AstNode* right = parse_factor(parser);
        left = create_binary_expr_node(left, operator, right);
           }

    return left;
}
