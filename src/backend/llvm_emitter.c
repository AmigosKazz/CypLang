#include "llvm_emitter.h"

#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>

int emit_llvm(IRProgram* program, const char* module_name) {
    (void)program; // not consumed yet — Phase 2.3 will walk IR instructions

    LLVMContextRef ctx = LLVMContextCreate();
    LLVMModuleRef module = LLVMModuleCreateWithNameInContext(module_name, ctx);

    char* ir_text = LLVMPrintModuleToString(module);
    printf("=== LLVM IR ===\n%s", ir_text);
    LLVMDisposeMessage(ir_text);

    LLVMDisposeModule(module);
    LLVMContextDispose(ctx);
    return 0;
}
