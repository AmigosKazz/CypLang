#include "llvm_emitter.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>

// ---------- symbol table: maps an IR name ("t0", "x", ...) to its alloca + type ----------

typedef struct Symbol {
    char* name;
    LLVMValueRef alloca;
    LLVMTypeRef type;   // i32 or double (Phase 2.4)
    struct Symbol* next;
} Symbol;

typedef struct {
    LLVMContextRef ctx;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMTypeRef i32_type;
    LLVMTypeRef double_type;
    LLVMValueRef current_function;
    Symbol* symbols;
} EmitCtx;

// Bundles a value with its LLVM type, used when resolving IR args.
typedef struct {
    LLVMValueRef value;
    LLVMTypeRef type;
} TypedValue;

static Symbol* sym_lookup(EmitCtx* ec, const char* name) {
    for (Symbol* s = ec->symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

// Get-or-create the alloca for `name`. On creation, uses `type` (i32 or double).
// On lookup of an existing symbol, `type` is ignored (alloca is already typed).
static Symbol* sym_get_or_create(EmitCtx* ec, const char* name, LLVMTypeRef type) {
    Symbol* s = sym_lookup(ec, name);
    if (s) return s;

    // Place allocas in the entry block so mem2reg can promote them later.
    LLVMBasicBlockRef entry = LLVMGetEntryBasicBlock(ec->current_function);
    LLVMValueRef first = LLVMGetFirstInstruction(entry);
    LLVMBuilderRef tmp = LLVMCreateBuilderInContext(ec->ctx);
    if (first) {
        LLVMPositionBuilderBefore(tmp, first);
    } else {
        LLVMPositionBuilderAtEnd(tmp, entry);
    }
    LLVMValueRef alloca = LLVMBuildAlloca(tmp, type, name);
    LLVMDisposeBuilder(tmp);

    s = malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->alloca = alloca;
    s->type = type;
    s->next = ec->symbols;
    ec->symbols = s;
    return s;
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

// Returns true if `s` looks like an integer literal (optional leading '-', then digits, no dot).
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

// Returns true if `s` looks like a float literal (digits with exactly one '.').
static int is_float_literal(const char* s) {
    if (!s || !*s) return 0;
    if (*s == '-') s++;
    int has_digit = 0, has_dot = 0;
    while (*s) {
        if (*s == '.') {
            if (has_dot) return 0;
            has_dot = 1;
        } else if (isdigit((unsigned char)*s)) {
            has_digit = 1;
        } else {
            return 0;
        }
        s++;
    }
    return has_digit && has_dot;
}

// Resolve an IR arg into a typed LLVM value.
// - "3"      → i32 constant
// - "3.14"   → double constant
// - "t0", "x" → load from the symbol's alloca, returning its declared type
static TypedValue arg_to_typed(EmitCtx* ec, const char* arg) {
    TypedValue tv;
    if (is_int_literal(arg)) {
        tv.type = ec->i32_type;
        tv.value = LLVMConstInt(tv.type, (unsigned long long)strtol(arg, NULL, 10), /*SignExtend=*/1);
        return tv;
    }
    if (is_float_literal(arg)) {
        tv.type = ec->double_type;
        tv.value = LLVMConstReal(tv.type, strtod(arg, NULL));
        return tv;
    }
    // Symbol: must exist by now (IR is generated top-down).
    Symbol* s = sym_lookup(ec, arg);
    if (!s) {
        // Defensive fallback — should not happen for valid IR.
        fprintf(stderr, "warning: unknown IR symbol '%s' — defaulting to i32 0\n", arg);
        tv.type = ec->i32_type;
        tv.value = LLVMConstInt(tv.type, 0, 0);
        return tv;
    }
    tv.type = s->type;
    tv.value = LLVMBuildLoad2(ec->builder, s->type, s->alloca, arg);
    return tv;
}

// ---------- per-instruction emission ----------

// Emit a single IR instruction. Unknown ops are silently skipped (handled in later phases).
// IR_RETURN is handled here only as a fallback; functions handle it in emit_function below.
static void emit_one(EmitCtx* ec, IrInstruction* inst) {
    switch (inst->op) {
        case IR_ASSIGN: {
            TypedValue v = arg_to_typed(ec, inst->arg1);
            Symbol* s = sym_get_or_create(ec, inst->result, v.type);
            LLVMBuildStore(ec->builder, v.value, s->alloca);
            break;
        }
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV: {
            TypedValue l = arg_to_typed(ec, inst->arg1);
            TypedValue r = arg_to_typed(ec, inst->arg2);
            // Mixed-type arithmetic (int+double) is not yet handled — Phase 2.4.1.
            // We use l's type as the result type; if r differs, clang will reject.
            int is_fp = (l.type == ec->double_type);
            LLVMValueRef res;
            switch (inst->op) {
                case IR_ADD: res = is_fp ? LLVMBuildFAdd(ec->builder, l.value, r.value, inst->result)
                                         : LLVMBuildAdd (ec->builder, l.value, r.value, inst->result); break;
                case IR_SUB: res = is_fp ? LLVMBuildFSub(ec->builder, l.value, r.value, inst->result)
                                         : LLVMBuildSub (ec->builder, l.value, r.value, inst->result); break;
                case IR_MUL: res = is_fp ? LLVMBuildFMul(ec->builder, l.value, r.value, inst->result)
                                         : LLVMBuildMul (ec->builder, l.value, r.value, inst->result); break;
                case IR_DIV: res = is_fp ? LLVMBuildFDiv(ec->builder, l.value, r.value, inst->result)
                                         : LLVMBuildSDiv(ec->builder, l.value, r.value, inst->result); break;
                default:     res = NULL; // unreachable
            }
            Symbol* s = sym_get_or_create(ec, inst->result, l.type);
            LLVMBuildStore(ec->builder, res, s->alloca);
            break;
        }
        case IR_RETURN: {
            if (inst->arg1) {
                TypedValue v = arg_to_typed(ec, inst->arg1);
                LLVMBuildRet(ec->builder, v.value);
            } else {
                LLVMBuildRet(ec->builder, LLVMConstInt(ec->i32_type, 0, 0));
            }
            break;
        }
        default:
            // IR_NEG / IR_NOT / comparisons / control flow / calls:
            // not yet supported, will be added in later jalons.
            break;
    }
}

// ---------- function emission ----------

// thread proper types from the AST/IR (cyplang IR is currently untyped).
static void emit_function(EmitCtx* ec, IrFunction* func) {
    // Build the signature: (i32, i32, ...) -> i32
    LLVMTypeRef* param_types = NULL;
    if (func->param_count > 0) {
        param_types = malloc(func->param_count * sizeof(LLVMTypeRef));
        for (int i = 0; i < func->param_count; i++) param_types[i] = ec->i32_type;
    }
    LLVMTypeRef func_type = LLVMFunctionType(ec->i32_type, param_types,
                                             (unsigned)func->param_count, /*IsVarArg=*/0);
    free(param_types);

    LLVMValueRef llvm_func = LLVMAddFunction(ec->module, func->name, func_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ec->ctx, llvm_func, "entry");
    LLVMPositionBuilderAtEnd(ec->builder, entry);

    // Save outer scope (we're about to enter a fresh symbol table for this function).
    Symbol* saved_symbols = ec->symbols;
    LLVMValueRef saved_function = ec->current_function;
    ec->symbols = NULL;
    ec->current_function = llvm_func;

    // For each parameter: alloca + store the incoming LLVM param into it.
    // This lets the body load/store params just like locals (mem2reg will clean it up).
    for (int i = 0; i < func->param_count; i++) {
        const char* pname = func->params[i];
        Symbol* s = sym_get_or_create(ec, pname, ec->i32_type);
        LLVMValueRef param_val = LLVMGetParam(llvm_func, (unsigned)i);
        LLVMSetValueName2(param_val, pname, strlen(pname));
        LLVMBuildStore(ec->builder, param_val, s->alloca);
    }

    // Walk the function's IR. IR_FUNC_BEGIN / IR_PARAM / IR_FUNC_END are pure
    // bookkeeping at the IR level and have no LLVM counterpart here.
    for (IrInstruction* inst = func->instructions; inst; inst = inst->next) {
        if (inst->op == IR_FUNC_BEGIN || inst->op == IR_PARAM || inst->op == IR_FUNC_END) {
            continue;
        }
        emit_one(ec, inst);
    }

    // Fallback: every basic block must end with a terminator.
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(ec->builder);
    if (!LLVMGetBasicBlockTerminator(current_block)) {
        LLVMBuildRet(ec->builder, LLVMConstInt(ec->i32_type, 0, 0));
    }

    // Restore the outer scope.
    sym_free_all(ec);
    ec->symbols = saved_symbols;
    ec->current_function = saved_function;
}

// ---------- public entry point ----------

int emit_llvm(IRProgram* program, const char* module_name, const char* output_path) {
    EmitCtx ec = {0};
    ec.ctx = LLVMContextCreate();
    ec.module = LLVMModuleCreateWithNameInContext(module_name, ec.ctx);
    ec.builder = LLVMCreateBuilderInContext(ec.ctx);
    ec.i32_type = LLVMInt32TypeInContext(ec.ctx);
    ec.double_type = LLVMDoubleTypeInContext(ec.ctx);

    // Emit user-defined functions first, so global code in `main` can call them.
    for (IrFunction* f = program ? program->functions : NULL; f; f = f->next) {
        emit_function(&ec, f);
    }

    // Wrap global IR instructions in `main` (i32).
    // TODO(phase 2.5.1): rename this if the user defines their own `main`.
    LLVMTypeRef main_type = LLVMFunctionType(ec.i32_type, NULL, 0, /*IsVarArg=*/0);
    ec.current_function = LLVMAddFunction(ec.module, "main", main_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ec.ctx, ec.current_function, "entry");
    LLVMPositionBuilderAtEnd(ec.builder, entry);

    for (IrInstruction* inst = program ? program->global_instructions : NULL; inst; inst = inst->next) {
        emit_one(&ec, inst);
    }

    // Always terminate with `ret i32 0` so the module verifies.
    LLVMBuildRet(ec.builder, LLVMConstInt(ec.i32_type, 0, 0));

    int rc = 0;
    if (output_path) {
        char* err = NULL;
        if (LLVMPrintModuleToFile(ec.module, output_path, &err) != 0) {
            fprintf(stderr, "Failed to write LLVM IR to %s: %s\n",
                    output_path, err ? err : "(unknown error)");
            if (err) LLVMDisposeMessage(err);
            rc = 1;
        }
    } else {
        char* ir_text = LLVMPrintModuleToString(ec.module);
        printf("=== LLVM IR ===\n%s", ir_text);
        LLVMDisposeMessage(ir_text);
    }

    sym_free_all(&ec);
    LLVMDisposeBuilder(ec.builder);
    LLVMDisposeModule(ec.module);
    LLVMContextDispose(ec.ctx);
    return rc;
}
