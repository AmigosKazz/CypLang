#ifndef LLVM_EMITTER_H
#define LLVM_EMITTER_H

#include "../middle/ir_generator.h"

// Walk the IR program and produce an LLVM IR module.
// - If `output_path` is NULL, prints to stdout (preceded by "=== LLVM IR ===\n").
// - Otherwise writes the module to `output_path` (no banner, no stdout noise).
// Returns 0 on success, non-zero on error.
int emit_llvm(IRProgram* program, const char* module_name, const char* output_path);

#endif // LLVM_EMITTER_H
