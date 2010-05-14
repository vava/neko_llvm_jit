#include "common.h"
#include "helper.h"

#include "llvm/LLVMContext.h"

class Stack {
public:
	Stack(llvm::BasicBlock * entryBB);
	~Stack();

	void push(llvm::IRBuilder<> & builder, llvm::Value * value);
	void pop(int how_many);

	llvm::Value * load(llvm::IRBuilder<> & builder, int index);
	void store(llvm::IRBuilder<> & builder, int index, llvm::Value * value);

private:
	llvm::AllocaInst * get(int index);

	llvm::BasicBlock * entryBB;
	llvm::IRBuilder<> entryBuilder;
	Helper h;

	std::vector<llvm::AllocaInst *> stack;
};
