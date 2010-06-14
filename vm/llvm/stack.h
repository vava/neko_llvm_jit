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

	void trap_push(std::pair<llvm::BasicBlock *, llvm::AllocaInst *> const & trap) {
		traps.push_back(trap);
	}

	std::pair<llvm::BasicBlock *, llvm::AllocaInst *> trap_back() const {
		return traps.back();
	}

	bool trap_empty() const { return traps.empty(); }

	void trap_pop() {
		traps.pop_back();
	}

	llvm::Value * load(llvm::IRBuilder<> & builder, int_val index);
	void store(llvm::IRBuilder<> & builder, int_val index, llvm::Value * value);

	LockedStack lockStack(llvm::IRBuilder<> & builder);

	int_val size() const { return stack.size(); }
	bool empty() const { return stack.empty(); }

	bool operator==(Stack const & rhs) const {
		return entryBB == rhs.entryBB
			&& stack == rhs.stack
			&& traps == rhs.traps;
	}
private:
	llvm::AllocaInst * get(int_val index);

	llvm::BasicBlock * entryBB;
	llvm::IRBuilder<> entryBuilder;
	Helper h;

	std::vector<llvm::AllocaInst *> stack;
	std::vector<std::pair<llvm::BasicBlock *, llvm::AllocaInst *> > traps;
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

	void trap_push(std::pair<llvm::BasicBlock *, llvm::AllocaInst *> const & trap) {
		stack.trap_push(trap);
	}

	std::pair<llvm::BasicBlock *, llvm::AllocaInst *> trap_back() const {
		return stack.trap_back();
	}

	bool trap_empty() const { return stack.trap_empty(); }

	void trap_pop() { stack.trap_pop(); }

	llvm::Value * load(int_val index) {
		return stack.load(builder, index);
	}

	void store(int_val index, llvm::Value * value) {
		stack.store(builder, index, value);
	}

	int_val size() const { return stack.size(); }
	bool empty() const { return stack.empty(); }

	Stack & unlock() const { return stack; }

private:
	Stack & stack;
	llvm::IRBuilder<> & builder;
};
