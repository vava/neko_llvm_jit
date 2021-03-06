#include "stack.h"

#include <stdio.h>
#include <algorithm>

Stack::Stack(llvm::BasicBlock * entryBB_)
  : entryBB(entryBB_)
  , entryBuilder(entryBB)
  , h(entryBB->getContext())
{
}

Stack::~Stack() {
}

void Stack::push(llvm::IRBuilder<> & builder, llvm::Value * acc) {
	stack.push_back(entryBuilder.CreateAlloca(h.int_t(), 0));
	store(builder, 0, acc);
}

void Stack::pop(int_val how_many) {
	how_many = std::min(how_many, (int_val)stack.size());
	assert(stack.size() >= (size_t)how_many);
	stack.erase(stack.end() - how_many, stack.end());
}

llvm::AllocaInst * Stack::get(int_val index) {
	assert(stack.size() > (size_t)index);
	return *(stack.end() - index - 1);
}

llvm::Value * Stack::load(llvm::IRBuilder<> & builder, int_val index) {
	return builder.CreateLoad(get(index));
}

void Stack::store(llvm::IRBuilder<> & builder, int_val index, llvm::Value * value) {
	builder.CreateStore(value, get(index));
}

LockedStack Stack::lockStack(llvm::IRBuilder<> & builder) {
	return LockedStack(*this, builder);
}
