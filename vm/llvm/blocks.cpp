#include "blocks.h"

#include <sstream>

namespace {
	std::string intToString(ptr_val id) {
		std::stringstream bb_name;
		bb_name << id;
		return bb_name.str();
	}
}


void Block::copyOrCheckStack(Stack const &) {}
bool Block::hasBeenCompiled() const { return true; }
void Block::setCompiled() {}
bool Block::hasStack() const { return true; }
Stack * Block::getStack() { return 0; }
neko::BasicBlock const * Block::getNekoBlock() const {return 0;}
llvm::BasicBlock * Block::getLLVMBlock() {return 0;}

NekoBlock::NekoBlock(neko::BasicBlock const * neko_bb_, llvm::Function * function_, Block * next)
	: Block(next)
	, neko_bb(neko_bb_)
	, function(function_)
	, stack(0)
	, beenCompiled(false)
	, llvm_bb(0)
{
}

NekoBlock::~NekoBlock() {
	delete stack;
}

llvm::BasicBlock * NekoBlock::getLLVMBlock() {
	if (!llvm_bb) {
		llvm_bb = llvm::BasicBlock::Create(function->getContext(),
										   intToString(neko_bb->getId()),
										   function);
	}
	return llvm_bb;
}

Blocks::Blocks(neko::Function const & neko_function, llvm::BasicBlock * returnBlock, llvm::Function * llvm_function)
{
	blocks.push_back(new NativeBlock(returnBlock, 0));
	for (neko::Function::const_reverse_iterator it = neko_function.rbegin();
		 it != neko_function.rend();
		 ++it)
		{
			Block * next = blocks.back();
			Block * new_block = new NekoBlock(&*it, llvm_function, next);
			blocks.push_back(new_block);
			id2block.insert(std::make_pair(it->getId(), new_block));
		}

	//make sure compilation list has entry block
	addToCompilationQueue(first());
}

Blocks::~Blocks() {
	for (std::vector<Block *>::iterator it = blocks.begin(); it != blocks.end(); ++it) {
		delete *it;
	}
}

Block * Blocks::getById(ptr_val id) const {
	Blocks::id2block_type::const_iterator it = id2block.find(id);
	return (it == id2block.end()) ? 0 : it->second;
}

Block * Blocks::getNextToCompile() {
	if (blocks_to_compile.empty()) {
		return 0;
	} else {
		Block * block = blocks_to_compile.front();
		blocks_to_compile.pop_front();
		return block;
	}
}

void Blocks::addToCompilationQueue(Block * block) {
	if (block && !block->hasBeenCompiled()) {
		block->setCompiled();
		blocks_to_compile.push_back(block);
	}
}
