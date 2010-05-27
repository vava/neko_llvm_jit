#pragma once

#include "common.h"
#include "helper.h"

#include "llvm/LLVMContext.h"
#include "llvm/Support/IRBuilder.h"

class LockedStack;

class Stack {
public:
	Stack(llvm::BasicBlock * entryBB);
	~Stack();

	void push(llvm::IRBuilder<> & builder, llvm::Value * value);
	void pop(int_val how_many);

	llvm::Value * load(llvm::IRBuilder<> & builder, int_val index);
	void store(llvm::IRBuilder<> & builder, int_val index, llvm::Value * value);

	LockedStack lockStack(llvm::IRBuilder<> & builder);

private:
	llvm::AllocaInst * get(int_val index);

	llvm::BasicBlock * entryBB;
	llvm::IRBuilder<> entryBuilder;
	Helper h;

	std::vector<llvm::AllocaInst *> stack;
};

class LockedStack {
public:
	LockedStack(Stack & stack_, llvm::IRBuilder<> & builder_)
		: stack(stack_)
		, builder(builder_)
	{}

	void push(llvm::Value * value) {
		stack.push(builder, value);
	}

	void pop(int_val how_many) { stack.pop(how_many); }

	llvm::Value * load(int_val index) {
		return stack.load(builder, index);
	}

	void store(int_val index, llvm::Value * value) {
		stack.store(builder, index, value);
	}
private:
	Stack & stack;
	llvm::IRBuilder<> & builder;
};
