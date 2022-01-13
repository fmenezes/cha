#include <llvm/IR/Value.h>

#include "nodes.h"

llvm::Value *ni::NInteger::codegen() const
{
    return NULL;
}

llvm::Value *ni::NUnaryOperation::codegen() const
{
    return NULL;
}

llvm::Value *ni::NBinaryOperation::codegen() const
{
    return NULL;
}

llvm::Value *ni::NVariableDeclaration::codegen() const
{
    return NULL;
}

llvm::Value *ni::NVariableAssignment::codegen() const
{
    return NULL;
}

llvm::Value *ni::NVariableLookup::codegen() const
{
    return NULL;
}

llvm::Value *ni::NStatementList::codegen() const
{
    return NULL;
}

llvm::Value *ni::NProgram::codegen() const
{
    return NULL;
}
