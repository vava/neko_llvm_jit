#include "common.h"
#include "helper.h"

#include "llvm/LLVMContext.h"

class Stack {
public:
	Stack(llvm::LLVMContext & ctx);
	~Stack();

	void InsertInit(Builder * builder);

	void InsertPush(llvm::Value * value);
	void InsertPop(llvm::Value * how_many_to_skip);
	void InsertPop(int how_many_to_skip) { InsertPop(h.int_n(how_many_to_skip)); }
	//That's how strange looking stack it is in neko
	llvm::Value * Load(llvm::Value * acc, llvm::Value * index);
	llvm::Value * Load(llvm::Value * acc, int index) { return Load(acc, h.int_n(index)); }
	void InsertStore(llvm::Value * acc, llvm::Value * index);
	void InsertStore(llvm::Value * acc, int index)  { InsertStore(acc, h.int_n(index)); }

private:
	llvm::LLVMContext & ctx;
	Helper h;

	//This class does not own anything
	Builder * builder;

	llvm::Value * stack;
	llvm::Value * idx;
};
