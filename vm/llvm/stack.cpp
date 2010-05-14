#include "stack.h"

#include <stdio.h>

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

void Stack::pop(int how_many) {
	stack.erase(stack.end() - how_many, stack.end());
}

llvm::AllocaInst * Stack::get(int index) {
	return *(stack.end() - index - 1);
}

llvm::Value * Stack::load(llvm::IRBuilder<> & builder, int index) {
	return builder.CreateLoad(get(index));
}

void Stack::store(llvm::IRBuilder<> & builder, int index, llvm::Value * value) {
	builder.CreateStore(value, get(index));
}
