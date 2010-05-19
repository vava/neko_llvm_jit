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
	OPCODE opcodes[] = {Jump, (OPCODE)100, Add, Push, Pop, (OPCODE)5};
	ptr_val * opcodes_addr = (ptr_val *)opcodes;
	std::auto_ptr<neko_module> nm(makeNekoModule(opcodes, sizeof(opcodes) / sizeof(OPCODE)));
	NekoCodeContainer cont(nm.get());
	NekoCodeChunk chunk = cont.getNekoCodeChunk();

	EXPECT_THAT(chunk, ElementsAre(Pair((ptr_val)opcodes_addr,	 Pair((ptr_val)Jump, 100)),
								   Pair((ptr_val)(opcodes_addr + 2), Pair((ptr_val)Add, _)),
								   Pair((ptr_val)(opcodes_addr + 3), Pair((ptr_val)Push, _)),
								   Pair((ptr_val)(opcodes_addr + 4), Pair((ptr_val)Pop, 5))));
}

TEST(NekoCodeContainerTest, WorksWithNoCode) {
	OPCODE opcodes[] = {};
	std::auto_ptr<neko_module> nm(makeNekoModule(opcodes, sizeof(opcodes) / sizeof(OPCODE)));
	NekoCodeContainer cont(nm.get());
	NekoCodeChunk chunk = cont.getNekoCodeChunk();

	EXPECT_THAT(chunk, ElementsAre());
	EXPECT_TRUE(chunk.begin() == chunk.end()) << "begin() and end() has to have the same value";
}
