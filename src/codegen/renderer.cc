#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <string>
#include <cstdlib>

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
    auto initResult = jit_builder.create();
    if (auto err = initResult.takeError()) {
        llvm::errs() << "Could not create LLJIT: " << err << "\n";
        exit(1);
    }

    engine = std::move(initResult.get());
}

IRRenderer::IRRenderer(const IRRenderer &other)
    : IRRenderer(llvm::CloneModule(*other.module).release()) {}

IRRenderer::IRRenderer(Module *module)
    : context(std::make_unique<LLVMContext>()),
      module(std::unique_ptr<Module>(module)),
      builder(std::make_unique<IRBuilder<>>(*context)) {

    auto jit_builder = llvm::orc::LLJITBuilder();
    auto initResult = jit_builder.create();
    if (auto err = initResult.takeError()) {
        llvm::errs() << "Could not create LLJIT: " << err << "\n";
        exit(1);
    }

    engine = std::move(initResult.get());
}

IRRenderer::IRRenderer(IRRenderer &&other) {
    context = std::move(other.context);
    module = std::move(other.module);
    engine = std::move(other.engine);
    builder = std::move(other.builder);
}

IRRenderer &
IRRenderer::operator =(IRRenderer other) {
    std::swap(context, other.context);
    std::swap(module, other.module);
    std::swap(engine, other.engine);
    std::swap(builder, other.builder);
    return *this;
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
    return named_values[name];
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
