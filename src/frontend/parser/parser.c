#include "parser.h"
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
           parser->current_token->type == TOKEN_OU) {
        TokenType operator = parser->current_token->type;
        advance(parser);
        AstNode* right = parse_factor(parser);
        left = create_binary_expr_node(left, operator, right);
    }

    return left;
}

AstNode* parse_factor(Parser* parser) {
    AstNode* left = parse_unary(parser);

    while (parser->current_token->type == TOKEN_ASTERISK ||
        parser->current_token->type == TOKEN_SLASH ||
        parser->current_token->type == TOKEN_MOD ||
        parser->current_token->type == TOKEN_DIV ||
        parser->current_token->type == TOKEN_ET) {
        const TokenType operator = parser->current_token->type;
        advance(parser);
        AstNode* right = parse_unary(parser);
        left = create_binary_expr_node(left, operator, right);
    }

    return left;
}

AstNode* parse_unary(Parser* parser) {
    if (parser->current_token->type == TOKEN_MINUS ||
        parser->current_token->type == TOKEN_NON) {
        TokenType operator = parser->current_token->type;
        advance(parser);
        AstNode* operand = parse_unary(parser);
        return create_unary_expr_node(operator, operand);
    }

    return parse_primary(parser);
}

AstNode* parse_primary(Parser* parser) {
    const TokenType token_type = parser->current_token->type;

    if (token_type == TOKEN_NUMBER) {
        const int value = atoi(parser->current_token->value);
        advance(parser);
        return create_literal_node_int(value);
    }

    if (token_type == TOKEN_STRING) {
        char* value = strdup(parser->current_token->value);
        advance(parser);
        return create_literal_node_string(value);
    }

    if (token_type == TOKEN_CHARACTER) {
        const char value = parser->current_token->value[0];
        advance(parser);
        return create_literal_node_char(value);
    }

    if (token_type == TOKEN_VRAI || token_type == TOKEN_FAUX) {
        const int value = (token_type == TOKEN_VRAI) ? 1 : 0;
        advance(parser);
        return create_literal_node_bool(value);
    }

    if (token_type == TOKEN_NIL) {
        advance(parser);
        return create_literal_node_int(0); // nil like 0
    }

    if (token_type == TOKEN_IDENTIFIER) {
        char* name = strdup(parser->current_token->value);
        advance(parser);

        if (parser->current_token->type == TOKEN_LPAREN) {
            return parse_function_call(parser, name);
        }

        AstNode* var_node = create_variable_node(name);
        free(name);
        return var_node;
    }

    if (token_type == TOKEN_LPAREN) {
        advance(parser);
        AstNode* expr = parse_expression(parser);

        if (!expect(parser, TOKEN_RPAREN, ") attendu")) {
            free_ast_node(expr);
            return NULL;
        }

        return expr;
    }

    fprintf(stderr, "Syntax error in line %d, column %d: %s\n", parser->current_token->line, parser->current_token->column, parser->current_token->value);

    return NULL;
}

AstNode* parse_function_call(Parser* parser, char* name) {
    if (!expect(parser, TOKEN_LPAREN, "( attendu")) {
        free(name);
        return NULL;
    }

    AstNode** arguments = NULL;
    int argument_count = 0;
    int capacity = 4;

    if (parser->current_token->type != TOKEN_RPAREN) {
        arguments = malloc(capacity * sizeof(AstNode*));

        do {
            AstNode* arg = parse_expression(parser);
            if (!arg) {
                for (int i = 0; i < argument_count; i++) {
                    free_ast_node(arguments[i]);
                }
                free(arguments);
                free(name);
                return NULL;
            }

            if (argument_count >= capacity) {
                capacity *= 2;
                arguments = realloc(arguments, capacity * sizeof(AstNode*));
            }
            arguments[argument_count++] = arg;

        } while (match(parser, (TokenType)TOKEN_COMMA));
    }

    if (!expect(parser, TOKEN_RPAREN, ") attendu")) {
        for (int i = 0; i < argument_count; i++) {
            free_ast_node(arguments[i]);
        }
        free(arguments);
        free(name);
        return NULL;
    }

    AstNode* result = create_function_call_node(name, arguments, argument_count);
    free(name);
    return result;
}

AstNode* parse_if_statement(Parser* parser) {
    advance(parser);
    AstNode* condition = parse_expression(parser);
    if (!condition) {
        return NULL;
    }

    if (!expect(parser, TOKEN_ALORS, "alors attendu")) {
        free_ast_node(condition);
        return NULL;
    }

    AstNode* then_branch = parse_block(parser);
    if (!then_branch) {
        free_ast_node(condition);
        return NULL;
    }

    AstNode* else_branch = NULL;
    if (parser->current_token->type == TOKEN_SINON) {
        advance(parser);
        else_branch = parse_block(parser);
        if (!else_branch) {
            free_ast_node(condition);
            free_ast_node(then_branch);
            return NULL;
        }
    }

    if (!expect(parser, TOKEN_FINSI, "finsi attendu")) {
        free_ast_node(condition);
        free_ast_node(then_branch);
        if (else_branch) free_ast_node(else_branch);
        return NULL;
    }

    return create_if_stmt_node(condition, then_branch, else_branch);

}

AstNode* parse_while_statement(Parser* parser) {
    advance(parser);

    AstNode* condition = parse_expression(parser);
    if (!condition) {
        return NULL;
    }

    if (!expect(parser, TOKEN_FAIRE, "faire attendu")) {
        free_ast_node(condition);
        return NULL;
    }

    AstNode* body = parse_block(parser);
    if (!body) {
        free_ast_node(condition);
        return NULL;
    }

    if (!expect(parser, TOKEN_FINFAIRE, "finfaire attendu")) {
        free_ast_node(condition);
        free_ast_node(body);
        return NULL;
    }

    return create_while_stmt_node(condition, body);
}