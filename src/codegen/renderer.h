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
    map<string, int> function_arg_counts;  // Store function name -> argument count

    // The module is bound to this renderer's LLVMContext, so the copy
    // constructor cannot be implemented correctly (the previous one cloned the
    // module but then handed it to a *different* context). Copying is not used
    // anywhere, so forbid it outright.
    IRRenderer(const IRRenderer &other) = delete;
    IRRenderer &operator =(const IRRenderer &other) = delete;
    IRRenderer(IRRenderer &&other);

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
    void declare_external_function(const std::string &name);
    Function *get_function(const std::string &name);
    void add_function_type(const std::string &name, llvm::FunctionType *type);
    void reset_function_types();
};
