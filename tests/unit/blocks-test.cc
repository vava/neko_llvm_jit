#include "llvm/neko_function.h"
#include "llvm/neko_basic_block.h"
#include "llvm/blocks.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

class BlocksTest : public testing::Test {
	neko_code_container make_code_container() const {
		neko_code_container result;

		result.insert(std::make_pair(0, std::make_pair((ptr_val)Jump, 70)));
		result.insert(std::make_pair(10, std::make_pair((ptr_val)Add, 0)));
		result.insert(std::make_pair(20, std::make_pair((ptr_val)Sub, 0)));
		result.insert(std::make_pair(30, std::make_pair((ptr_val)JumpIf, 50)));
		result.insert(std::make_pair(40, std::make_pair((ptr_val)Pop, 0)));
		result.insert(std::make_pair(50, std::make_pair((ptr_val)Mult, 0)));
		result.insert(std::make_pair(60, std::make_pair((ptr_val)JumpIfNot, 30)));
		result.insert(std::make_pair(70, std::make_pair((ptr_val)AccBuiltin, 100)));
		result.insert(std::make_pair(80, std::make_pair((ptr_val)AccNull, 0)));
		result.insert(std::make_pair(90, std::make_pair((ptr_val)AccStack, 0)));

		return result;
	}
protected:
	BlocksTest() : code_container(make_code_container())
				 , main_chunk(0, &code_container, 0, 100)
				 , function(main_chunk, "name")
	{}

	neko_code_container code_container;
	NekoCodeChunk main_chunk;
	neko::Function function;
};


TEST_F(BlocksTest, Constructor) {
	llvm::LLVMContext & ctx = llvm::getGlobalContext();

	llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
													  false);
	llvm::Function * llvm_func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage);
	llvm::BasicBlock * retBlock = llvm::BasicBlock::Create(ctx, "return", llvm_func);

	Blocks blocks(function, retBlock, llvm_func);

	Block * first = blocks.first();
	EXPECT_NE((NekoBlock *)0, dynamic_cast<NekoBlock *>(first));
	EXPECT_EQ(true, first->hasBeenCompiled());
	EXPECT_EQ(false, first->hasStack());
	EXPECT_EQ(0, first->getStack());
	EXPECT_EQ((ptr_val)0u, first->getNekoBlock()->getId());
	EXPECT_NE((Block *)0, first->getNext());

	Block * second = first->getNext();
	EXPECT_NE((NekoBlock *)0, dynamic_cast<NekoBlock *>(second));
	EXPECT_EQ(false, second->hasBeenCompiled());
	EXPECT_EQ(false, second->hasStack());
	EXPECT_EQ(0, second->getStack());
	EXPECT_EQ((ptr_val)30u, second->getNekoBlock()->getId());
	EXPECT_NE((Block *)0, second->getNext());

	Block * third = second->getNext();
	EXPECT_NE((NekoBlock *)0, dynamic_cast<NekoBlock *>(third));
	EXPECT_EQ(false, third->hasBeenCompiled());
	EXPECT_EQ(false, third->hasStack());
	EXPECT_EQ(0, third->getStack());
	EXPECT_EQ((ptr_val)40u, third->getNekoBlock()->getId());
	EXPECT_NE((Block *)0, third->getNext());

	Block * forth = third->getNext();
	EXPECT_NE((NekoBlock *)0, dynamic_cast<NekoBlock *>(forth));
	EXPECT_EQ(false, forth->hasBeenCompiled());
	EXPECT_EQ(false, forth->hasStack());
	EXPECT_EQ(0, forth->getStack());
	EXPECT_EQ((ptr_val)50u, forth->getNekoBlock()->getId());
	EXPECT_NE((Block *)0, forth->getNext());

	Block * fifth = forth->getNext();
	EXPECT_NE((NekoBlock *)0, dynamic_cast<NekoBlock *>(fifth));
	EXPECT_EQ(false, fifth->hasBeenCompiled());
	EXPECT_EQ(false, fifth->hasStack());
	EXPECT_EQ(0, fifth->getStack());
	EXPECT_EQ((ptr_val)70u, fifth->getNekoBlock()->getId());
	EXPECT_NE((Block *)0, fifth->getNext());

	Block * sixth = fifth->getNext();
	EXPECT_NE((NativeBlock *)0, dynamic_cast<NativeBlock *>(sixth));
	EXPECT_EQ(true, sixth->hasBeenCompiled());
	EXPECT_EQ(true, sixth->hasStack());
	EXPECT_EQ(0, sixth->getStack());
	EXPECT_EQ((neko::BasicBlock *)0, sixth->getNekoBlock());
	EXPECT_EQ((Block *)0, sixth->getNext());
	EXPECT_EQ(retBlock, sixth->getLLVMBlock());

	EXPECT_EQ(first, blocks.getNextToCompile());
	EXPECT_EQ((Block *)0, blocks.getNextToCompile());

	EXPECT_EQ(first, blocks.getById(0));
	EXPECT_EQ(second, blocks.getById(30));
	EXPECT_EQ(third, blocks.getById(40));
	EXPECT_EQ(forth, blocks.getById(50));
	EXPECT_EQ(fifth, blocks.getById(70));

	blocks.addToCompilationQueue(second);
	EXPECT_EQ(second, blocks.getNextToCompile());
	EXPECT_EQ((Block *)0, blocks.getNextToCompile());

	blocks.addToCompilationQueue(second);
	EXPECT_EQ((Block *)0, blocks.getNextToCompile());
}
