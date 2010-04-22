#include "stack.h"

#include <stdio.h>

Stack::Stack(llvm::LLVMContext & ctx_): ctx(ctx_), h(ctx_) {
}

Stack::~Stack() {
}

void Stack::init(Builder * builder_) {
	builder = builder_;
}

void Stack::push(llvm::Value * acc) {
	stack.push_back(builder->CreateAlloca(h.int_t(), 0));
	store(0, acc);
}

void Stack::pop(int how_many) {
	stack.erase(stack.end() - how_many, stack.end());
}

llvm::AllocaInst * Stack::get(int index) {
	return *(stack.end() - index - 1);
}

llvm::Value * Stack::load(int index) {
	return builder->CreateLoad(get(index));
}

void Stack::store(int index, llvm::Value * value) {
	builder->CreateStore(value, get(index));
}
