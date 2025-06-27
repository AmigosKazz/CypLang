#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token* current_token;
    Token* previous_token;
} Parser;

// parser function
Parser* init_parser(Lexer* lexer);
void advance(Parser* parser);
int match(Parser* parser, Token* type);
int expect(Parser* parser, Token* type, const char* error_message);
void free_parser(Parser* parser);

//function of parsing
AstNode* parse(Parser* parser);
AstNode* parse_program(Parser* parser);
AstNode* parse_declaration(Parser* parser);
AstNode* parse_function_declaration(Parser* parser);
AstNode* parse_variable_declaration(Parser* parser);
AstNode* parse_statement(Parser* parser);
AstNode* parse_block(Parser* parser);
AstNode* parse_if_statement(Parser* parser);
AstNode* parse_while_statement(Parser* parser);
AstNode* parse_for_statement(Parser* parser);
AstNode* parse_return_statement(Parser* parser);
AstNode* parse_assignment(Parser* parser);
AstNode* parse_expression(Parser* parser);
AstNode* parse_equality(Parser* parser);
AstNode* parse_comparison(Parser* parser);
AstNode* parse_term(Parser* parser);
AstNode* parse_factor(Parser* parser);
AstNode* parse_unary(Parser* parser);
AstNode* parse_primary(Parser* parser);
AstNode* parse_function_call(Parser* parser, char* name);

#endif //PARSER_H
