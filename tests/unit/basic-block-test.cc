#include "llvm/basic_block.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

using namespace testing;

class BasicBlockTest : public testing::Test {
	neko_code_container make_code_container() const {
		neko_code_container result;

		result.insert(std::make_pair(0, std::make_pair((unsigned int)Jump, 100)));
		result.insert(std::make_pair(9, std::make_pair((unsigned int)Add, 0)));
		result.insert(std::make_pair(10, std::make_pair((unsigned int)Sub, 0)));
		result.insert(std::make_pair(12, std::make_pair((unsigned int)Push, 0)));
		result.insert(std::make_pair(30, std::make_pair((unsigned int)Pop, 0)));
		result.insert(std::make_pair(40, std::make_pair((unsigned int)Mult, 0)));
		result.insert(std::make_pair(49, std::make_pair((unsigned int)AccInt, 10)));
		result.insert(std::make_pair(50, std::make_pair((unsigned int)AccBuiltin, 100)));
		result.insert(std::make_pair(51, std::make_pair((unsigned int)AccNull, 0)));
		result.insert(std::make_pair(60, std::make_pair((unsigned int)AccStack, 0)));

		return result;
	}
protected:
	BasicBlockTest() : code_container(make_code_container())
						, chunk(&code_container, 10, 50)
						, bb(chunk)
	{}

	neko_code_container code_container;
	NekoCodeChunk chunk;
	BasicBlock bb;
};


TEST_F(BasicBlockTest, ExposeAllCode) {
	EXPECT_THAT(bb, ElementsAre(Pair(10, Pair(Sub, 0)),
								Pair(12, Pair(Push, 0)),
								Pair(30, Pair(Pop, 0)),
								Pair(40, Pair(Mult, 0)),
								Pair(49, Pair(AccInt, 10))));
}
