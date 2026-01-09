#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <iostream>
#include <sstream>
#include <string>

#include "codegen/renderer.h"
#include "parsing/tree.h"


int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    IRRenderer *renderer = new IRRenderer();

    STree *tree = new STree();

    std::string input;
    fprintf(stderr, "ready> ");
    while (std::getline(std::cin, input, ';')) {
        std::istringstream iss(input);

        tree->parse(iss);
        if ( tree->root != 0 ) {
            if ( llvm::Function *func = static_cast<llvm::Function*>(tree->root->codegen(renderer)) ) {
                if ( func->getName() == "" ) {
                    // Add the module to the JIT
                    auto tsm = llvm::orc::ThreadSafeModule(
                        std::move(renderer->module),
                        std::make_unique<llvm::LLVMContext>()
                    );
                    if (auto err = renderer->engine->addIRModule(std::move(tsm))) {
                        llvm::errs() << "Failed to add module: " << err << "\n";
                        continue;
                    }

                    // Lookup the function
                    auto sym = renderer->engine->lookup("__anon_expr");
                    if (!sym) {
                        llvm::errs() << "Failed to lookup function: " << sym.takeError() << "\n";
                        continue;
                    }

                    void *func_ptr = reinterpret_cast<void*>(sym->getValue());
                    double (*func_pointer)() = (double(*)())(intptr_t)func_ptr;
                    fprintf(stderr, "Evaluated to: %f\n", func_pointer());

                    // Create a new module for next iteration
                    renderer->module = std::make_unique<llvm::Module>("my cool jit", renderer->llvm_context());
                    renderer->builder = std::make_unique<llvm::IRBuilder<>>(renderer->llvm_context());
                }
            }
        }
        fprintf(stderr, "ready> ");
    }

    renderer->module->print(llvm::errs(), nullptr);

    return 0;
}

/// putchard - putchar that takes a double and returns 0.
extern "C"
double putchard(double X) {
  putchar((char)X);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C"
double printd(double X) {
  printf("%f\n", X);
  return 0;
}
