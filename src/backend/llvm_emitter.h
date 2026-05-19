#ifndef LLVM_EMITTER_H
#define LLVM_EMITTER_H

#include "../middle/ir_generator.h"

// Walk the IR program and print an LLVM IR module to stdout.
// For now: produces an empty module (Phase 2.2 skeleton).
// Returns 0 on success, non-zero on error.
int emit_llvm(IRProgram* program, const char* module_name);

#endif // LLVM_EMITTER_H
