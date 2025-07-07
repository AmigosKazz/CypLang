#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

typedef struct Variable {
    char* name;
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
        char char_value;
        int bool_value;
    } value;
    struct Variable* next;
} Variable;

typedef struct Environment {
    Variable* variables;
    struct Environment* parent;
} Environment;

typedef struct {
    AstNode* ast_root;
    Environment* current_env;
    int return_value;
    int has_return;
} InterpreterState;

Environment* create_environment(Environment* parent) {
    Environment* env = (Environment*)malloc(sizeof(Environment));
    if (!env) {
        fprintf(stderr, "Erreur d'allocation mémoire pour l'environnement\n");
        exit(EXIT_FAILURE);
    }
    env->variables = NULL;
    env->parent = parent;
    return env;
}

void free_environment(Environment* env) {
    if (!env) return;

    Variable* current = env->variables;
    while (current) {
        Variable* next = current->next;
        free(current->name);
        if (current->type == TOKEN_STRING) {
            free(current->value.string_value);
        }
        free(current);
        current = next;
    }

    free(env);
}

Variable* get_variable(Environment* env, const char* name) {
    while (env) {
        Variable* var = env->variables;
        while (var) {
            if (strcmp(var->name, name) == 0) {
                return var;
            }
            var = var->next;
        }
        env = env->parent;
    }
    return NULL;
}

void set_variable(Environment* env, const char* name, int type, void* value) {
    Variable* var = get_variable(env, name);

    if (var) {
        if (var->type == TOKEN_STRING && type != TOKEN_STRING) {
            free(var->value.string_value);
        }

        var->type = type;
        switch (type) {
            case TOKEN_NUMBER:
                var->value.int_value = *(int*)value;
                break;
            case TOKEN_REEL:
                var->value.float_value = *(float*)value;
                break;
            case TOKEN_STRING:
                var->value.string_value = strdup((char*)value);
                break;
            case TOKEN_CHARACTER:
                var->value.char_value = *(char*)value;
                break;
            case TOKEN_VRAI:
            case TOKEN_FAUX:
                var->value.bool_value = type == TOKEN_VRAI ? 1 : 0;
                break;
        }
    } else {
        var = (Variable*)malloc(sizeof(Variable));
        if (!var) {
            fprintf(stderr, "Erreur d'allocation mémoire pour la variable\n");
            exit(EXIT_FAILURE);
        }

        var->name = strdup(name);
        var->type = type;

        switch (type) {
            case TOKEN_NUMBER:
                var->value.int_value = *(int*)value;
                break;
            case TOKEN_REEL:
                var->value.float_value = *(float*)value;
                break;
            case TOKEN_STRING:
                var->value.string_value = strdup((char*)value);
                break;
            case TOKEN_CHARACTER:
                var->value.char_value = *(char*)value;
                break;
            case TOKEN_VRAI:
            case TOKEN_FAUX:
                var->value.bool_value = type == TOKEN_VRAI ? 1 : 0;
                break;
        }

        var->next = env->variables;
        env->variables = var;
    }
}

Interpreter* init_interpreter(AstNode* ast_root) {
    Interpreter* interpreter = (Interpreter*)malloc(sizeof(Interpreter));
    if (!interpreter) {
        fprintf(stderr, "Erreur d'allocation mémoire pour l'interpréteur\n");
        exit(EXIT_FAILURE);
    }

    interpreter->ast_root = ast_root;

    InterpreterState* state = (InterpreterState*)malloc(sizeof(InterpreterState));
    if (!state) {
        fprintf(stderr, "Erreur d'allocation mémoire pour l'état de l'interpréteur\n");
        free(interpreter);
        exit(EXIT_FAILURE);
    }

    state->ast_root = ast_root;
    state->current_env = create_environment(NULL);
    state->return_value = 0;
    state->has_return = 0;

    *(InterpreterState**)interpreter = state;

    return interpreter;
}

void free_interpreter(Interpreter* interpreter) {
    if (!interpreter) return;

    InterpreterState* state = *(InterpreterState**)interpreter;
    if (state) {
        free_environment(state->current_env);
        free(state);
    }

    free(interpreter);
}

void interpret(Interpreter* interpreter) {
    if (!interpreter) return;

    InterpreterState* state = *(InterpreterState**)interpreter;
    if (!state || !state->ast_root) return;

    interpret_node(interpreter, state->ast_root);
}

void interpret_node(Interpreter* interpreter, AstNode* node) {
    if (!interpreter || !node) return;

    InterpreterState* state = *(InterpreterState**)interpreter;

    switch (node->type) {
        case AST_PROGRAM:
            {
                AstProgram* program = (AstProgram*)node;
                for (int i = 0; i < program->declaration_count; i++) {
                    interpret_node(interpreter, program->declarations[i]);
                }
            }
            break;
        case AST_BLOCK_DECL:
            interpret_block(interpreter, (AstBlock*)node);
            break;
        case AST_VARIABLE_DECL:
            interpret_variable_decl(interpreter, (AstVariableDeclaration*)node);
            break;
        case AST_FUNCTION_DECL:
            interpret_function_decl(interpreter, (AstFunctionDeclaration*)node);
            break;
        case AST_IF_STATEMENT:
            interpret_if_statement(interpreter, node);
            break;
        case AST_WHILE_STATEMENT:
            interpret_while_statement(interpreter, (AstWhileStatement*)node);
            break;
        case AST_FOR_STATEMENT:
            interpret_for_statement(interpreter, (AstForStatement*)node);
            break;
        case AST_RETURN_STATEMENT:
            interpret_return_statement(interpreter, (AstReturnStatement*)node);
            break;
        case AST_FUNCTION_CALL:
            interpret_function_call(interpreter, (AstFunctionCall*)node);
            break;
        case AST_ASSIGNMENT:
            {
                AstAssignment* assignment = (AstAssignment*)node;

                int value = evaluate_expression(interpreter, assignment->value);

                if (assignment->target->type == AST_VARIABLE) {
                    AstVariable* var = (AstVariable*)assignment->target;
                    set_variable(state->current_env, var->name, TOKEN_NUMBER, &value);
                } else {
                    fprintf(stderr, "Erreur: tentative d'assignation à une non-variable\n");
                }
            }
            break;
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
        case AST_VARIABLE:
        case AST_LITERAL:
            evaluate_expression(interpreter, node);
            break;
        default:
            fprintf(stderr, "Type de nœud AST non géré: %d\n", node->type);
    }
}

void interpret_block(Interpreter* interpreter, AstBlock* block) {
    if (!interpreter || !block) return;

    InterpreterState* state = *(InterpreterState**)interpreter;

    Environment* previous_env = state->current_env;
    state->current_env = create_environment(previous_env);

    for (int i = 0; i < block->statement_count && !state->has_return; i++) {
        interpret_node(interpreter, block->statements[i]);
    }

    Environment* block_env = state->current_env;
    state->current_env = previous_env;
    free_environment(block_env);
}

void interpret_variable_decl(Interpreter* interpreter, AstVariableDeclaration* var_decl) {
    if (!interpreter || !var_decl) return;

    InterpreterState* state = *(InterpreterState**)interpreter;

    if (var_decl->initializer) {
        int value = evaluate_expression(interpreter, var_decl->initializer);
        set_variable(state->current_env, var_decl->name, TOKEN_NUMBER, &value);
    } else {
        int zero = 0;
        set_variable(state->current_env, var_decl->name, TOKEN_NUMBER, &zero);
    }
}

void interpret_function_decl(Interpreter* interpreter, AstFunctionDeclaration* func_decl) {
    //
}

void interpret_function_call(Interpreter* interpreter, AstFunctionCall* func_call) {
    if (!interpreter || !func_call) return;

    if (strcmp(func_call->name, "ecrire") == 0) {
        printf("Fonction 'ecrire' appelée\n");
        native_ecrire("Message par défaut");
    } else {
        fprintf(stderr, "Appel à une fonction non définie: %s\n", func_call->name);
    }
}

void interpret_if_statement(Interpreter* interpreter, AstNode* if_stmt) {
    if (!interpreter || !if_stmt || if_stmt->type != AST_IF_STATEMENT) return;

    // AstNode* if_stmt is structured as an AstBlock with 3 elements:
    // statements[0] = condition
    // statements[1] = then_branch
    // statements[2] = else_branch (maybe NULL)

    AstBlock* if_block = (AstBlock*)if_stmt;
    if (if_block->statement_count < 2) return;

    int condition = evaluate_expression(interpreter, if_block->statements[0]);

    if (condition) {
        interpret_node(interpreter, if_block->statements[1]);
    } else if (if_block->statement_count > 2 && if_block->statements[2]) {
        interpret_node(interpreter, if_block->statements[2]);
    }
}

void interpret_while_statement(Interpreter* interpreter, AstWhileStatement* while_stmt) {
    if (!interpreter || !while_stmt) return;

    InterpreterState* state = *(InterpreterState**)interpreter;

    while (!state->has_return) {
        int condition = evaluate_expression(interpreter, while_stmt->condition);
        if (!condition) break;

        interpret_node(interpreter, while_stmt->body);
    }
}

void interpret_for_statement(Interpreter* interpreter, AstForStatement* for_stmt) {
    if (!interpreter || !for_stmt) return;

    InterpreterState* state = *(InterpreterState**)interpreter;

    Environment* previous_env = state->current_env;
    state->current_env = create_environment(previous_env);

    if (for_stmt->init) {
        interpret_node(interpreter, for_stmt->init);
    }

    while (!state->has_return) {
        if (for_stmt->condition) {
            int condition = evaluate_expression(interpreter, for_stmt->condition);
            if (!condition) break;
        }

        interpret_node(interpreter, for_stmt->body);

        if (for_stmt->update) {
            evaluate_expression(interpreter, for_stmt->update);
        }
    }

    Environment* loop_env = state->current_env;
    state->current_env = previous_env;
    free_environment(loop_env);
}

void interpret_return_statement(Interpreter* interpreter, AstReturnStatement* ret_stmt) {
    if (!interpreter || !ret_stmt) return;

    InterpreterState* state = *(InterpreterState**)interpreter;

    if (ret_stmt->value) {
        state->return_value = evaluate_expression(interpreter, ret_stmt->value);
    } else {
        state->return_value = 0;
    }

    state->has_return = 1;
}

int evaluate_expression(Interpreter* interpreter, AstNode* expr) {
    if (!interpreter || !expr) return 0;

    switch (expr->type) {
        case AST_BINARY_EXPR:
            return evaluate_binary_expr(interpreter, (AstBinaryExpr*)expr);
        case AST_UNARY_EXPR:
            return evaluate_unary_expr(interpreter, (AstUnaryExpr*)expr);
        case AST_LITERAL:
            return evaluate_literal(interpreter, (AstLiteral*)expr);
        case AST_VARIABLE:
            return evaluate_variable(interpreter, (AstVariable*)expr);
        case AST_FUNCTION_CALL:
            interpret_function_call(interpreter, (AstFunctionCall*)expr);
            return 0;
        default:
            fprintf(stderr, "Type d'expression non géré: %d\n", expr->type);
            return 0;
    }
}

int evaluate_binary_expr(Interpreter* interpreter, AstBinaryExpr* bin_expr) {
    if (!interpreter || !bin_expr) return 0;

    int left = evaluate_expression(interpreter, bin_expr->left);
    int right = evaluate_expression(interpreter, bin_expr->right);

    switch (bin_expr->operator) {
        case TOKEN_PLUS:
            return left + right;
        case TOKEN_MINUS:
            return left - right;
        case TOKEN_ASTERISK:
            return left * right;
        case TOKEN_SLASH:
            if (right == 0) {
                fprintf(stderr, "Erreur: division par zéro\n");
                return 0;
            }
            return left / right;
        case TOKEN_EQUAL:
            return left == right;
        case TOKEN_BANG_EQUAL:
            return left != right;
        case TOKEN_LESS:
            return left < right;
        case TOKEN_LESS_EQUAL:
            return left <= right;
        case TOKEN_GREATER:
            return left > right;
        case TOKEN_GREATER_EQUAL:
            return left >= right;
        default:
            fprintf(stderr, "Opérateur binaire non géré: %d\n", bin_expr->operator);
            return 0;
    }
}

int evaluate_unary_expr(Interpreter* interpreter, AstUnaryExpr* un_expr) {
    if (!interpreter || !un_expr) return 0;

    int operand = evaluate_expression(interpreter, un_expr->operand);

    switch (un_expr->operator) {
        case TOKEN_MINUS:
            return -operand;
        case TOKEN_BANG:
            return !operand;
        default:
            fprintf(stderr, "Opérateur unaire non géré: %d\n", un_expr->operator);
            return 0;
    }
}

int evaluate_literal(Interpreter* interpreter, AstLiteral* lit) {
    if (!interpreter || !lit) return 0;

    switch (lit->literal_type) {
        case TOKEN_NUMBER:
            return lit->value.int_value;
        case TOKEN_VRAI:
            return 1;
        case TOKEN_FAUX:
            return 0;
        case TOKEN_STRING:
            return strlen(lit->value.string_value);
        default:
            return 0;
    }
}

int evaluate_variable(Interpreter* interpreter, AstVariable* var) {
    if (!interpreter || !var) return 0;

    InterpreterState* state = *(InterpreterState**)interpreter;

    Variable* variable = get_variable(state->current_env, var->name);

    if (variable) {
        if (variable->type == TOKEN_NUMBER) {
            return variable->value.int_value;
        } else if (variable->type == TOKEN_VRAI) {
            return 1;
        } else if (variable->type == TOKEN_FAUX) {
            return 0;
        }
    } else {
        fprintf(stderr, "Variable non définie: %s\n", var->name);
    }

    return 0;
}

void native_ecrire(const char* message) {
    printf("%s\n", message);
}
