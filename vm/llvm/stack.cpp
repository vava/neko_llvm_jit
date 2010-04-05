#include "stack.h"

#include <stdio.h>

Stack::Stack(llvm::LLVMContext & ctx_): ctx(ctx_) {
}

Stack::~Stack() {
}

void Stack::InsertInit(Builder * builder_) {
	builder = builder_;

	llvm::IntegerType const * const intType = llvm::Type::getInt32Ty(ctx);
	llvm::ArrayType const * const arrayType = llvm::ArrayType::get(intType, 0);

	idx = builder->CreateAlloca(intType, 0, "stack_index");
	builder->CreateStore(llvm::ConstantInt::get(intType, 0), idx);

	stack = builder->CreateAlloca(arrayType, 0, "stack");
	//not sure we need it
	//builder.CreateStore(llvm::ConstantArray::get(arrayType, std::vector<llvm::Constant *>()), stack);
}

void Stack::InsertPush(llvm::Value * value) {
}

void Stack::InsertPop(llvm::Value * how_many_to_skip) {
}

void Stack::InsertLoad(llvm::Value * acc, llvm::Value * index) {
}

void Stack::InsertStore(llvm::Value * acc, llvm::Value * index) {
}
