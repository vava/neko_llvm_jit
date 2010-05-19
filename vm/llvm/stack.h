#include "common.h"
#include "helper.h"

#include "llvm/LLVMContext.h"
#include "llvm/Support/IRBuilder.h"

class Stack {
public:
	Stack(llvm::BasicBlock * entryBB);
	~Stack();

	void push(llvm::IRBuilder<> & builder, llvm::Value * value);
	void pop(int_val how_many);

	llvm::Value * load(llvm::IRBuilder<> & builder, int_val index);
	void store(llvm::IRBuilder<> & builder, int_val index, llvm::Value * value);

private:
	llvm::AllocaInst * get(int_val index);

	llvm::BasicBlock * entryBB;
	llvm::IRBuilder<> entryBuilder;
	Helper h;

	std::vector<llvm::AllocaInst *> stack;
};
