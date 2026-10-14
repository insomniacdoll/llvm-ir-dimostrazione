#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/TargetSelect.h"

#include <cstdlib>
#include <string>

#include "renderer.h"

using ::llvm::AllocaInst;
using ::llvm::Function;
using ::llvm::IRBuilder;
using ::llvm::LLVMContext;
using ::llvm::Module;
using ::llvm::Type;
using ::llvm::orc::LLJIT;


IRRenderer::IRRenderer() {
    context = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("my cool jit", *context);
    builder = std::make_unique<IRBuilder<>>(*context);

    auto jit_builder = llvm::orc::LLJITBuilder();
    auto init_result = jit_builder.create();
    // Check the Expected before taking the error out of it: LLVM documents an
    // Expected as destruct-only once takeError() has been called on it, so the
    // old "if (auto err = init_result.takeError())" followed by get() on the
    // success path was relying on undefined behaviour.
    if (!init_result) {
        llvm::errs() << "Could not create LLJIT: "
                     << llvm::toString(init_result.takeError()) << "\n";
        exit(1);
    }

    engine = std::move(*init_result);
}

IRRenderer::IRRenderer(IRRenderer &&other) {
    context = std::move(other.context);
    module = std::move(other.module);
    engine = std::move(other.engine);
    builder = std::move(other.builder);
    // The name bindings are part of the renderer's state: leaving these out
    // made a moved-to renderer look healthy while having lost every variable.
    named_values = std::move(other.named_values);
    function_arg_counts = std::move(other.function_arg_counts);
}

IRRenderer::~IRRenderer() {
    builder.reset();
    engine.reset();
    module.reset();
    context.reset();
}

llvm::LLVMContext &
IRRenderer::llvm_context() { return *context; }

llvm::AllocaInst *
IRRenderer::get_named_value (const std::string &name){
    // Use find() rather than operator[]: the latter would silently insert a
    // null entry for every unknown name, growing the map on every lookup miss.
    auto it = named_values.find(name);
    return it == named_values.end() ? nullptr : it->second;
}

void
IRRenderer::set_named_value(const std::string &name, llvm::AllocaInst *value) {
    named_values[name] = value;
}

void
IRRenderer::clear_named_value(const std::string &name) {
    named_values.erase(name);
}

void
IRRenderer::clear_all_named_values() {
    named_values.clear();
}

AllocaInst *
IRRenderer::create_entry_block_alloca(Function *func, const std::string &name) {
    IRBuilder<> tmp_builder(&func->getEntryBlock(),
                            func->getEntryBlock().begin());

    return tmp_builder.CreateAlloca(Type::getDoubleTy(module->getContext()),
                                    0,
                                    name.c_str());
}

void
IRRenderer::declare_external_function(const std::string &name) {
    // Create function type for a function that takes a double and returns a double
    std::vector<Type*> param_types = {Type::getDoubleTy(module->getContext())};
    llvm::FunctionType *func_type = llvm::FunctionType::get(
        Type::getDoubleTy(module->getContext()),
        param_types,
        false
    );

    // Create or get the function with ExternalLinkage (external function).
    // Function::Create would silently rename a second attempt to "name.1", so
    // reuse an existing declaration instead.
    if (module->getFunction(name) == nullptr) {
        Function::Create(
            func_type,
            Function::ExternalLinkage,
            name,
            module.get()
        );
    }

    // Record the signature: once this module is handed to the JIT and replaced
    // by a fresh one, get_function() can only rebuild a declaration for a
    // JIT-resident symbol if it knows the parameter count. Without this,
    // external functions were uncallable until the user typed an "extern".
    add_function_type(name, func_type);
}

Function *
IRRenderer::get_function(const std::string &name) {
    // First, try to find the function in the current module
    Function *func = module->getFunction(name);
    if (func != nullptr) {
        return func;
    }

    // If not found in the module, try to find it in the JIT
    auto sym = engine->lookup(name);
    if (sym) {
        // Found in JIT, create a declaration in the current module
        // Use the stored argument count if available
        auto it = function_arg_counts.find(name);
        if (it == function_arg_counts.end()) {
            // If we don't have argument count information, we can't create the declaration
            return nullptr;
        }

        int arg_count = it->second;

        // Create the function type in the current context
        // Assume all parameters are doubles and return type is double
        std::vector<Type*> param_types(arg_count, Type::getDoubleTy(module->getContext()));
        llvm::FunctionType *func_type = llvm::FunctionType::get(
            Type::getDoubleTy(module->getContext()),
            param_types,
            false
        );

        func = Function::Create(
            func_type,
            Function::ExternalLinkage,
            name,
            module.get()
        );
        return func;
    }

    return nullptr;
}

void
IRRenderer::add_function_type(const std::string &name, llvm::FunctionType *type) {
    function_arg_counts[name] = type->getNumParams();
}

void
IRRenderer::reset_function_types() {
    // Collect function argument counts from the current module
    for (auto &func : *module) {
        if (!func.isDeclaration()) {
            function_arg_counts[func.getName().str()] = func.arg_size();
        }
    }
}
