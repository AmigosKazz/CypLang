#include "ir_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static IrFunction* current_function = NULL;

IRProgram* init_ir_program() {
    IRProgram* program = (IRProgram*)malloc(sizeof(IRProgram));
    if (!program) {
        fprintf(stderr, "Memory allocation failed for IR program\n");
        return NULL;
    }
    program->functions = NULL;
    program->global_instructions = NULL;
    program->temp_counter = 0;
    program->label_counter = 0;
    return program;
}

char* new_temp(IRProgram* program) {
    char* temp = (char*)malloc(20);
    sprintf(temp, "t%d", program->temp_counter++);
    return temp;
}

char* new_label(IRProgram* program) {
    char* label = (char*)malloc(20);
    sprintf(label, "L%d", program->label_counter++);
    return label;
}

IrInstruction* create_instruction(IrOpType op) {
    IrInstruction* inst = (IrInstruction*)malloc(sizeof(IrInstruction));
    if (!inst) {
        fprintf(stderr, "Memory allocation failed for IR instruction\n");
        return NULL;
    }
    inst->op = op;
    inst->result = NULL;
    inst->arg1 = NULL;
    inst->arg2 = NULL;
    inst->label = NULL;
    inst->line = 0;
    inst->next = NULL;
    return inst;
}

void emit_instruction(IRProgram* program, IrInstruction* instruction) {
    if (!instruction) return;

    if (current_function) {
        if (!current_function->instructions) {
            current_function->instructions = instruction;
        } else {
            IrInstruction* tail = current_function->instructions;
            while (tail->next) {
                tail = tail->next;
            }
            tail->next = instruction;
        }
    } else {
        if (!program->global_instructions) {
            program->global_instructions = instruction;
        } else {
            IrInstruction* tail = program->global_instructions;
            while (tail->next) {
                tail = tail->next;
            }
            tail->next = instruction;
        }
    }
}

char* generate_ir_from_literal(IRProgram* program, AstLiteral* literal) {
    char* result = new_temp(program);
    IrInstruction* inst = create_instruction(IR_ASSIGN);
    inst->result = result;

    char* value = (char*)malloc(100);
    switch (literal->literal_type) {
        case TOKEN_NUMBER:
            sprintf(value, "%d", literal->value.int_value);
            break;
        case TOKEN_REEL:
            sprintf(value, "%f", literal->value.float_value);
            break;
        case TOKEN_STRING:
            sprintf(value, "\"%s\"", literal->value.string_value);
            break;
        case TOKEN_CHARACTER:
            sprintf(value, "'%c'", literal->value.char_value);
            break;
        case TOKEN_VRAI:
            strcpy(value, "true");
            break;
        case TOKEN_FAUX:
            strcpy(value, "false");
            break;
        default:
            strcpy(value, "0");
    }
    inst->arg1 = value;
    emit_instruction(program, inst);
    return result;
}

char* generate_ir_from_variable(IRProgram* program, AstVariable* var) {
    return strdup(var->name);
}

char* generate_ir_from_binary_expr(IRProgram* program, AstBinaryExpr* expr) {
    char* left = generate_ir_from_node(program, expr->left, NULL);
    char* right = generate_ir_from_node(program, expr->right, NULL);
    char* result = new_temp(program);

    IrInstruction* inst = NULL;
    switch (expr->operator) {
        case TOKEN_PLUS:
            inst = create_instruction(IR_ADD);
            break;
        case TOKEN_MINUS:
            inst = create_instruction(IR_SUB);
            break;
        case TOKEN_ASTERISK:
            inst = create_instruction(IR_MUL);
            break;
        case TOKEN_SLASH:
        case TOKEN_DIV:
            inst = create_instruction(IR_DIV);
            break;
        case TOKEN_MOD:
            inst = create_instruction(IR_MOD);
            break;
        case TOKEN_EQUAL:
            inst = create_instruction(IR_EQ);
            break;
        case TOKEN_BANG_EQUAL:
            inst = create_instruction(IR_NEQ);
            break;
        case TOKEN_LESS:
            inst = create_instruction(IR_LT);
            break;
        case TOKEN_LESS_EQUAL:
            inst = create_instruction(IR_LE);
            break;
        case TOKEN_GREATER:
            inst = create_instruction(IR_GT);
            break;
        case TOKEN_GREATER_EQUAL:
            inst = create_instruction(IR_GE);
            break;
        case TOKEN_ET:
            inst = create_instruction(IR_AND);
            break;
        case TOKEN_OU:
            inst = create_instruction(IR_OR);
            break;
        default:
            fprintf(stderr, "Unknown binary operator\n");
            return NULL;
    }

    if (inst) {
        inst->result = result;
        inst->arg1 = left;
        inst->arg2 = right;
        emit_instruction(program, inst);
    }

    return result;
}

char* generate_ir_from_unary_expr(IRProgram* program, AstUnaryExpr* expr) {
    char* operand = generate_ir_from_node(program, expr->operand, NULL);
    char* result = new_temp(program);

    IrInstruction* inst = NULL;
    switch (expr->operator) {
        case TOKEN_MINUS:
            inst = create_instruction(IR_NEG);
            break;
        case TOKEN_NON:
        case TOKEN_BANG:
            inst = create_instruction(IR_NOT);
            break;
        default:
            fprintf(stderr, "Unknown unary operator\n");
            return NULL;
    }

    if (inst) {
        inst->result = result;
        inst->arg1 = operand;
        emit_instruction(program, inst);
    }

    return result;
}

char* generate_ir_from_assignment(IRProgram* program, AstAssignment* assign) {
    char* value = generate_ir_from_node(program, assign->value, NULL);

    if (assign->target->type == AST_ARRAY_ACCESS) {
        AstArrayAccess* array_access = (AstArrayAccess*)assign->target;
        char* array = generate_ir_from_node(program, array_access->array, NULL);
        char* index = generate_ir_from_node(program, array_access->index, NULL);

        IrInstruction* inst = create_instruction(IR_ARRAY_ASSIGN);
        inst->result = array;
        inst->arg1 = index;
        inst->arg2 = value;
        emit_instruction(program, inst);
        return value;
    } else {
        char* target = generate_ir_from_node(program, assign->target, NULL);
        IrInstruction* inst = create_instruction(IR_ASSIGN);
        inst->result = target;
        inst->arg1 = value;
        emit_instruction(program, inst);
        return target;
    }
}

void generate_ir_from_if_statement(IRProgram* program, AstIfStatement* if_stmt) {
    char* condition = generate_ir_from_node(program, if_stmt->condition, NULL);

    char* else_label = new_label(program);
    char* end_label = new_label(program);

    // if !cond goto else_label
    IrInstruction* if_inst = create_instruction(IR_IF_GOTO);
    if_inst->arg1 = condition;
    if_inst->label = strdup(else_label);
    emit_instruction(program, if_inst);

    // then branch
    generate_ir_from_node(program, if_stmt->then_branch, NULL);

    // goto end
    IrInstruction* goto_end = create_instruction(IR_GOTO);
    goto_end->label = strdup(end_label);
    emit_instruction(program, goto_end);

    // else_label:
    IrInstruction* else_label_inst = create_instruction(IR_LABEL);
    else_label_inst->label = else_label;
    emit_instruction(program, else_label_inst);

    // else branch (optional)
    if (if_stmt->else_branch) {
        generate_ir_from_node(program, if_stmt->else_branch, NULL);
    }

    // end_label:
    IrInstruction* end_label_inst = create_instruction(IR_LABEL);
    end_label_inst->label = end_label;
    emit_instruction(program, end_label_inst);
}

void generate_ir_from_while_statement(IRProgram* program, AstWhileStatement* while_stmt) {
    char* loop_start = new_label(program);
    char* loop_end = new_label(program);

    IrInstruction* start_label = create_instruction(IR_LABEL);
    start_label->label = strdup(loop_start);
    emit_instruction(program, start_label);

    char* condition = generate_ir_from_node(program, while_stmt->condition, NULL);

    IrInstruction* if_inst = create_instruction(IR_IF_GOTO);
    if_inst->arg1 = condition;
    if_inst->label = loop_end;
    emit_instruction(program, if_inst);

    generate_ir_from_node(program, while_stmt->body, NULL);

    IrInstruction* goto_start = create_instruction(IR_GOTO);
    goto_start->label = loop_start;
    emit_instruction(program, goto_start);

    IrInstruction* end_label = create_instruction(IR_LABEL);
    end_label->label = strdup(loop_end);
    emit_instruction(program, end_label);
}

void generate_ir_from_for_statement(IRProgram* program, AstForStatement* for_stmt) {
    if (for_stmt->init) {
        generate_ir_from_node(program, for_stmt->init, NULL);
    }

    char* loop_start = new_label(program);
    char* loop_end = new_label(program);

    IrInstruction* start_label = create_instruction(IR_LABEL);
    start_label->label = strdup(loop_start);
    emit_instruction(program, start_label);

    if (for_stmt->condition) {
        char* condition = generate_ir_from_node(program, for_stmt->condition, NULL);
        IrInstruction* if_inst = create_instruction(IR_IF_GOTO);
        if_inst->arg1 = condition;
        if_inst->label = loop_end;
        emit_instruction(program, if_inst);
    }

    generate_ir_from_node(program, for_stmt->body, NULL);

    if (for_stmt->update) {
        generate_ir_from_node(program, for_stmt->update, NULL);
    }

    IrInstruction* goto_start = create_instruction(IR_GOTO);
    goto_start->label = loop_start;
    emit_instruction(program, goto_start);

    IrInstruction* end_label = create_instruction(IR_LABEL);
    end_label->label = strdup(loop_end);
    emit_instruction(program, end_label);
}

void generate_ir_from_return_statement(IRProgram* program, AstReturnStatement* ret_stmt) {
    char* value = NULL;
    if (ret_stmt->value) {
        value = generate_ir_from_node(program, ret_stmt->value, NULL);
    }

    IrInstruction* inst = create_instruction(IR_RETURN);
    inst->arg1 = value;
    emit_instruction(program, inst);
}

char* generate_ir_from_function_call(IRProgram* program, AstFunctionCall* call) {
    char* result = new_temp(program);

    IrInstruction* inst = create_instruction(IR_CALL);
    inst->result = result;
    inst->arg1 = strdup(call->name);
    emit_instruction(program, inst);

    return result;
}

void generate_ir_from_block(IRProgram* program, AstBlock* block) {
    for (int i = 0; i < block->statement_count; i++) {
        generate_ir_from_node(program, block->statements[i], NULL);
    }
}

void generate_ir_from_function_decl(IRProgram* program, AstFunctionDeclaration* func_decl) {
    IrFunction* func = (IrFunction*)malloc(sizeof(IrFunction));
    func->name = strdup(func_decl->name);
    func->param_count = func_decl->parameter_count;
    func->params = NULL;
    func->instructions = NULL;
    func->next = NULL;

    if (func_decl->parameter_count > 0) {
        func->params = (char**)malloc(sizeof(char*) * func_decl->parameter_count);
        for (int i = 0; i < func_decl->parameter_count; i++) {
            AstParameter* param = (AstParameter*)func_decl->parameters[i];
            func->params[i] = strdup(param->name);
        }
    }

    if (!program->functions) {
        program->functions = func;
    } else {
        IrFunction* tail = program->functions;
        while (tail->next) {
            tail = tail->next;
        }
        tail->next = func;
    }

    current_function = func;

    IrInstruction* func_begin = create_instruction(IR_FUNC_BEGIN);
    func_begin->arg1 = strdup(func_decl->name);
    emit_instruction(program, func_begin);

    for (int i = 0; i < func_decl->parameter_count; i++) {
        AstParameter* param = (AstParameter*)func_decl->parameters[i];
        IrInstruction* param_inst = create_instruction(IR_PARAM);
        param_inst->arg1 = strdup(param->name);
        emit_instruction(program, param_inst);
    }

    if (func_decl->body) {
        generate_ir_from_node(program, func_decl->body, NULL);
    }

    IrInstruction* func_end = create_instruction(IR_FUNC_END);
    emit_instruction(program, func_end);

    current_function = NULL;
}

void generate_ir_from_variable_decl(IRProgram* program, AstVariableDeclaration* var_decl) {
    if (var_decl->initializer) {
        char* value = generate_ir_from_node(program, var_decl->initializer, NULL);
        IrInstruction* inst = create_instruction(IR_ASSIGN);
        inst->result = strdup(var_decl->name);
        inst->arg1 = value;
        emit_instruction(program, inst);
    }
}

char* generate_ir_from_array_access(IRProgram* program, AstArrayAccess* array_access) {
    char* array = generate_ir_from_node(program, array_access->array, NULL);
    char* index = generate_ir_from_node(program, array_access->index, NULL);
    char* result = new_temp(program);

    IrInstruction* inst = create_instruction(IR_ARRAY_ACCESS);
    inst->result = result;
    inst->arg1 = array;
    inst->arg2 = index;
    emit_instruction(program, inst);

    return result;
}

char* generate_ir_from_node(IRProgram* program, AstNode* node, char* result_var) {
    if (!node) return NULL;

    switch (node->type) {
        case AST_PROGRAM: {
            AstProgram* prog = (AstProgram*)node;
            for (int i = 0; i < prog->declaration_count; i++) {
                generate_ir_from_node(program, prog->declarations[i], NULL);
            }
            return NULL;
        }
        case AST_FUNCTION_DECL:
            generate_ir_from_function_decl(program, (AstFunctionDeclaration*)node);
            return NULL;
        case AST_BLOCK_DECL:
            generate_ir_from_block(program, (AstBlock*)node);
            return NULL;
        case AST_VARIABLE_DECL:
            generate_ir_from_variable_decl(program, (AstVariableDeclaration*)node);
            return NULL;
        case AST_ASSIGNMENT:
            return generate_ir_from_assignment(program, (AstAssignment*)node);
        case AST_BINARY_EXPR:
            return generate_ir_from_binary_expr(program, (AstBinaryExpr*)node);
        case AST_UNARY_EXPR:
            return generate_ir_from_unary_expr(program, (AstUnaryExpr*)node);
        case AST_IF_STATEMENT:
            generate_ir_from_if_statement(program, (AstIfStatement*)node);
            return NULL;
        case AST_WHILE_STATEMENT:
            generate_ir_from_while_statement(program, (AstWhileStatement*)node);
            return NULL;
        case AST_FOR_STATEMENT:
            generate_ir_from_for_statement(program, (AstForStatement*)node);
            return NULL;
        case AST_RETURN_STATEMENT:
            generate_ir_from_return_statement(program, (AstReturnStatement*)node);
            return NULL;
        case AST_FUNCTION_CALL:
            return generate_ir_from_function_call(program, (AstFunctionCall*)node);
        case AST_VARIABLE:
            return generate_ir_from_variable(program, (AstVariable*)node);
        case AST_LITERAL:
            return generate_ir_from_literal(program, (AstLiteral*)node);
        case AST_ARRAY_ACCESS:
            return generate_ir_from_array_access(program, (AstArrayAccess*)node);
        default:
            fprintf(stderr, "Unknown AST node type: %d\n", node->type);
            return NULL;
    }
}

IRProgram* generate_ir(AstNode* ast) {
    IRProgram* program = init_ir_program();
    if (!program) return NULL;

    generate_ir_from_node(program, ast, NULL);
    return program;
}

void print_instruction(IrInstruction* inst) {
    switch (inst->op) {
        case IR_ASSIGN:
            printf("    %s = %s\n", inst->result, inst->arg1);
            break;
        case IR_ADD:
            printf("    %s = %s + %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_SUB:
            printf("    %s = %s - %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_MUL:
            printf("    %s = %s * %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_DIV:
            printf("    %s = %s / %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_MOD:
            printf("    %s = %s %% %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_NEG:
            printf("    %s = -%s\n", inst->result, inst->arg1);
            break;
        case IR_NOT:
            printf("    %s = !%s\n", inst->result, inst->arg1);
            break;
        case IR_EQ:
            printf("    %s = %s == %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_NEQ:
            printf("    %s = %s != %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_LT:
            printf("    %s = %s < %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_LE:
            printf("    %s = %s <= %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_GT:
            printf("    %s = %s > %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_GE:
            printf("    %s = %s >= %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_AND:
            printf("    %s = %s && %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_OR:
            printf("    %s = %s || %s\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_GOTO:
            printf("    goto %s\n", inst->label);
            break;
        case IR_IF_GOTO:
            printf("    if !%s goto %s\n", inst->arg1, inst->label);
            break;
        case IR_LABEL:
            printf("%s:\n", inst->label);
            break;
        case IR_CALL:
            printf("    %s = call %s\n", inst->result, inst->arg1);
            break;
        case IR_RETURN:
            if (inst->arg1) {
                printf("    return %s\n", inst->arg1);
            } else {
                printf("    return\n");
            }
            break;
        case IR_PARAM:
            printf("    param %s\n", inst->arg1);
            break;
        case IR_FUNC_BEGIN:
            printf("function %s:\n", inst->arg1);
            break;
        case IR_FUNC_END:
            printf("end function\n\n");
            break;
        case IR_ARRAY_ACCESS:
            printf("    %s = %s[%s]\n", inst->result, inst->arg1, inst->arg2);
            break;
        case IR_ARRAY_ASSIGN:
            printf("    %s[%s] = %s\n", inst->result, inst->arg1, inst->arg2);
            break;
    }
}

void ir_print_program(IRProgram* program) {
    if (!program) return;

    printf("=== IR Program ===\n\n");

    if (program->global_instructions) {
        printf("Global Instructions:\n");
        IrInstruction* inst = program->global_instructions;
        while (inst) {
            print_instruction(inst);
            inst = inst->next;
        }
        printf("\n");
    }

    IrFunction* func = program->functions;
    while (func) {
        printf("Function: %s\n", func->name);
        if (func->param_count > 0) {
            printf("Parameters: ");
            for (int i = 0; i < func->param_count; i++) {
                printf("%s", func->params[i]);
                if (i < func->param_count - 1) printf(", ");
            }
            printf("\n");
        }

        IrInstruction* inst = func->instructions;
        while (inst) {
            print_instruction(inst);
            inst = inst->next;
        }
        func = func->next;
    }

    printf("=== End IR Program ===\n");
}

void free_instruction(IrInstruction* inst) {
    if (!inst) return;
    free(inst->result);
    free(inst->arg1);
    free(inst->arg2);
    free(inst->label);
    free(inst);
}

void free_instruction_list(IrInstruction* inst) {
    while (inst) {
        IrInstruction* next = inst->next;
        free_instruction(inst);
        inst = next;
    }
}

void free_function(IrFunction* func) {
    if (!func) return;
    free(func->name);
    if (func->params) {
        for (int i = 0; i < func->param_count; i++) {
            free(func->params[i]);
        }
        free(func->params);
    }
    free_instruction_list(func->instructions);
    free(func);
}

void ir_free_program(IRProgram* program) {
    if (!program) return;

    free_instruction_list(program->global_instructions);

    IrFunction* func = program->functions;
    while (func) {
        IrFunction* next = func->next;
        free_function(func);
        func = next;
    }

    free(program);
}