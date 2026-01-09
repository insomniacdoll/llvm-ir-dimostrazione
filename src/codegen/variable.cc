#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"

#include "ast/variable.h"
#include "renderer.h"
#include "errors.h"


llvm::Value *
VariableNode::codegen(IRRenderer *renderer) {
    llvm::Value *val = renderer->get_named_value(name);
    if ( !val ) {
        return ErrorV("Unknown variable name");
    }

    llvm::AllocaInst *alloca = llvm::cast<llvm::AllocaInst>(val);
    return renderer->builder->CreateLoad(alloca->getAllocatedType(), val, name.c_str());
}
