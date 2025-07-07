#ifndef INTERPRETER_H

#define INTERPRETER_H

#include "ast.h"

typedef struct {
    AstNode* ast_root;
} Interpreter;

Interpreter* init_interpreter(AstNode* ast_root);
void free_interpreter(Interpreter* interpreter);
void interpret(Interpreter* interpreter);

void interpret_node(Interpreter* interpreter, AstNode* node);
void interpret_block(Interpreter* interpreter, AstBlock* block);
void interpret_variable_decl(Interpreter* interpreter, AstVariableDeclaration* var_decl);
void interpret_function_decl(Interpreter* interpreter, AstFunctionDeclaration* func_decl);
void interpret_function_call(Interpreter* interpreter, AstFunctionCall* func_call);
void interpret_if_statement(Interpreter* interpreter, AstNode* if_stmt);
void interpret_while_statement(Interpreter* interpreter, AstWhileStatement* while_stmt);
void interpret_for_statement(Interpreter* interpreter, AstForStatement* for_stmt);
void interpret_return_statement(Interpreter* interpreter, AstReturnStatement* ret_stmt);

int evaluate_expression(Interpreter* interpreter, AstNode* expr);
int evaluate_binary_expr(Interpreter* interpreter, AstBinaryExpr* bin_expr);
int evaluate_unary_expr(Interpreter* interpreter, AstUnaryExpr* un_expr);
int evaluate_literal(Interpreter* interpreter, AstLiteral* lit);
int evaluate_variable(Interpreter* interpreter, AstVariable* var);

void native_ecrire(const char* message);

#endif //INTERPRETER_H
