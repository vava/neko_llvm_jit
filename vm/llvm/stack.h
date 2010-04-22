#include "common.h"
#include "helper.h"

#include "llvm/LLVMContext.h"

class Stack {
public:
	Stack(llvm::LLVMContext & ctx);
	~Stack();

	void init(Builder * builder);

	void push(llvm::Value * value);
	void pop(int how_many);

	llvm::Value * load(int index);
	void store(int index, llvm::Value * value);

private:
	llvm::AllocaInst * get(int index);

	llvm::LLVMContext & ctx;
	Helper h;

	std::vector<llvm::AllocaInst *> stack;

	//This class does not own anything
	Builder * builder;
};
