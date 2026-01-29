#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"

#include "ast/binary.h"
#include "ast/variable.h"
#include "ast/var.h"
#include "renderer.h"
#include "errors.h"

using ::llvm::Value;
using ::llvm::Type;


Value *
BinaryNode::codegen(IRRenderer *renderer) {
    if ( op == '=' ) {
        VariableNode *lhse = dynamic_cast<VariableNode*>(lhs);
        if ( ! lhse ) {
            return ErrorV("destination of '=' must be a variable");
        }

        Value *val = rhs->codegen(renderer);
        if ( val == 0 ) { return 0; }

        Value *variable = renderer->get_named_value(lhse->getName());
        if ( variable == 0 ) {
            return ErrorV("Unknown variable name");
        }

        renderer->builder->CreateStore(val, variable);

        return val;
    }

    Value *lhs_value = lhs->codegen(renderer);
    Value *rhs_value = rhs->codegen(renderer);

    if (lhs_value == 0 || rhs_value == 0 ) { return 0; }

    Type *llvm_double_type = Type::getDoubleTy(renderer->module->getContext());

    switch (op) {
    case '+': return renderer->builder->CreateFAdd(lhs_value, rhs_value, "addtmp");
    case '-': return renderer->builder->CreateFSub(lhs_value, rhs_value, "subtmp");
    case '*': return renderer->builder->CreateFMul(lhs_value, rhs_value, "multmp");
    case '<':
        lhs_value = renderer->builder->CreateFCmpULT(lhs_value, rhs_value, "cmptmp");
        return renderer->builder->CreateUIToFP(lhs_value,
                                                llvm_double_type,
                                                "booltmp");
    case '>':
        rhs_value = renderer->builder->CreateFCmpULT(rhs_value, lhs_value, "cmptmp");
        return renderer->builder->CreateUIToFP(rhs_value,
                                                llvm_double_type,
                                                "booltmp");
    default: break;
    }

    return ErrorV("Unknown binary operator!");
}
