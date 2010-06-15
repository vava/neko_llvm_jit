#include "neko_basic_block.h"
#include "neko_function.h"

#include "stack.h"

#include "llvm/Function.h"

#include <deque>
#include <iostream>

class Block {
public:
	virtual void copyOrCheckStack(Stack const &) = 0;
	virtual bool hasBeenCompiled() const = 0;
	virtual void setCompiled() = 0;
	virtual bool hasStack() const = 0;
	virtual Stack * getStack() = 0;
	virtual neko::BasicBlock const * getNekoBlock() const = 0;
	virtual llvm::BasicBlock * getLLVMBlock() = 0;

	Block(Block * next_):next(next_) {}
	virtual ~Block() {};

	Block * getNext() const { return next; }
private:
	Block * next;
};

class NativeBlock : public Block {
public:
	NativeBlock(llvm::BasicBlock * bb, Block * next): Block(next), llvm_bb(bb) {}

	virtual void copyOrCheckStack(Stack const &) {}
	virtual bool hasBeenCompiled() const { return true; }
	virtual void setCompiled() {}
	virtual bool hasStack() const { return true; }
	virtual Stack * getStack() { return 0;}
	virtual neko::BasicBlock const * getNekoBlock() const { return 0; }
	virtual llvm::BasicBlock * getLLVMBlock() {
		return llvm_bb;
	}
private:
	llvm::BasicBlock * llvm_bb;
};

class NekoBlock : public Block {
public:
	NekoBlock(neko::BasicBlock const * neko_bb_, llvm::Function * function, Block * next);
	virtual ~NekoBlock();

	virtual void copyOrCheckStack(Stack const & newStack) {
		if (!stack) {
			stack = new Stack(newStack);
		} else {
			//Simple assert doesn't cut it anymore, sometimes this block
			// will be already compiled which affects the stack of course
			// Spaghetti stack will be able to manage those checks
			//assert(*stack == newStack);
		}
	}

	virtual bool hasBeenCompiled() const { return beenCompiled; }
	virtual void setCompiled() { beenCompiled = true; }
	virtual bool hasStack() const { return stack != 0; }
	virtual Stack * getStack() { return stack; }
	virtual neko::BasicBlock const * getNekoBlock() const { return neko_bb; }
	virtual llvm::BasicBlock * getLLVMBlock();
private:
	neko::BasicBlock const * neko_bb;
	llvm::Function * function;
	Stack * stack;
	bool beenCompiled;
	llvm::BasicBlock * llvm_bb;
};

class Blocks {
public:
	Blocks(neko::Function const & neko_function, llvm::BasicBlock * returnBlock, llvm::Function * llvm_function);
	~Blocks();

	Block * getById(ptr_val id) const;
	Block * getNextToCompile();
	void addToCompilationQueue(Block * block);
	Block * first() const {
		if (blocks.empty()) {
			return 0;
		} else {
			return blocks.back(); //they are reversed
		}
	}

private:
	typedef std::map<ptr_val, Block *> id2block_type;
	id2block_type id2block;
	std::vector<Block *> blocks; //blocks in _reverse_ order
	std::deque<Block *> blocks_to_compile;
};
