#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "../frontend/ast/ast.h"

typedef struct IRProgram IrProgram;
typedef struct AstNode AstNode;

static int temp_counter = 0;

IrProgram* generate_ir(AstNode* ast);
char* generate_ir_from_node(IRProgram* program, AstNode* node, char* result_var);
void ir_print_program(IRProgram* program);
void ir_free_program(IRProgram* program);

#endif //IR_GENERATOR_H
