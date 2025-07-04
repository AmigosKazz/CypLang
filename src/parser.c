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
    parser_advance(parser);

    return NULL;
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
        return create_literal_node_int(0);
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
    parser_advance(parser);
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
    parser_advance(parser);

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

AstNode* parse_variable_declaration(Parser* parser) {
    TokenType type_token = parser->current_token->type;
    parser_advance(parser);

    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: identifiant attendu après le type\n",
                parser->current_token->line, parser->current_token->column);
        return NULL;
    }

    char* name = strdup(parser->current_token->value);
    parser_advance(parser);

    AstNode* initializer = NULL;
    if (parser->current_token->type == TOKEN_ASSIGN) {
        parser_advance(parser);
        initializer = parse_expression(parser);
        if (!initializer) {
            free(name);
            return NULL;
        }
    }

    if (!expect(parser, TOKEN_SEMICOLON, "; attendu après la déclaration de variable")) {
        free(name);
        if (initializer) free_ast_node(initializer);
        return NULL;
    }

    AstNode* type = NULL;

    return create_variable_decl_node(name, type, initializer);
}

AstNode* parse_function_declaration(Parser* parser) {
    parser_advance(parser);

    TokenType return_type_token = parser->current_token->type;
    parser_advance(parser);
    AstNode* return_type = NULL;

    if (parser->current_token->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: identifiant attendu pour le nom de la fonction\n",
                parser->current_token->line, parser->current_token->column);
        return NULL;
    }

    char* name = strdup(parser->current_token->value);
    parser_advance(parser);

    if (!expect(parser, TOKEN_LPAREN, "( attendu après le nom de la fonction")) {
        free(name);
        return NULL;
    }

    AstNode** parameters = NULL;
    int parameter_count = 0;

    if (parser->current_token->type != TOKEN_RPAREN) {
        parameters = malloc(10 * sizeof(AstNode*));
        int capacity = 10;

        do {
            int param_type = 0;
            if (parser->current_token->type == TOKEN_D ||
                parser->current_token->type == TOKEN_R ||
                parser->current_token->type == TOKEN_DR) {
                param_type = parser->current_token->type;
                parser_advance(parser);
            }

            TokenType param_type_token = parser->current_token->type;
            parser_advance(parser);

            if (parser->current_token->type != TOKEN_IDENTIFIER) {
                fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: identifiant attendu pour le paramètre\n",
                        parser->current_token->line, parser->current_token->column);

                for (int i = 0; i < parameter_count; i++) {
                    free_ast_node(parameters[i]);
                }
                free(parameters);
                free(name);
                return NULL;
            }

            char* param_name = strdup(parser->current_token->value);
            parser_advance(parser);

            AstParameter* param = (AstParameter*)create_parameter_node(param_name, NULL, NULL);
            param->param_type = param_type;
            free(param_name);

            if (parameter_count >= capacity) {
                capacity *= 2;
                parameters = realloc(parameters, capacity * sizeof(AstNode*));
            }
            parameters[parameter_count++] = (AstNode*)param;

        } while (match(parser, TOKEN_COMMA));
    }

    if (!expect(parser, TOKEN_RPAREN, ") attendu après les paramètres")) {
        free(name);
        if (parameters) {
            for (int i = 0; i < parameter_count; i++) {
                free_ast_node(parameters[i]);
            }
            free(parameters);
        }
        return NULL;
    }

    AstNode* body = parse_block(parser);
    if (!body) {
        free(name);
        if (parameters) {
            for (int i = 0; i < parameter_count; i++) {
                free_ast_node(parameters[i]);
            }
            free(parameters);
        }
        return NULL;
    }

    if (!expect(parser, TOKEN_FINFONC, "finfonc attendu à la fin de la fonction")) {
        free(name);
        if (parameters) {
            for (int i = 0; i < parameter_count; i++) {
                free_ast_node(parameters[i]);
            }
            free(parameters);
        }
        free_ast_node(body);
        return NULL;
    }

    return create_function_decl_node(name, parameters, parameter_count, return_type, body);
}

AstNode* parse_block(Parser* parser) {
    AstBlock* block = (AstBlock*)create_block_node();

    int capacity = 10;
    block->statements = malloc(sizeof(AstNode*) * capacity);
    block->statement_count = 0;

    while (parser->current_token->type != TOKEN_FINFONC &&
           parser->current_token->type != TOKEN_FINSI &&
           parser->current_token->type != TOKEN_FINFAIRE &&
           parser->current_token->type != TOKEN_EOF) {

        AstNode* statement = parse_statement(parser);
        if (statement) {
            if (block->statement_count >= capacity) {
                capacity *= 2;
                block->statements = realloc(block->statements, sizeof(AstNode*) * capacity);
            }
            block->statements[block->statement_count++] = statement;
        } else {
            parser_advance(parser);
        }
    }

    return (AstNode*)block;
}

AstNode* parse_statement(Parser* parser) {
    TokenType token_type = parser->current_token->type;

    if (token_type == TOKEN_ENTIER ||
        token_type == TOKEN_REEL ||
        token_type == TOKEN_CHAINE ||
        token_type == TOKEN_CHARACTER ||
        token_type == TOKEN_BOOLEEN) {
        return parse_variable_declaration(parser);
    }
    else if (token_type == TOKEN_SI) {
        return parse_if_statement(parser);
    }
    else if (token_type == TOKEN_TANTQUE) {
        return parse_while_statement(parser);
    }
    else if (token_type == TOKEN_POUR) {
        return parse_for_statement(parser);
    }
    else if (token_type == TOKEN_RETOURNER) {
        return parse_return_statement(parser);
    }
    else if (token_type == TOKEN_IDENTIFIER) {
        char* name = strdup(parser->current_token->value);
        parser_advance(parser);

        if (parser->current_token->type == TOKEN_LPAREN) {
            AstNode* call = parse_function_call(parser, name);
            free(name);

            if (!expect(parser, TOKEN_SEMICOLON, "; attendu après l'appel de fonction")) {
                free_ast_node(call);
                return NULL;
            }

            return call;
        } else if (parser->current_token->type == TOKEN_ASSIGN) {
            AstNode* target = create_variable_node(name);
            free(name);

            parser_advance(parser);
            AstNode* value = parse_expression(parser);

            if (!value) {
                free_ast_node(target);
                return NULL;
            }

            if (!expect(parser, TOKEN_SEMICOLON, "; attendu après l'affectation")) {
                free_ast_node(target);
                free_ast_node(value);
                return NULL;
            }

            return create_assignment_node(target, value);
        }

        fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: expression invalide\n",
                parser->current_token->line, parser->current_token->column);
        free(name);
        return NULL;
    }
    else {
        fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: instruction invalide\n",
                parser->current_token->line, parser->current_token->column);
        return NULL;
    }
}

AstNode* parse_for_statement(Parser* parser) {
    parser_advance(parser);

    AstNode* init = NULL;
    if (parser->current_token->type == TOKEN_IDENTIFIER) {
        char* name = strdup(parser->current_token->value);
        parser_advance(parser);

        if (parser->current_token->type == TOKEN_ASSIGN) {
            parser_advance(parser);
            AstNode* value = parse_expression(parser);
            if (!value) {
                free(name);
                return NULL;
            }

            AstNode* target = create_variable_node(name);
            free(name);

            init = create_assignment_node(target, value);
        } else {
            free(name);
            fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: <- attendu après l'identifiant\n",
                    parser->current_token->line, parser->current_token->column);
            return NULL;
        }
    } else {
        fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: identifiant attendu après 'pour'\n",
                parser->current_token->line, parser->current_token->column);
        return NULL;
    }

    AstNode* condition = parse_expression(parser);
    if (!condition) {
        free_ast_node(init);
        return NULL;
    }

    int direction = 0;
    if (parser->current_token->type == TOKEN_HAUT) {
        direction = 1;
        parser_advance(parser);
    } else if (parser->current_token->type == TOKEN_BAS) {
        direction = -1;
        parser_advance(parser);
    } else {
        fprintf(stderr, "Erreur de syntaxe ligne %d, colonne %d: 'haut' ou 'bas' attendu\n",
                parser->current_token->line, parser->current_token->column);
        free_ast_node(init);
        free_ast_node(condition);
        return NULL;
    }

    AstNode* update = NULL;

    if (!expect(parser, TOKEN_FAIRE, "faire attendu après la direction")) {
        free_ast_node(init);
        free_ast_node(condition);
        return NULL;
    }

    AstNode* body = parse_block(parser);
    if (!body) {
        free_ast_node(init);
        free_ast_node(condition);
        return NULL;
    }

    if (!expect(parser, TOKEN_FINFAIRE, "finfaire attendu à la fin de la boucle for")) {
        free_ast_node(init);
        free_ast_node(condition);
        free_ast_node(body);
        return NULL;
    }

    return create_for_stmt_node(init, condition, update, body, direction);
}

AstNode* parse_return_statement(Parser* parser) {
    parser_advance(parser);

    AstNode* value = parse_expression(parser);
    if (!value) {
        return NULL;
    }

    if (!expect(parser, TOKEN_SEMICOLON, "; attendu après l'instruction de retour")) {
        free_ast_node(value);
        return NULL;
    }

    return create_return_stmt_node(value);
}
