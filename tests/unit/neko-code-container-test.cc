#include "llvm/neko_code_container.h"
#include "llvm/neko_code_chunk.h"

#include "common.h"

#include <gtest/gtest.h>

#include <memory>

extern "C" {
	#include "neko_mod.h"
}

TEST(NekoCodeContainerTest, ExposeAllCode) {
	OPCODE opcodes[] = {Jump, (OPCODE)100, Add, Push, Pop, (OPCODE)5};
	std::auto_ptr<neko_module> nm(makeNekoModule(opcodes, sizeof(opcodes) / sizeof(OPCODE)));
	NekoCodeContainer cont(nm.get());
	NekoCodeChunk chunk = cont.getNekoCodeChunk();

	for (NekoCodeChunk::const_iterator it = chunk.begin(); it != chunk.end(); ++it) {
	}
}

TEST(NekoCodeContainerTest, WorksWithNoCode) {
}
