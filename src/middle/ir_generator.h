#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "../frontend/ast/ast.h"
#include <stdbool.h>

typedef enum {
    IR_ASSIGN,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    IR_NOT,
    IR_EQ,
    IR_NEQ,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    IR_AND,
    IR_OR,
    IR_GOTO,
    IR_IF_GOTO,
    IR_LABEL,
    IR_CALL,
    IR_RETURN,
    IR_PARAM,
    IR_FUNC_BEGIN,
    IR_FUNC_END,
    IR_ARRAY_ACCESS,
    IR_ARRAY_ASSIGN
} IrOpType;

typedef struct IrInstruction {
    IrOpType op;
    char* result;
    char* arg1;
    char* arg2;
    char* label;
    int line;
    struct IrInstruction* next;
} IrInstruction;

typedef struct IrFunction {
    char* name;
    char** params;
    int param_count;
    IrInstruction* instructions;
    struct IrFunction* next;
} IrFunction;

typedef struct IRProgram {
    IrFunction* functions;
    IrInstruction* global_instructions;
    int temp_counter;
    int label_counter;
} IRProgram;

IRProgram* generate_ir(AstNode* ast);
char* generate_ir_from_node(IRProgram* program, AstNode* node, char* result_var);
void ir_print_program(IRProgram* program);
void ir_free_program(IRProgram* program);

char* new_temp(IRProgram* program);
char* new_label(IRProgram* program);
void emit_instruction(IRProgram* program, IrInstruction* instruction);
IrInstruction* create_instruction(IrOpType op);

#endif //IR_GENERATOR_H
