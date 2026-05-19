#include "llvm_emitter.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>

// ---------- symbol table: maps an IR name ("t0", "x", ...) to its alloca ----------

typedef struct Symbol {
    char* name;
    LLVMValueRef alloca;
    struct Symbol* next;
} Symbol;

typedef struct {
    LLVMContextRef ctx;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMTypeRef i32_type;
    LLVMValueRef current_function;
    Symbol* symbols;
} EmitCtx;

static Symbol* sym_lookup(EmitCtx* ec, const char* name) {
    for (Symbol* s = ec->symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

// Get the alloca for `name`, creating one (in the current function's entry block) if absent.
static LLVMValueRef sym_get_or_create(EmitCtx* ec, const char* name) {
    Symbol* s = sym_lookup(ec, name);
    if (s) return s->alloca;

    // Place allocas in the entry block so mem2reg can promote them later.
    LLVMBasicBlockRef entry = LLVMGetEntryBasicBlock(ec->current_function);
    LLVMValueRef first = LLVMGetFirstInstruction(entry);
    LLVMBuilderRef tmp = LLVMCreateBuilderInContext(ec->ctx);
    if (first) {
        LLVMPositionBuilderBefore(tmp, first);
    } else {
        LLVMPositionBuilderAtEnd(tmp, entry);
    }
    LLVMValueRef alloca = LLVMBuildAlloca(tmp, ec->i32_type, name);
    LLVMDisposeBuilder(tmp);

    s = malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->alloca = alloca;
    s->next = ec->symbols;
    ec->symbols = s;
    return alloca;
}

static void sym_free_all(EmitCtx* ec) {
    Symbol* s = ec->symbols;
    while (s) {
        Symbol* next = s->next;
        free(s->name);
        free(s);
        s = next;
    }
    ec->symbols = NULL;
}

// ---------- arg resolution: literal or loaded variable ----------

// Returns true if `s` looks like an integer literal (optional leading '-', then digits).
static int is_int_literal(const char* s) {
    if (!s || !*s) return 0;
    if (*s == '-') s++;
    if (!*s) return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

// Resolve an IR arg into an LLVMValueRef of i32.
// If the arg looks like an integer literal, emit a constant.
// Otherwise, treat it as a symbol name and load from its alloca.
static LLVMValueRef arg_to_value(EmitCtx* ec, const char* arg) {
    if (is_int_literal(arg)) {
        long v = strtol(arg, NULL, 10);
        return LLVMConstInt(ec->i32_type, (unsigned long long)v, /*SignExtend=*/1);
    }
    LLVMValueRef ptr = sym_get_or_create(ec, arg);
    return LLVMBuildLoad2(ec->builder, ec->i32_type, ptr, arg);
}

// ---------- per-instruction emission ----------

// Emit a single IR instruction. Unknown ops are silently skipped (handled in later phases).
static void emit_one(EmitCtx* ec, IrInstruction* inst) {
    switch (inst->op) {
        case IR_ASSIGN: {
            LLVMValueRef val = arg_to_value(ec, inst->arg1);
            LLVMValueRef ptr = sym_get_or_create(ec, inst->result);
            LLVMBuildStore(ec->builder, val, ptr);
            break;
        }
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV: {
            LLVMValueRef l = arg_to_value(ec, inst->arg1);
            LLVMValueRef r = arg_to_value(ec, inst->arg2);
            LLVMValueRef res;
            switch (inst->op) {
                case IR_ADD: res = LLVMBuildAdd(ec->builder, l, r, inst->result); break;
                case IR_SUB: res = LLVMBuildSub(ec->builder, l, r, inst->result); break;
                case IR_MUL: res = LLVMBuildMul(ec->builder, l, r, inst->result); break;
                case IR_DIV: res = LLVMBuildSDiv(ec->builder, l, r, inst->result); break;
                default:     res = NULL; // unreachable
            }
            LLVMValueRef ptr = sym_get_or_create(ec, inst->result);
            LLVMBuildStore(ec->builder, res, ptr);
            break;
        }
        default:
            // IR_NEG / IR_NOT / comparisons / control flow / calls / functions:
            // not yet supported in Phase 2.3, will be added in later jalons.
            break;
    }
}

// ---------- public entry point ----------

int emit_llvm(IRProgram* program, const char* module_name) {
    EmitCtx ec = {0};
    ec.ctx = LLVMContextCreate();
    ec.module = LLVMModuleCreateWithNameInContext(module_name, ec.ctx);
    ec.builder = LLVMCreateBuilderInContext(ec.ctx);
    ec.i32_type = LLVMInt32TypeInContext(ec.ctx);

    // Wrap global IR instructions in `main` (i32) so clang/lld can link it directly.
    // Phase 2.5 will need to rename this if the user defines their own `main`.
    LLVMTypeRef main_type = LLVMFunctionType(ec.i32_type, NULL, 0, /*IsVarArg=*/0);
    ec.current_function = LLVMAddFunction(ec.module, "main", main_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ec.ctx, ec.current_function, "entry");
    LLVMPositionBuilderAtEnd(ec.builder, entry);

    for (IrInstruction* inst = program ? program->global_instructions : NULL; inst; inst = inst->next) {
        emit_one(&ec, inst);
    }

    // Always terminate with `ret i32 0` so the module verifies.
    LLVMBuildRet(ec.builder, LLVMConstInt(ec.i32_type, 0, 0));

    char* ir_text = LLVMPrintModuleToString(ec.module);
    printf("=== LLVM IR ===\n%s", ir_text);
    LLVMDisposeMessage(ir_text);

    sym_free_all(&ec);
    LLVMDisposeBuilder(ec.builder);
    LLVMDisposeModule(ec.module);
    LLVMContextDispose(ec.ctx);
    return 0;
}
