#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"

#include "ast/for.h"
#include "renderer.h"

using ::llvm::AllocaInst;
using ::llvm::APFloat;
using ::llvm::BasicBlock;
using ::llvm::Constant;
using ::llvm::ConstantFP;
using ::llvm::Function;
using ::llvm::Value;
using ::llvm::Type;


Value *
ForNode::codegen(IRRenderer *renderer) {
    Function *func = renderer->builder->GetInsertBlock()->getParent();

    AllocaInst *alloca = renderer->create_entry_block_alloca(func, var_name);

    Value *start_value = start->codegen(renderer);
    if ( start_value == 0 ) { return 0; }

    renderer->builder->CreateStore(start_value, alloca);

    Value *step_value;
    if ( step ) {
        step_value = step->codegen(renderer);
        if ( step_value == 0 ) { return 0; }
    } else {
        step_value = ConstantFP::get(renderer->module->getContext(), APFloat(1.0));
    }

    BasicBlock *loop_block = BasicBlock::Create(renderer->module->getContext(), "loop", func);
    BasicBlock *after_block = BasicBlock::Create(renderer->module->getContext(), "afterloop", func);

    renderer->builder->CreateBr(loop_block);
    renderer->builder->SetInsertPoint(loop_block);

    AllocaInst *old_value = renderer->get_named_value(var_name);
    renderer->set_named_value(var_name, alloca);

    if ( body->codegen(renderer) == 0 ) { return 0; }

    Value *current_var = renderer->builder->CreateLoad(alloca->getAllocatedType(), alloca, var_name.c_str());
    Value *next_var = renderer->builder->CreateFAdd(current_var, step_value, "nextvar");
    renderer->builder->CreateStore(next_var, alloca);

    Value *end_condition = end->codegen(renderer);
    if ( end_condition == 0 ) { return 0; }

    Value *loop_condition = renderer->builder->CreateFCmpULT(
        next_var,
        end_condition,
        "loopcond"
    );

    renderer->builder->CreateCondBr(loop_condition, loop_block, after_block);
    renderer->builder->SetInsertPoint(after_block);

    if ( old_value ) {
        renderer->set_named_value(var_name, old_value);
    } else {
        renderer->clear_named_value(var_name);
    }

    return Constant::getNullValue(Type::getDoubleTy(renderer->module->getContext()));
}
