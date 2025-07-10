#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "../parser/parser.h"

void print_ast(AstNode* node, int depth) {
    if (!node) return;

    char indent[100] = "";
    for (int i = 0; i < depth; i++) strcat(indent, "  ");

    switch(node->type) {
        case AST_PROGRAM:
            printf("%sProgramme avec %d déclarations\n", indent, ((AstProgram*)node)->declaration_count);
            for (int i = 0; i < ((AstProgram*)node)->declaration_count; i++) {
                print_ast(((AstProgram*)node)->declarations[i], depth + 1);
            }
            break;
        case AST_VARIABLE_DECL:
            printf("%sVariable: %s\n", indent, ((AstVariableDeclaration*)node)->name);
            if (((AstVariableDeclaration*)node)->initializer) {
                printf("%s  Initializer:\n", indent);
                print_ast(((AstVariableDeclaration*)node)->initializer, depth + 2);
            }
            break;
        case AST_FUNCTION_DECL:
            printf("%sFonction: %s avec %d paramètres\n", indent,
                   ((AstFunctionDeclaration*)node)->name,
                   ((AstFunctionDeclaration*)node)->parameter_count);

            for (int i = 0; i < ((AstFunctionDeclaration*)node)->parameter_count; i++) {
                printf("%s  Paramètre %d:\n", indent, i + 1);
                print_ast(((AstFunctionDeclaration*)node)->parameters[i], depth + 2);
            }

            printf("%s  Corps de la fonction:\n", indent);
            print_ast(((AstFunctionDeclaration*)node)->body, depth + 2);
            break;
        case AST_PARAMETER:
            printf("%sParamètre: %s\n", indent, ((AstParameter*)node)->name);
            break;
        case AST_BLOCK_DECL:
            printf("%sBloc avec %d instructions\n", indent, ((AstBlock*)node)->statement_count);
            for (int i = 0; i < ((AstBlock*)node)->statement_count; i++) {
                print_ast(((AstBlock*)node)->statements[i], depth + 1);
            }
            break;
        case AST_BINARY_EXPR:
            printf("%sExpression binaire (opérateur %d)\n", indent, ((AstBinaryExpr*)node)->operator);
            printf("%s  Gauche:\n", indent);
            print_ast(((AstBinaryExpr*)node)->left, depth + 2);
            printf("%s  Droite:\n", indent);
            print_ast(((AstBinaryExpr*)node)->right, depth + 2);
            break;
        case AST_UNARY_EXPR:
            printf("%sExpression unaire (opérateur %d)\n", indent, ((AstUnaryExpr*)node)->operator);
            print_ast(((AstUnaryExpr*)node)->operand, depth + 1);
            break;
        case AST_ASSIGNMENT:
            printf("%sAffectation\n", indent);
            printf("%s  Cible:\n", indent);
            print_ast(((AstAssignment*)node)->target, depth + 2);
            printf("%s  Valeur:\n", indent);
            print_ast(((AstAssignment*)node)->value, depth + 2);
            break;
        case AST_IF_STATEMENT:
            printf("%sCondition Si\n", indent);
            printf("%s  Condition:\n", indent);
            print_ast(((AstBlock*)node)->statements[0], depth + 2);
            printf("%s  Alors:\n", indent);
            print_ast(((AstBlock*)node)->statements[1], depth + 2);
            if (((AstBlock*)node)->statements[2]) {
                printf("%s  Sinon:\n", indent);
                print_ast(((AstBlock*)node)->statements[2], depth + 2);
            }
            break;
        case AST_WHILE_STATEMENT:
            printf("%sBoucle Tant Que\n", indent);
            printf("%s  Condition:\n", indent);
            print_ast(((AstWhileStatement*)node)->condition, depth + 2);
            printf("%s  Corps:\n", indent);
            print_ast(((AstWhileStatement*)node)->body, depth + 2);
            break;
        case AST_FOR_STATEMENT:
            printf("%sBoucle Pour\n", indent);
            printf("%s  Initialisation:\n", indent);
            print_ast(((AstForStatement*)node)->init, depth + 2);
            printf("%s  Condition:\n", indent);
            print_ast(((AstForStatement*)node)->condition, depth + 2);
            printf("%s  Direction: %s\n", indent, ((AstForStatement*)node)->direction > 0 ? "haut" : "bas");
            printf("%s  Corps:\n", indent);
            print_ast(((AstForStatement*)node)->body, depth + 2);
            break;
        case AST_RETURN_STATEMENT:
            printf("%sRetour\n", indent);
            print_ast(((AstReturnStatement*)node)->value, depth + 1);
            break;
        case AST_FUNCTION_CALL:
            printf("%sAppel de fonction: %s\n", indent, ((AstFunctionCall*)node)->name);
            break;
        case AST_VARIABLE:
            printf("%sVariable: %s\n", indent, ((AstVariable*)node)->name);
            break;
        case AST_LITERAL:
            switch(((AstLiteral*)node)->literal_type) {
                case TOKEN_NUMBER:
                    printf("%sLittéral (nombre): %d\n", indent, ((AstLiteral*)node)->value.int_value);
                    break;
                case TOKEN_STRING:
                    printf("%sLittéral (chaîne): \"%s\"\n", indent, ((AstLiteral*)node)->value.string_value);
                    break;
                case TOKEN_CHARACTER:
                    printf("%sLittéral (caractère): '%c'\n", indent, ((AstLiteral*)node)->value.char_value);
                    break;
                case TOKEN_VRAI:
                    printf("%sLittéral (booléen): vrai\n", indent);
                    break;
                case TOKEN_FAUX:
                    printf("%sLittéral (booléen): faux\n", indent);
                    break;
                default:
                    printf("%sLittéral (type inconnu)\n", indent);
            }
            break;
        default:
            printf("%sNœud de type %d\n", indent, node->type);
    }
}

AstNode* create_program_node() {
    AstProgram* program = (AstProgram*)malloc(sizeof(AstProgram));
    if (!program) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    program->base.type = AST_PROGRAM;
    program->declarations = NULL;
    program->declaration_count = 0;
    return (AstNode*)program;
}

AstNode* create_binary_expr_node(AstNode* left, TokenType operator, AstNode* right) {
    AstBinaryExpr* expr = (AstBinaryExpr*)malloc(sizeof(AstBinaryExpr));
    if (!expr) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    expr->base.type = AST_BINARY_EXPR;
    expr->left = left;
    expr->operator = operator;
    expr->right = right;
    return (AstNode*)expr;
}

AstNode* create_unary_expr_node(TokenType operator, AstNode* operand) {
    AstUnaryExpr* expr = (AstUnaryExpr*)malloc(sizeof(AstUnaryExpr));
    if (!expr) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    expr->base.type = AST_UNARY_EXPR;
    expr->operator = operator;
    expr->operand = operand;
    return (AstNode*)expr;
}

AstNode* create_variable_decl_node(char* name, AstNode* type, AstNode* initializer) {
    AstVariableDeclaration* var = (AstVariableDeclaration*)malloc(sizeof(AstVariableDeclaration));
    if (!var) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    var->base.type = AST_VARIABLE_DECL;
    var->name = strdup(name);
    var->type = type;
    var->initializer = initializer;
    return (AstNode*)var;
}

AstNode* create_function_decl_node(char* name, AstNode** params, int param_count, AstNode* return_type, AstNode* body) {
    AstFunctionDeclaration* func = (AstFunctionDeclaration*)malloc(sizeof(AstFunctionDeclaration));
    if (!func) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    func->base.type = AST_FUNCTION_DECL;
    func->name = strdup(name);
    func->parameters = params;
    func->parameter_count = param_count;
    func->return_type = return_type;
    func->body = body;
    return (AstNode*)func;
}

AstNode* create_parameter_node(char* name, AstNode* type, AstNode* initializer) {
    AstParameter* param = (AstParameter*)malloc(sizeof(AstParameter));
    if (!param) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    param->base.type = AST_PARAMETER;
    param->name = strdup(name);
    param->type = type;
    param->param_type = 0; // Default value
    return (AstNode*)param;
}

AstNode* create_block_node() {
    AstBlock* block = (AstBlock*)malloc(sizeof(AstBlock));
    if (!block) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    block->base.type = AST_BLOCK_DECL;
    block->statements = NULL;
    block->statement_count = 0;
    return (AstNode*)block;
}

AstNode* create_assignment_node(AstNode* target, AstNode* value) {
    AstAssignment* assign = (AstAssignment*)malloc(sizeof(AstAssignment));
    if (!assign) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    assign->base.type = AST_ASSIGNMENT;
    assign->target = target;
    assign->value = value;
    return (AstNode*)assign;
}

AstNode* create_if_stmt_node(AstNode* condition, AstNode* then_branch, AstNode* else_branch) {
    // We're using a dummy struct for if statements until we create a proper one
    // This is a placeholder implementation
    AstBlock* if_stmt = (AstBlock*)malloc(sizeof(AstBlock));
    if (!if_stmt) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    if_stmt->base.type = AST_IF_STATEMENT;
    // For now, just add the condition and branches to the statements array
    if_stmt->statements = malloc(3 * sizeof(AstNode*));
    if_stmt->statements[0] = condition;
    if_stmt->statements[1] = then_branch;
    if_stmt->statements[2] = else_branch;
    if_stmt->statement_count = 3;
    return (AstNode*)if_stmt;
}

AstNode* create_while_stmt_node(AstNode* condition, AstNode* body) {
    AstWhileStatement* while_stmt = (AstWhileStatement*)malloc(sizeof(AstWhileStatement));
    if (!while_stmt) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    while_stmt->base.type = AST_WHILE_STATEMENT;
    while_stmt->condition = condition;
    while_stmt->body = body;
    return (AstNode*)while_stmt;
}

AstNode* create_for_stmt_node(AstNode* init, AstNode* condition, AstNode* update, AstNode* body, int direction) {
    AstForStatement* for_stmt = (AstForStatement*)malloc(sizeof(AstForStatement));
    if (!for_stmt) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    for_stmt->base.type = AST_FOR_STATEMENT;
    for_stmt->init = init;
    for_stmt->condition = condition;
    for_stmt->update = update;
    for_stmt->body = body;
    for_stmt->direction = direction;
    return (AstNode*)for_stmt;
}

AstNode* create_return_stmt_node(AstNode* value) {
    AstReturnStatement* ret = (AstReturnStatement*)malloc(sizeof(AstReturnStatement));
    if (!ret) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    ret->base.type = AST_RETURN_STATEMENT;
    ret->value = value;
    return (AstNode*)ret;
}

AstNode* create_function_call_node(char* name, AstNode** arguments, int argument_count) {
    // This is a placeholder implementation
    AstFunctionCall* call = (AstFunctionCall*)malloc(sizeof(AstFunctionCall));
    if (!call) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    call->base.type = AST_FUNCTION_CALL;
    call->name = strdup(name);
    // Note: We're not storing the arguments here, this would need to be extended
    return (AstNode*)call;
}

AstNode* create_variable_node(char* name) {
    AstVariable* var = (AstVariable*)malloc(sizeof(AstVariable));
    if (!var) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    var->base.type = AST_VARIABLE;
    var->name = strdup(name);
    return (AstNode*)var;
}

AstNode* create_literal_node_int(int value) {
    AstLiteral* literal = (AstLiteral*)malloc(sizeof(AstLiteral));
    if (!literal) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    literal->base.type = AST_LITERAL;
    literal->literal_type = TOKEN_NUMBER;
    literal->value.int_value = value;
    return (AstNode*)literal;
}

AstNode* create_literal_node_float(float value) {
    AstLiteral* literal = (AstLiteral*)malloc(sizeof(AstLiteral));
    if (!literal) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    literal->base.type = AST_LITERAL;
    literal->literal_type = TOKEN_NUMBER;
    literal->value.float_value = value;
    return (AstNode*)literal;
}

AstNode* create_literal_node_string(char* value) {
    AstLiteral* literal = (AstLiteral*)malloc(sizeof(AstLiteral));
    if (!literal) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    literal->base.type = AST_LITERAL;
    literal->literal_type = TOKEN_STRING;
    literal->value.string_value = strdup(value);
    return (AstNode*)literal;
}

AstNode* create_literal_node_char(char value) {
    AstLiteral* literal = (AstLiteral*)malloc(sizeof(AstLiteral));
    if (!literal) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    literal->base.type = AST_LITERAL;
    literal->literal_type = TOKEN_CHARACTER;
    literal->value.char_value = value;
    return (AstNode*)literal;
}

AstNode* create_literal_node_bool(int value) {
    AstLiteral* literal = (AstLiteral*)malloc(sizeof(AstLiteral));
    if (!literal) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    literal->base.type = AST_LITERAL;
    literal->literal_type = value ? TOKEN_VRAI : TOKEN_FAUX;
    literal->value.bool_value = value;
    return (AstNode*)literal;
}

AstNode* create_array_access_node(AstNode* array, AstNode* index) {
    AstArrayAccess* access = (AstArrayAccess*)malloc(sizeof(AstArrayAccess));
    if (!access) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    access->base.type = AST_ARRAY_ACCESS;
    access->array = array;
    access->index = index;
    return (AstNode*)access;
}

AstNode* create_struct_access_node(AstNode* structure, char* field_name) {
    AstStructAccess* access = (AstStructAccess*)malloc(sizeof(AstStructAccess));
    if (!access) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        exit(EXIT_FAILURE);
    }
    access->base.type = AST_STRUCT_ACCESS;
    access->structure = structure;
    access->field_name = strdup(field_name);
    return (AstNode*)access;
}

void free_ast_node(AstNode* node) {
    if (!node) return;

    switch(node->type) {
        case AST_PROGRAM: {
            AstProgram* program = (AstProgram*)node;
            for (int i = 0; i < program->declaration_count; i++) {
                free_ast_node(program->declarations[i]);
            }
            free(program->declarations);
            break;
        }
        case AST_FUNCTION_DECL: {
            AstFunctionDeclaration* func = (AstFunctionDeclaration*)node;
            for (int i = 0; i < func->parameter_count; i++) {
                free_ast_node(func->parameters[i]);
            }
            free(func->parameters);
            free_ast_node(func->return_type);
            free_ast_node(func->body);
            free(func->name);
            break;
        }
        case AST_BLOCK_DECL: {
            AstBlock* block = (AstBlock*)node;
            for (int i = 0; i < block->statement_count; i++) {
                free_ast_node(block->statements[i]);
            }
            free(block->statements);
            break;
        }
        case AST_VARIABLE_DECL: {
            AstVariableDeclaration* var = (AstVariableDeclaration*)node;
            free_ast_node(var->initializer);
            free(var->name);
            break;
        }
        case AST_BINARY_EXPR: {
            AstBinaryExpr* expr = (AstBinaryExpr*)node;
            free_ast_node(expr->left);
            free_ast_node(expr->right);
            break;
        }
        case AST_UNARY_EXPR: {
            AstUnaryExpr* expr = (AstUnaryExpr*)node;
            free_ast_node(expr->operand);
            break;
        }
        case AST_LITERAL: {
            AstLiteral* literal = (AstLiteral*)node;
            if (literal->literal_type == TOKEN_STRING) {
                free(literal->value.string_value);
            }
            break;
        }
        case AST_VARIABLE: {
            AstVariable* var = (AstVariable*)node;
            free(var->name);
            break;
        }
        case AST_PARAMETER: {
            AstParameter* param = (AstParameter*)node;
            free(param->name);
            break;
        }
        case AST_IF_STATEMENT: {
            AstBlock* if_stmt = (AstBlock*)node;
            for (int i = 0; i < if_stmt->statement_count; i++) {
                if (if_stmt->statements[i]) {
                    free_ast_node(if_stmt->statements[i]);
                }
            }
            free(if_stmt->statements);
            break;
        }
        case AST_WHILE_STATEMENT: {
            AstWhileStatement* while_stmt = (AstWhileStatement*)node;
            free_ast_node(while_stmt->condition);
            free_ast_node(while_stmt->body);
            break;
        }
        case AST_FOR_STATEMENT: {
            AstForStatement* for_stmt = (AstForStatement*)node;
            free_ast_node(for_stmt->init);
            free_ast_node(for_stmt->condition);
            if (for_stmt->update) {
                free_ast_node(for_stmt->update);
            }
            free_ast_node(for_stmt->body);
            break;
        }
        case AST_RETURN_STATEMENT: {
            AstReturnStatement* ret = (AstReturnStatement*)node;
            free_ast_node(ret->value);
            break;
        }
        case AST_FUNCTION_CALL: {
            AstFunctionCall* call = (AstFunctionCall*)node;
            free(call->name);
            break;
        }
        case AST_ASSIGNMENT: {
            AstAssignment* assign = (AstAssignment*)node;
            free_ast_node(assign->target);
            free_ast_node(assign->value);
            break;
        }
        case AST_ARRAY_ACCESS: {
            AstArrayAccess* access = (AstArrayAccess*)node;
            free_ast_node(access->array);
            free_ast_node(access->index);
            break;
        }
        case AST_STRUCT_ACCESS: {
            AstStructAccess* access = (AstStructAccess*)node;
            free_ast_node(access->structure);
            free(access->field_name);
            break;
        }
    }

    free(node);
}
