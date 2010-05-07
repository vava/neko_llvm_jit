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
	unsigned int * opcodes_addr = (unsigned int *)opcodes;
	std::auto_ptr<neko_module> nm(makeNekoModule(opcodes, sizeof(opcodes) / sizeof(OPCODE)));
	NekoCodeContainer cont(nm.get());
	NekoCodeChunk chunk = cont.getNekoCodeChunk();

	EXPECT_THAT(chunk, ElementsAre(Pair((unsigned int)opcodes_addr,	 Pair((unsigned int)Jump, 100)),
								   Pair((unsigned int)(opcodes_addr + 2), Pair((unsigned int)Add, _)),
								   Pair((unsigned int)(opcodes_addr + 3), Pair((unsigned int)Push, _)),
								   Pair((unsigned int)(opcodes_addr + 4), Pair((unsigned int)Pop, 5))));
}

TEST(NekoCodeContainerTest, WorksWithNoCode) {
}
