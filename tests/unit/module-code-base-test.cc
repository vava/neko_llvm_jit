#include "llvm/module_code_base.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

extern "C" {
	#include "neko_mod.h"
}

using namespace testing;

class ModuleCodeBaseTest : public testing::Test {
protected:
	ModuleCodeBaseTest() {
		int code[] = {
			Jump, 10,
			Add,
			Push,
			Pop, 5,
			Sub,
			Mult,
			AccInt, 10,
			AccBuiltin, 100,
			Jump, -10
		};

		value globals[] = {
			vh.makeFunction((void *)2, 2, 0)
		};

		nm.reset(new NekoModuleWrapper(code, globals));
	}

	NekoValueHolder vh;
	std::auto_ptr<NekoModuleWrapper> nm;
};

TEST_F(ModuleCodeBaseTest, Constructor) {
	//ModuleCodeBase c(nm->get());
}
