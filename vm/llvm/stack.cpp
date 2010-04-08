#include "stack.h"

#include <stdio.h>

Stack::Stack(llvm::LLVMContext & ctx_): ctx(ctx_), h(ctx_) {
}

Stack::~Stack() {
}

void Stack::InsertInit(Builder * builder_) {
	builder = builder_;

	llvm::ArrayType const * const arrayType = llvm::ArrayType::get(h.int_t(), 0);

	idx = builder->CreateAlloca(h.int_t(), 0, "stack_index");
	builder->CreateStore(h.int_0(), idx);

	stack = builder->CreateAlloca(arrayType, 0, "stack");
	//not sure we need it
	//builder.CreateStore(llvm::ConstantArray::get(arrayType, std::vector<llvm::Constant *>()), stack);
}

void Stack::InsertPush(llvm::Value * acc) {
	llvm::Value * idx_value = builder->CreateLoad(idx, "stack_index_tmp");
	builder->CreateStore(builder->CreateLoad(acc, "acc_tmp"), h.getArrayIndex(*builder, stack, idx, "stack"));
	builder->CreateStore(builder->CreateAdd(idx_value, h.int_1()), idx);
}

void Stack::InsertPop(llvm::Value * how_many_to_skip) {
	builder->CreateStore(
		builder->CreateSub(builder->CreateLoad(idx, "stack_index_tmp"),
						   how_many_to_skip),
		idx);
}

llvm::Value * Stack::Load(llvm::Value * acc, llvm::Value * index) {
	llvm::Value * idx_value = builder->CreateLoad(idx, "stack_index_tmp");
	return builder->CreateLoad(
		h.getArrayIndex(*builder,
						stack,
						builder->CreateSub(builder->CreateSub(idx_value, index), h.int_1()),
						"stack"));
}

void Stack::InsertStore(llvm::Value * acc, llvm::Value * index) {
}
