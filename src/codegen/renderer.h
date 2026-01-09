#pragma once

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <memory>
#include <string>

using ::std::map;
using ::std::string;
using ::std::unique_ptr;

using ::llvm::AllocaInst;
using ::llvm::Function;
using ::llvm::LLVMContext;
using ::llvm::Module;
using ::llvm::IRBuilder;
using ::llvm::orc::LLJIT;


class IRRenderer {
    map<string, AllocaInst*> named_values;

    IRRenderer(const IRRenderer &other);
    IRRenderer(Module *module);
    IRRenderer(IRRenderer &&other);

    IRRenderer &operator =(IRRenderer other);

public:
    IRRenderer();
    ~IRRenderer();

    unique_ptr<LLVMContext> context;
    unique_ptr<Module> module;
    unique_ptr<LLJIT> engine;
    unique_ptr<IRBuilder<> > builder;

    LLVMContext &llvm_context();

    AllocaInst *get_named_value(const std::string &name);
    void set_named_value(const std::string &name, AllocaInst* value);
    void clear_named_value(const std::string &name);
    void clear_all_named_values();

    AllocaInst *create_entry_block_alloca(Function *func, const std::string &name);
};
