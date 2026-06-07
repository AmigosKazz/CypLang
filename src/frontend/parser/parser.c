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

void parser_advance (Parser* parser) {
    if (parser->previous_token) {
        free(parser->previous_token);
    }
    parser-> previous_token = parser->current_token;
    parser->current_token = get_the_next_token(parser->lexer);
}

int match (Parser* parser, TokenType type) {
    if (parser->current_token->type == type) {
        parser_advance(parser);
        return 1;
    }

    return 0;
}

int expect(Parser* parser, TokenType type, const char* error_message) {
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
        } else {
            if (parser->current_token->type != TOKEN_EOF) {
                parser_advance(parser);
            }
        }
    }

    return (AstNode*)program;
}

AstNode* parse_declaration(Parser* parser) {
    if (parser->current_token->type == TOKEN_DEBFONC) {
        return parse_function_declaration(parser);
    }
    if (parser->current_token->type == TOKEN_ENTIER ||
        parser->current_token->type == TOKEN_REEL ||
        parser->current_token->type == TOKEN_CHAINE ||
        parser->current_token->type == TOKEN_CHARACTER ||
        parser->current_token->type == TOKEN_BOOLEEN) {
        return parse_variable_declaration(parser);
    }
    // Anything else at the top level: treat as a statement (expression statement,
    // function call, control flow). This makes `afficher("Hello")` a valid top-level
    // statement, which Phase 2.7 needs for Hello World.
    return parse_statement(parser);
}

AstNode*  parse_expression(Parser* parser) {
    return parse_equality(parser);
}

AstNode* parse_equality (Parser* parser) {
    AstNode* left = parse_comparison(parser);

    while (parser->current_token->type == TOKEN_EQUAL || parser->current_token->type == TOKEN_BANG_EQUAL) {
        TokenType operator = parser->current_token->type;
        parser_advance(parser);
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
        parser_advance(parser);
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
        parser_advance(parser);
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
        parser_advance(parser);
        AstNode* right = parse_unary(parser);
        left = create_binary_expr_node(left, operator, right);
    }

    return left;
}

AstNode* parse_unary(Parser* parser) {
    if (parser->current_token->type == TOKEN_MINUS ||
        parser->current_token->type == TOKEN_NON) {
        TokenType operator = parser->current_token->type;
        parser_advance(parser);
        AstNode* operand = parse_unary(parser);
        return create_unary_expr_node(operator, operand);
    }

    return parse_primary(parser);
}

AstNode* parse_primary(Parser* parser) {
    const TokenType token_type = parser->current_token->type;

    if (token_type == TOKEN_NUMBER) {
        const int value = atoi(parser->current_token->value);
        parser_advance(parser);
        return create_literal_node_int(value);
    }

    if (token_type == TOKEN_FLOAT) {
        const float value = strtof(parser->current_token->value, NULL);
        parser_advance(parser);
        return create_literal_node_float(value);
    }

    if (token_type == TOKEN_STRING) {
        char* value = strdup(parser->current_token->value);
        parser_advance(parser);
        return create_literal_node_string(value);
    }

    if (token_type == TOKEN_CHARACTER) {
        const char value = parser->current_token->value[0];
        parser_advance(parser);
        return create_literal_node_char(value);
    }

    if (token_type == TOKEN_VRAI || token_type == TOKEN_FAUX) {
        const int value = (token_type == TOKEN_VRAI) ? 1 : 0;
        parser_advance(parser);
        return create_literal_node_bool(value);
    }

    if (token_type == TOKEN_NIL) {
        parser_advance(parser);
        return create_literal_node_int(0); // nil like 0
    }

    if (token_type == TOKEN_IDENTIFIER) {
        char* name = strdup(parser->current_token->value);
        parser_advance(parser);

        if (parser->current_token->type == TOKEN_LPAREN) {
            return parse_function_call(parser, name);
        }

        AstNode* var_node = create_variable_node(name);
        free(name);
        return var_node;
    }

    if (token_type == TOKEN_LPAREN) {
        parser_advance(parser);
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
    // No advance here — parse_statement already consumed TOKEN_SI via match().
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
        parser_advance(parser);
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
    // No advance here — parse_statement already consumed TOKEN_TANTQUE via match().
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

AstNode* parse_function_declaration(Parser* parser) {
    if (!match(parser, TOKEN_DEBFONC)) {
        return NULL;
    }

    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Nom de fonction attendu\n");
        return NULL;
    }

    char* func_name = strdup(parser->current_token->value);
    parser_advance(parser);

    if (!expect(parser, TOKEN_LPAREN, "( attendu")) {
        free(func_name);
        return NULL;
    }

    // Parameters: comma-separated list of [d|r|dr]? type name
    AstNode** params = NULL;
    int param_count = 0;
    int param_capacity = 0;

    while (parser->current_token->type != TOKEN_RPAREN &&
           parser->current_token->type != TOKEN_EOF) {

        // Optional passing-mode prefix (d/r/dr): 0=default, 1=d, 2=r, 3=dr
        int p_mode = 0;
        if (parser->current_token->type == TOKEN_D)        { p_mode = 1; parser_advance(parser); }
        else if (parser->current_token->type == TOKEN_R)   { p_mode = 2; parser_advance(parser); }
        else if (parser->current_token->type == TOKEN_DR)  { p_mode = 3; parser_advance(parser); }

        // Type keyword (optional for now; just consume if present)
        if (parser->current_token->type == TOKEN_ENTIER  ||
            parser->current_token->type == TOKEN_REEL    ||
            parser->current_token->type == TOKEN_CHAINE  ||
            parser->current_token->type == TOKEN_BOOLEEN) {
            parser_advance(parser);
        }

        if (parser->current_token->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Nom de paramètre attendu (ligne %d, colonne %d)\n",
                    parser->current_token->line, parser->current_token->column);
            for (int i = 0; i < param_count; i++) free_ast_node(params[i]);
            free(params);
            free(func_name);
            return NULL;
        }

        char* param_name = strdup(parser->current_token->value);
        parser_advance(parser);

        AstNode* param = create_parameter_node(param_name, NULL, NULL);
        ((AstParameter*)param)->param_type = p_mode;
        free(param_name);

        if (param_count >= param_capacity) {
            param_capacity = param_capacity == 0 ? 4 : param_capacity * 2;
            params = realloc(params, param_capacity * sizeof(AstNode*));
        }
        params[param_count++] = param;

        if (parser->current_token->type == TOKEN_COMMA) {
            parser_advance(parser);
        }
    }

    if (!expect(parser, TOKEN_RPAREN, ") attendu")) {
        for (int i = 0; i < param_count; i++) free_ast_node(params[i]);
        free(params);
        free(func_name);
        return NULL;
    }

    AstNode* body = parse_block(parser);
    if (!body) {
        for (int i = 0; i < param_count; i++) free_ast_node(params[i]);
        free(params);
        free(func_name);
        return NULL;
    }

    if (!expect(parser, TOKEN_FINFONC, "FINFONC attendu")) {
        for (int i = 0; i < param_count; i++) free_ast_node(params[i]);
        free(params);
        free(func_name);
        free_ast_node(body);
        return NULL;
    }

    return create_function_decl_node(func_name, params, param_count, NULL, body);
}

AstNode* parse_variable_declaration(Parser* parser) {
    TokenType type_token = parser->current_token->type;
    if (type_token != TOKEN_ENTIER && type_token != TOKEN_REEL &&
        type_token != TOKEN_CHAINE && type_token != TOKEN_BOOLEEN) {
        return NULL;
    }

    parser_advance(parser);

    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Nom de variable attendu\n");
        return NULL;
    }

    char* var_name = strdup(parser->current_token->value);
    parser_advance(parser);

    AstNode* initializer = NULL;
    if (match(parser, TOKEN_ASSIGN)) {
        initializer = parse_expression(parser);
        if (!initializer) {
            free(var_name);
            return NULL;
        }
    }

    return create_variable_decl_node(var_name, NULL, initializer);
}

AstNode* parse_statement(Parser* parser) {
    // Handle different statement types
    if (match(parser, TOKEN_SI)) {
        return parse_if_statement(parser);
    }
    if (match(parser, TOKEN_TANTQUE)) {
        return parse_while_statement(parser);
    }
    if (match(parser, TOKEN_RETOURNER)) {
        return parse_return_statement(parser);
    }

    // Try variable declaration
    if (parser->current_token->type == TOKEN_ENTIER ||
        parser->current_token->type == TOKEN_REEL ||
        parser->current_token->type == TOKEN_CHAINE ||
        parser->current_token->type == TOKEN_BOOLEEN) {
        return parse_variable_declaration(parser);
    }

    // Otherwise: expression, or assignment (`x <- expr`)
    AstNode* left = parse_expression(parser);
    if (left && parser->current_token->type == TOKEN_ASSIGN) {
        parser_advance(parser); // consume <-
        AstNode* value = parse_expression(parser);
        if (!value) {
            free_ast_node(left);
            return NULL;
        }
        return create_assignment_node(left, value);
    }
    return left;
}

AstNode* parse_return_statement(Parser* parser) {
    AstNode* value = NULL;
    if (parser->current_token->type != TOKEN_EOF &&
        parser->current_token->type != TOKEN_FINFONC) {
        value = parse_expression(parser);
    }
    return create_return_stmt_node(value);
}

AstNode* parse_block(Parser* parser) {
    AstBlock* block = (AstBlock*)create_block_node();
    int capacity = 8;
    block->statements = malloc(capacity * sizeof(AstNode*));

    while (parser->current_token->type != TOKEN_EOF &&
           parser->current_token->type != TOKEN_FINSI &&
           parser->current_token->type != TOKEN_SINON &&
           parser->current_token->type != TOKEN_FINFAIRE &&
           parser->current_token->type != TOKEN_FINFONC) {

        AstNode* stmt = parse_statement(parser);
        if (!stmt) break;

        if (block->statement_count >= capacity) {
            capacity *= 2;
            block->statements = realloc(block->statements, capacity * sizeof(AstNode*));
        }
        block->statements[block->statement_count++] = stmt;
    }

    return (AstNode*)block;
}