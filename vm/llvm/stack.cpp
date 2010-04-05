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

namespace {
	llvm::Value * getArrayIndex(Builder * builder, llvm::Value * array, llvm::Value * idx, std::string const & array_name = "array") {
		llvm::IntegerType const * const intType = llvm::Type::getInt32Ty(llvm::getGlobalContext());

		llvm::Value * indexies[2] = {idx, llvm::ConstantInt::get(intType, 0)};
		return builder->CreateGEP(array, indexies, indexies + 2, array_name + "[idx]");
	}
}

void Stack::InsertPush(llvm::Value * acc) {
	llvm::IntegerType const * const intType = llvm::Type::getInt32Ty(ctx);

	llvm::Value * idx_value = builder->CreateLoad(idx, "stack_index_tmp");
	builder->CreateStore(builder->CreateLoad(acc, "acc_tmp"), getArrayIndex(builder, stack, idx, "stack"));
	builder->CreateStore(builder->CreateAdd(idx_value, llvm::ConstantInt::get(intType, 1)), idx);
}

void Stack::InsertPop(llvm::Value * how_many_to_skip) {
}

void Stack::InsertLoad(llvm::Value * acc, llvm::Value * index) {
}

void Stack::InsertStore(llvm::Value * acc, llvm::Value * index) {
}
