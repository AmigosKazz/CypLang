#ifndef AST_H
#define AST_H

#include "../token/token.h"
#include <stdlib.h>

#include "../lexer/lexer.h"

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION_DECL,
    AST_BLOCK_DECL,
    AST_VARIABLE_DECL,
    AST_PARAMETER,

    AST_ASSIGNMENT,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,

    AST_IF_STATEMENT,
    AST_WHILE_STATEMENT,
    AST_FOR_STATEMENT,
    AST_RETURN_STATEMENT,

    AST_FUNCTION_CALL,
    AST_VARIABLE,
    AST_LITERAL,

    AST_ARRAY_ACCESS,
    AST_STRUCT_ACCESS
} AstNodeType;

typedef struct AstNode {
    AstNodeType type;
    int line;
    int column;
} AstNode;

typedef struct {
    AstNode base;
    struct AstNode** declarations;
    int declaration_count;
} AstProgram;

typedef struct {
    AstNode base;
    char* name;
    struct AstNode** parameters;
    int parameter_count;
    struct AstNode* return_type;
    struct  AstNode* body;
} AstFunctionDeclaration;

typedef struct {
    AstNode base;
    struct AstNode** statements;
    int statement_count;
} AstBlock;

typedef struct {
    AstNode base;
    char* name;
    struct AstNode* type;
    struct AstNode* initializer;
} AstVariableDeclaration;

typedef struct {
    AstNode base;
    char* name;
    struct AstNode* type;
    int param_type;
} AstParameter;

typedef struct {
    AstNode base;
    struct AstNode* target;
    struct AstNode* value;
} AstAssignment;

typedef struct {
    AstNode base;
    struct AstNode* left;
    TokenType operator;
    struct AstNode* right;
} AstBinaryExpr;

typedef struct {
    AstNode base;
    TokenType operator;
    struct AstNode* operator_node;
    struct AstNode* operand;
} AstUnaryExpr;

typedef struct {
    AstNode base;
    struct AstNode* condition;
    struct AstNode* body;
} AstWhileStatement;

typedef struct {
    AstNode base;
    struct AstNode* init;
    struct AstNode* condition;
    struct AstNode* update;
    struct AstNode* body;
    int direction;
} AstForStatement;

typedef struct {
    AstNode base;
    struct AstNode* value;
} AstReturnStatement;

typedef struct {
    AstNode base;
    char* name;
} AstFunctionCall;

typedef struct {
    AstNode base;
    char* name;
} AstVariable;

typedef struct {
    AstNode base;
    TokenType literal_type;
    union {
        int int_value;
        float float_value;
        char* string_value;
        char char_value;
        int bool_value;         // 0: false , 1: true
    } value;
} AstLiteral;
typedef struct {
    AstNode base;
    struct AstNode* array;
    struct AstNode* index;
} AstArrayAccess;

typedef struct {
    AstNode base;
    struct AstNode* structure;
    char* field_name;
} AstStructAccess;

AstNode* create_program_node();
AstNode* create_function_decl_node(char* name, AstNode** params, int param_count, AstNode* return_type, AstNode* body);
AstNode* create_block_node();
AstNode* create_variable_decl_node(char* name, AstNode* type, AstNode* initializer);
AstNode* create_parameter_node(char* name, AstNode* type, AstNode* initializer);
AstNode* create_assignment_node(AstNode* target, AstNode* value);
AstNode* create_binary_expr_node(AstNode* left, TokenType operator, AstNode* right);
AstNode* create_unary_expr_node(TokenType operator, AstNode* operand);
AstNode* create_if_stmt_node(AstNode* condition, AstNode* then_branch, AstNode* else_branch);
AstNode* create_while_stmt_node(AstNode* condition, AstNode* body);
AstNode* create_for_stmt_node(AstNode* init, AstNode* condition, AstNode* update, AstNode* body, int direction);
AstNode* create_return_stmt_node(AstNode* value);
AstNode* create_function_call_node(char* name, AstNode** arguments, int argument_count);
AstNode* create_variable_node(char* name);
AstNode* create_literal_node_int(int value);
AstNode* create_literal_node_float(float value);
AstNode* create_literal_node_string(char* value);
AstNode* create_literal_node_char(char value);
AstNode* create_literal_node_bool(int value);
AstNode* create_array_access_node(AstNode* array, AstNode* index);
AstNode* create_struct_access_node(AstNode* structure, char* field_name);

void free_ast_node(AstNode* node);
void print_ast(AstNode* node, int depth);

#endif //AST_H
