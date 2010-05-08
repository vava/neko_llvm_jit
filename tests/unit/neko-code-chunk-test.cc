#include "llvm/neko_code_chunk.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

using namespace testing;

class NekoCodeChunkTest : public testing::Test {
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
	NekoCodeChunkTest() : code_container(make_code_container())
						, chunk(&code_container, 10, 50)
						, over_chunk(&code_container, 0, 70)
	{}

	neko_code_container code_container;
	NekoCodeChunk chunk;
	NekoCodeChunk over_chunk;
};


TEST_F(NekoCodeChunkTest, ExposeAllCode) {
	EXPECT_THAT(chunk, ElementsAre(Pair(10, Pair(Sub, 0)),
								   Pair(12, Pair(Push, 0)),
								   Pair(30, Pair(Pop, 0)),
								   Pair(40, Pair(Mult, 0)),
								   Pair(49, Pair(AccInt, 10))));

	EXPECT_EQ(10u, chunk.getFromAddress());
	EXPECT_EQ(50u, chunk.getToAddress());

	EXPECT_THAT(over_chunk, ElementsAre(Pair(0, Pair(Jump, 100)),
										Pair(9, Pair(Add, 0)),
										Pair(10, Pair(Sub, 0)),
										Pair(12, Pair(Push, 0)),
										Pair(30, Pair(Pop, 0)),
										Pair(40, Pair(Mult, 0)),
										Pair(49, Pair(AccInt, 10)),
										Pair(50, Pair(AccBuiltin, 100)),
										Pair(51, Pair(AccNull, 0)),
										Pair(60, Pair(AccStack, 0))));

	EXPECT_EQ(0u, over_chunk.getFromAddress());
	EXPECT_EQ(70u, over_chunk.getToAddress());
}

TEST_F(NekoCodeChunkTest, SubChunk) {
	NekoCodeChunk ch = over_chunk.getSubChunk(10, 50);
	EXPECT_THAT(ch, ElementsAre(Pair(10, Pair(Sub, 0)),
								Pair(12, Pair(Push, 0)),
								Pair(30, Pair(Pop, 0)),
								Pair(40, Pair(Mult, 0)),
								Pair(49, Pair(AccInt, 10))));

	EXPECT_EQ(10u, ch.getFromAddress());
	EXPECT_EQ(50u, ch.getToAddress());
}

TEST_F(NekoCodeChunkTest, SplitInside) {
	unsigned int inside_int[] = {11, 20, 30};
	std::vector<unsigned int> inside(inside_int, inside_int + sizeof(inside_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(inside), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 11),
						  ElementsAre(Pair(10, Pair(Sub, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 11),
						  Property(&NekoCodeChunk::getToAddress, 20),
						  ElementsAre(Pair(12, Pair(Push, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 20),
						  Property(&NekoCodeChunk::getToAddress, 30),
						  ElementsAre()),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 30),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitUnsorted) {
	unsigned int unsorted_int[] = {20, 11, 30};
	std::vector<unsigned int> unsorted(unsorted_int, unsorted_int + sizeof(unsorted_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(unsorted), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 11),
						  ElementsAre(Pair(10, Pair(Sub, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 11),
						  Property(&NekoCodeChunk::getToAddress, 20),
						  ElementsAre(Pair(12, Pair(Push, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 20),
						  Property(&NekoCodeChunk::getToAddress, 30),
						  ElementsAre()),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 30),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitDuplicated) {
	unsigned int unsorted_int[] = {20, 11, 20, 30};
	std::vector<unsigned int> unsorted(unsorted_int, unsorted_int + sizeof(unsorted_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(unsorted), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 11),
						  ElementsAre(Pair(10, Pair(Sub, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 11),
						  Property(&NekoCodeChunk::getToAddress, 20),
						  ElementsAre(Pair(12, Pair(Push, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 20),
						  Property(&NekoCodeChunk::getToAddress, 30),
						  ElementsAre()),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 30),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitOneBorder1) {
	unsigned int one_border_int[] = {10, 20, 30};
	std::vector<unsigned int> one_border(one_border_int, one_border_int + sizeof(one_border_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(one_border), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 20),
						  ElementsAre(Pair(10, Pair(Sub, 0)),
									  Pair(12, Pair(Push, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 20),
						  Property(&NekoCodeChunk::getToAddress, 30),
						  ElementsAre()),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 30),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitOneBorder2) {
	unsigned int one_border_int[] = {11, 20, 50};
	std::vector<unsigned int> one_border(one_border_int, one_border_int + sizeof(one_border_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(one_border), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 11),
						  ElementsAre(Pair(10, Pair(Sub, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 11),
						  Property(&NekoCodeChunk::getToAddress, 20),
						  ElementsAre(Pair(12, Pair(Push, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 20),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitTwoBorders) {
	unsigned int two_borders_int[] = {10, 20, 50};
	std::vector<unsigned int> two_borders(two_borders_int, two_borders_int + sizeof(two_borders_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(two_borders), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 20),
						  ElementsAre(Pair(10, Pair(Sub, 0)),
									  Pair(12, Pair(Push, 0)))),
					AllOf(Property(&NekoCodeChunk::getFromAddress, 20),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitWholeInterval) {
	unsigned int whole_interval_int[] = {10, 50};
	std::vector<unsigned int> whole_interval(whole_interval_int, whole_interval_int + sizeof(whole_interval_int)/sizeof(int));

	EXPECT_THAT(chunk.splitByAddresses(whole_interval), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(10, Pair(Sub, 0)),
									  Pair(12, Pair(Push, 0)),
									  Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SplitEmpty) {
	std::vector<unsigned int> empty_interval;

	EXPECT_THAT(chunk.splitByAddresses(empty_interval), ElementsAre(
					AllOf(Property(&NekoCodeChunk::getFromAddress, 10),
						  Property(&NekoCodeChunk::getToAddress, 50),
						  ElementsAre(Pair(10, Pair(Sub, 0)),
									  Pair(12, Pair(Push, 0)),
									  Pair(30, Pair(Pop, 0)),
									  Pair(40, Pair(Mult, 0)),
									  Pair(49, Pair(AccInt, 10))))));
}

TEST_F(NekoCodeChunkTest, SubChunkOver) {
	ASSERT_DEATH(chunk.getSubChunk(0, 100), ".*[Aa]ssertion.*");
}

