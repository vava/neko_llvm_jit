#include "llvm/neko_module.h"

#include "common.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <sstream>

extern "C" {
	#include "neko_mod.h"
}

using namespace testing;
using namespace neko;

class ModuleTest : public testing::Test {
protected:
	ModuleTest(): name(vh.makeString("test module")) {
	}

	NekoValueHolder vh;
	value name;
};

TEST_F(ModuleTest, Constructor) {
	int_val code_int[] = {
		Jump, 6,
		Add,
		Push,
		Pop, 5,
		Sub,
		Mult,
		AccInt, 10,
		AccBuiltin, 100,
		Jump, 7
	};

	value globals_int[] = {
		vh.makeFunction((void *)2, 2, 0)
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	int_val * code = nm->get()->code;

	Module module(nm->get());

	std::stringstream code2;
	code2 << (ptr_val)(code + 2);

	//check names
	EXPECT_THAT(module,
				ElementsAre(AllOf(Property(&Function::getName, code2.str()),
								  Property(&Function::getArgumentsCount, 2)),
							AllOf(Property(&Function::getName, "main"),
								  Property(&Function::getArgumentsCount, 0))
				));

	EXPECT_THAT(module,
				ElementsAre(ElementsAre(
								ElementsAre(
									Pair((ptr_val)(code + 2), Pair(Add, _)),
									Pair((ptr_val)(code + 3), Pair(Push, _)),
									Pair((ptr_val)(code + 4), Pair(Pop, 5)) )),
							ElementsAre(
								ElementsAre(
									Pair((ptr_val)(code + 6), Pair(Sub, _)) ),
								ElementsAre(
									Pair((ptr_val)(code + 7), Pair(Mult, _)),
									Pair((ptr_val)(code + 8), Pair(AccInt, 10)),
									Pair((ptr_val)(code + 10), Pair(AccBuiltin, 100)),
									Pair((ptr_val)(code + 12), Pair(Jump, (ptr_val)(code + 7))) )) ));
}

TEST_F(ModuleTest, ConstructorJustMain) {
	int_val code_int[] = {
		Jump, 6,
		Add,
		Push,
		Pop, 5,
		Sub,
		Mult,
		AccInt, 10,
		AccBuiltin, 100,
		Jump, 7
	};

	value globals_int[] = {
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	int_val * code = nm->get()->code;

	Module module(nm->get());

	EXPECT_THAT(module,
				ElementsAre(Property(&Function::getName, "main")));

	EXPECT_THAT(module,
				ElementsAre(ElementsAre(
								ElementsAre(
									Pair((ptr_val)(code), Pair(Jump, (ptr_val)(code + 6))),
									Pair((ptr_val)(code + 2), Pair(Add, _)),
									Pair((ptr_val)(code + 3), Pair(Push, _)),
									Pair((ptr_val)(code + 4), Pair(Pop, 5)) ),
								ElementsAre(
									Pair((ptr_val)(code + 6), Pair(Sub, _)) ),
								ElementsAre(
									Pair((ptr_val)(code + 7), Pair(Mult, _)),
									Pair((ptr_val)(code + 8), Pair(AccInt, 10)),
									Pair((ptr_val)(code + 10), Pair(AccBuiltin, 100)),
									Pair((ptr_val)(code + 12), Pair(Jump, (ptr_val)(code + 7))) )) ));
}

TEST_F(ModuleTest, ConstructorEmpty) {
	int_val code_int[] = {
	};

	value globals_int[] = {
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	EXPECT_THAT(Module(nm->get()),
				ElementsAre(AllOf(
								Property(&Function::getName, "main"),
								ElementsAre(ElementsAre()))));
}

typedef ModuleTest ModuleDeathTest;

TEST_F(ModuleDeathTest, IntersectedBlocks) {
	int_val code_int[] = {
		Jump, 6,
		Add,
		Push,
		Pop, 5,
		Sub,
		Mult,
		AccInt, 10,
		AccBuiltin, 100,
		Jump, 5
	};

	value globals_int[] = {
		vh.makeFunction((void *)2, 2, 0)
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	ASSERT_DEATH(Module(nm->get()), "[Aa]ssertion");
}

TEST_F(ModuleDeathTest, NoJumpToMain) {
	int_val code_int[] = {
		Add,
		Push,
		Pop, 5,
		Sub,
		Mult,
		AccInt, 10,
		AccBuiltin, 100,
		Jump, 5
	};

	value globals_int[] = {
		vh.makeFunction((void *)2, 2, 0)
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	ASSERT_DEATH(Module(nm->get()), "[Aa]ssertion");
}

TEST_F(ModuleDeathTest, FirstJumpAdoptedInFunction) {
	int_val code_int[] = {
		Jump, 6,
		Add,
		Push,
		Pop, 5,
		Sub,
		Mult,
		AccInt, 10,
		AccBuiltin, 100,
		Jump, 5
	};

	value globals_int[] = {
		vh.makeFunction((void *)0, 2, 0)
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	ASSERT_DEATH(Module(nm->get()), "[Aa]ssertion");
}
