#include "llvm/neko_function.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

using namespace testing;
using namespace neko;

class FunctionTest : public testing::Test {
	neko_code_container make_code_container() const {
		neko_code_container result;

		result.insert(std::make_pair(0, std::make_pair((unsigned int)Jump, 70)));
		result.insert(std::make_pair(10, std::make_pair((unsigned int)Add, 0)));
		result.insert(std::make_pair(20, std::make_pair((unsigned int)Sub, 0)));
		result.insert(std::make_pair(30, std::make_pair((unsigned int)JumpIf, 50)));
		result.insert(std::make_pair(40, std::make_pair((unsigned int)Pop, 0)));
		result.insert(std::make_pair(50, std::make_pair((unsigned int)Mult, 0)));
		result.insert(std::make_pair(60, std::make_pair((unsigned int)JumpIfNot, 30)));
		result.insert(std::make_pair(70, std::make_pair((unsigned int)AccBuiltin, 100)));
		result.insert(std::make_pair(80, std::make_pair((unsigned int)AccNull, 0)));
		result.insert(std::make_pair(90, std::make_pair((unsigned int)AccStack, 0)));

		return result;
	}
protected:
	FunctionTest() : code_container(make_code_container())
				   , main_chunk(&code_container, 0, 100)
	{}

	neko_code_container code_container;
	NekoCodeChunk main_chunk;
};


TEST_F(FunctionTest, ExposeAllCode) {
	EXPECT_EQ("name", Function(main_chunk, "name").getName());

	EXPECT_THAT(Function(main_chunk, "name"),
				ElementsAre(
					ElementsAre(
						Pair(0, Pair(Jump, 70)),
						Pair(10, Pair(Add, 0)),
						Pair(20, Pair(Sub, 0))),
					ElementsAre(
						Pair(30, Pair(JumpIf, 50)),
						Pair(40, Pair(Pop, 0))),
					ElementsAre(
						Pair(50, Pair(Mult, 0)),
						Pair(60, Pair(JumpIfNot, 30))),
					ElementsAre(
						Pair(70, Pair(AccBuiltin, 100)),
						Pair(80, Pair(AccNull, 0)),
						Pair(90, Pair(AccStack, 0)))));
}

TEST_F(FunctionTest, WorksWithoutJumps) {
	EXPECT_THAT(Function(main_chunk.getSubChunk(70, 100), "name"),
				ElementsAre(
					ElementsAre(
						Pair(70, Pair(AccBuiltin, 100)),
						Pair(80, Pair(AccNull, 0)),
						Pair(90, Pair(AccStack, 0)))));
}

TEST_F(FunctionTest, WorksWithEmpty) {
	EXPECT_THAT(Function(main_chunk.getSubChunk(0, 0), "name"),
				ElementsAre());
}
