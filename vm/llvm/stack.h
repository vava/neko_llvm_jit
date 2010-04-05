#include "common.h"

#include "llvm/LLVMContext.h"

class Stack {
public:
	Stack(llvm::LLVMContext & ctx);
	~Stack();

	void InsertInit(Builder * builder);

	void InsertPush(llvm::Value * value);
	void InsertPop(llvm::Value * how_many_to_skip);
	//That's how strange looking stack it is in neko
	void InsertLoad(llvm::Value * acc, llvm::Value * index);
	void InsertStore(llvm::Value * acc, llvm::Value * index);

private:
	llvm::LLVMContext & ctx;

	//This class does not own anything
	Builder * builder;

	llvm::Value * stack;
	llvm::Value * idx;
};
