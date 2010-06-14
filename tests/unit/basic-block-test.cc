#include "llvm/neko_basic_block.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

using namespace testing;
using namespace neko;

class BasicBlockTest : public testing::Test {
	neko_code_container make_code_container() const {
		neko_code_container result;

		result.insert(std::make_pair(0, std::make_pair((ptr_val)Jump, 100)));
		result.insert(std::make_pair(9, std::make_pair((ptr_val)Add, 0)));
		result.insert(std::make_pair(10, std::make_pair((ptr_val)Sub, 0)));
		result.insert(std::make_pair(12, std::make_pair((ptr_val)Push, 0)));
		result.insert(std::make_pair(30, std::make_pair((ptr_val)Pop, 0)));
		result.insert(std::make_pair(40, std::make_pair((ptr_val)Mult, 0)));
		result.insert(std::make_pair(49, std::make_pair((ptr_val)AccInt, 10)));
		result.insert(std::make_pair(50, std::make_pair((ptr_val)AccBuiltin, 100)));
		result.insert(std::make_pair(51, std::make_pair((ptr_val)AccNull, 0)));
		result.insert(std::make_pair(60, std::make_pair((ptr_val)AccStack, 0)));

		return result;
	}
protected:
	BasicBlockTest() : code_container(make_code_container())
					 , chunk(0, &code_container, 10, 50)
					 , bb(chunk)
	{}

	neko_code_container code_container;
	NekoCodeChunk chunk;
	BasicBlock bb;
};


TEST_F(BasicBlockTest, ExposeAllCode) {
	EXPECT_EQ(10u, bb.getId());

	EXPECT_THAT(bb, ElementsAre(Pair(10, Pair(Sub, 0)),
								Pair(12, Pair(Push, 0)),
								Pair(30, Pair(Pop, 0)),
								Pair(40, Pair(Mult, 0)),
								Pair(49, Pair(AccInt, 10))));

	EXPECT_THAT(BasicBlock(chunk.getSubChunk(10, 10)), ElementsAre());
}
