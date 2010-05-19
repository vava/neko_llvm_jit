#include "llvm/neko_code_container.h"
#include "llvm/neko_code_chunk.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

extern "C" {
	#include "neko_mod.h"
}

using namespace testing;

TEST(NekoCodeContainerTest, ExposeAllCode) {
	int_val opcodes[] = {Jump, 100, Add, Push, Pop, 5};
	ptr_val * opcodes_addr = (ptr_val *)opcodes;
	std::auto_ptr<neko_module> nm(makeNekoModule(opcodes, sizeof(opcodes) / sizeof(opcodes[0])));
	NekoCodeContainer cont(nm.get());
	NekoCodeChunk chunk = cont.getNekoCodeChunk();

	EXPECT_THAT(chunk, ElementsAre(Pair((ptr_val)opcodes_addr,	 Pair(Jump, 100)),
								   Pair((ptr_val)(opcodes_addr + 2), Pair(Add, _)),
								   Pair((ptr_val)(opcodes_addr + 3), Pair(Push, _)),
								   Pair((ptr_val)(opcodes_addr + 4), Pair(Pop, 5))));
}

TEST(NekoCodeContainerTest, WorksWithNoCode) {
	int_val opcodes[] = {};
	std::auto_ptr<neko_module> nm(makeNekoModule(opcodes, sizeof(opcodes) / sizeof(opcodes[0])));
	NekoCodeContainer cont(nm.get());
	NekoCodeChunk chunk = cont.getNekoCodeChunk();

	EXPECT_THAT(chunk, ElementsAre());
	EXPECT_TRUE(chunk.begin() == chunk.end()) << "begin() and end() has to have the same value";
}
