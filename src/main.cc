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
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <iostream>
#include <sstream>
#include <string>

#include "codegen/renderer.h"
#include "parsing/tree.h"

// Forward declarations
extern "C" double putchard(double X);
extern "C" double printd(double X);

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    IRRenderer *renderer = new IRRenderer();

    // Register external functions with JIT using SymbolMap
    auto &es = renderer->engine->getExecutionSession();
    auto &jd = renderer->engine->getMainJITDylib();
    
    llvm::orc::SymbolMap symbolMap;
    auto putchardAddr = reinterpret_cast<llvm::JITTargetAddress>(putchard);
    auto printdAddr = reinterpret_cast<llvm::JITTargetAddress>(printd);
    symbolMap[es.intern("putchard")] = llvm::JITEvaluatedSymbol(putchardAddr, llvm::JITSymbolFlags::Callable);
    symbolMap[es.intern("printd")] = llvm::JITEvaluatedSymbol(printdAddr, llvm::JITSymbolFlags::Callable);
    
    if (auto err = jd.define(llvm::orc::absoluteSymbols(symbolMap))) {
        llvm::errs() << "Failed to register external symbols: " << err << "\n";
        exit(1);
    }

    STree *tree = new STree();

    std::string input;
    fprintf(stderr, "ready> ");
    while (std::getline(std::cin, input, ';')) {
        // Trim whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r"));
        input.erase(input.find_last_not_of(" \t\n\r") + 1);

        // Skip empty input
        if (input.empty()) {
            continue;
        }

        // Check for quit or exit commands (case-insensitive)
        std::string lower_input = input;
        std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
        if (lower_input == "quit" || lower_input == "exit") {
            fprintf(stderr, "Goodbye!\n");
            break;
        }

        std::istringstream iss(input);

        tree->parse(iss);
        if ( tree->root != 0 ) {
            llvm::Value *value = tree->root->codegen(renderer);
            if ( value != 0 ) {
                llvm::Function *func = llvm::dyn_cast<llvm::Function>(value);
                if ( func != 0 && func->getName() == "__anon_expr" ) {
                    // Add the module to the JIT
                    auto tsm = llvm::orc::ThreadSafeModule(
                        std::move(renderer->module),
                        std::move(renderer->context)
                    );
                    if (auto err = renderer->engine->addIRModule(std::move(tsm))) {
                        llvm::handleAllErrors(std::move(err), [](const llvm::ErrorInfoBase &E) {
                            llvm::errs() << "Failed to add module: " << E.message() << "\n";
                        });
                        continue;
                    }

                    // Lookup the function
                    auto sym = renderer->engine->lookup("__anon_expr");
                    if (!sym) {
                        llvm::handleAllErrors(sym.takeError(), [](const llvm::ErrorInfoBase &E) {
                            llvm::errs() << "Failed to lookup function: " << E.message() << "\n";
                        });
                        continue;
                    }

                    void *func_ptr = reinterpret_cast<void*>(sym->getAddress());
                    double (*func_pointer)() = (double(*)())(intptr_t)func_ptr;
                    fprintf(stderr, "Evaluated to: %f\n", func_pointer());

                    // Create a new module for next iteration
                    renderer->context = std::make_unique<llvm::LLVMContext>();
                    renderer->module = std::make_unique<llvm::Module>("my cool jit", *renderer->context);
                    renderer->builder = std::make_unique<llvm::IRBuilder<>>(*renderer->context);
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
