#include "llvm/module_code_base.h"

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

class ModuleCodeBaseTest : public testing::Test {
protected:
	ModuleCodeBaseTest(): name(vh.makeString("test module")) {
	}

	NekoValueHolder vh;
	value name;
};

TEST_F(ModuleCodeBaseTest, Constructor) {
	int code_int[] = {
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

	int * code = nm->get()->code;

	ModuleCodeBase module(nm->get());

	std::stringstream code2;
	code2 << (int)(code + 2);

	//check names
	EXPECT_THAT(module,
				ElementsAre(Pair((int)(code + 2),
								 Property(&Function::getName, code2.str())),
							Pair((int)(code + 6),
								 Property(&Function::getName, "main"))
				));

	EXPECT_THAT(module,
				ElementsAre(Pair((int)(code + 2),
								 ElementsAre(
									 Pair((int)(code + 2),
										  ElementsAre(
											  Pair((int)(code + 2), Pair(Add, _)),
											  Pair((int)(code + 3), Pair(Push, _)),
											  Pair((int)(code + 4), Pair(Pop, 5)) )) )),
							Pair((int)(code + 6),
								 ElementsAre(
									 Pair((int)(code + 6),
										  ElementsAre(
											  Pair((int)(code + 6), Pair(Sub, _)) )),
									 Pair((int)(code + 7),
										  ElementsAre(
											  Pair((int)(code + 7), Pair(Mult, _)),
											  Pair((int)(code + 8), Pair(AccInt, 10)),
											  Pair((int)(code + 10), Pair(AccBuiltin, 100)),
											  Pair((int)(code + 12), Pair(Jump, (int)(code + 7))) )) ))));
}

TEST_F(ModuleCodeBaseTest, ConstructorJustMain) {
	int code_int[] = {
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

	int * code = nm->get()->code;

	ModuleCodeBase module(nm->get());

	EXPECT_THAT(module,
				ElementsAre(Pair((int)(code), Property(&Function::getName, "main"))));

	EXPECT_THAT(module,
				ElementsAre(Pair((int)(code),
								 ElementsAre(
									 Pair((int)(code),
										  ElementsAre(
											  Pair((int)(code), Pair(Jump, (int)(code + 6))),
											  Pair((int)(code + 2), Pair(Add, _)),
											  Pair((int)(code + 3), Pair(Push, _)),
											  Pair((int)(code + 4), Pair(Pop, 5)) )),
									 Pair((int)(code + 6),
										  ElementsAre(
											  Pair((int)(code + 6), Pair(Sub, _)) )),
									 Pair((int)(code + 7),
										  ElementsAre(
											  Pair((int)(code + 7), Pair(Mult, _)),
											  Pair((int)(code + 8), Pair(AccInt, 10)),
											  Pair((int)(code + 10), Pair(AccBuiltin, 100)),
											  Pair((int)(code + 12), Pair(Jump, (int)(code + 7))) )) ))));
}

TEST_F(ModuleCodeBaseTest, ConstructorEmpty) {
	int code_int[] = {
	};

	value globals_int[] = {
	};

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name, code_int, globals_int));

	EXPECT_THAT(ModuleCodeBase(nm->get()),
				ElementsAre(Pair(0, AllOf(
									 Property(&Function::getName, "main"),
									 ElementsAre(Pair(0, ElementsAre()))))));
}

TEST_F(ModuleCodeBaseTest, ConstructorName) {
	int code_int[] = {
	};

	value globals_int[] = {
	};

	value name_int = vh.makeString("Long module name");

	std::auto_ptr<NekoModuleWrapper> nm(new NekoModuleWrapper(name_int, code_int, globals_int));

	EXPECT_EQ("Long module name", ModuleCodeBase(nm->get()).getName());
}

typedef ModuleCodeBaseTest ModuleCodeBaseDeathTest;

TEST_F(ModuleCodeBaseDeathTest, IntersectedBlocks) {
	int code_int[] = {
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

	ASSERT_DEATH(ModuleCodeBase(nm->get()), "[Aa]ssertion");
}

TEST_F(ModuleCodeBaseDeathTest, NoJumpToMain) {
	int code_int[] = {
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

	ASSERT_DEATH(ModuleCodeBase(nm->get()), "[Aa]ssertion");
}

TEST_F(ModuleCodeBaseDeathTest, FirstJumpAdoptedInFunction) {
	int code_int[] = {
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

	ASSERT_DEATH(ModuleCodeBase(nm->get()), "[Aa]ssertion");
}
