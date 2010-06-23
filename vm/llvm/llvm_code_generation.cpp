#include "llvm_code_generation.h"

#include "repeat.h"
#include "primitives.h"
#include "stack.h"
#include "llvm_instr_helper.h"

#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"

#include <sstream>
#include <stdio.h>
#include <iostream>

extern "C" {
	#include "../opcodes.h"
	#include "neko_mod.h"
}

namespace {

class CodeGeneration {
public:
	CodeGeneration(Blocks & blocks_,
				   llvm::AllocaInst * acc_,
				   llvm::Function * function_,
				   llvm::Module * module_,
				   neko_module * m_)
		: blocks(blocks_)
		, acc(acc_)
		, function(function_)
		, module(module_)
		, m(m_)
		, stack(&function->getEntryBlock())
	{
		llvm::IRBuilder<> builder(&function->getEntryBlock());
		llvm::Function::arg_iterator it = function->arg_begin();
		vm = &*it++; //vm is always first parameter
		for (; it != function->arg_end(); ++it) {
			stack.push(builder, &*it);
		}

		blocks.first()->copyOrCheckStack(stack);
	}

	~CodeGeneration() {
	}

	void makeBasicBlock(Block * block) {
		LLVMInstrHelper instr_generator(block,
										acc, vm,
										function, module,
										blocks, m);

		neko::BasicBlock const * neko_bb = block->getNekoBlock();

		for (neko::BasicBlock::const_iterator it = neko_bb->begin();
			 it != neko_bb->end();
			 ++it)
			{
				if (it->second.first == JumpTable) {
					std::vector<ptr_val> cases;
					int_val case_count = it->second.second / 2;
					//get addresses of cases
					for (int_val i = 0; i < case_count; ++i) {
						cases.push_back((++it)->second.second);
					}
					instr_generator.makeJumpTable(cases, block->getNext());
				} else {
					//std::cout << "at " << it->first << std::endl;
					instr_generator.makeOpCode(it->second.first, it->second.second, it->first);
				}

				if (it->second.first == Ret || it->second.first == Jump) {
					//rest of the block is ignored
					return;
				}
			}
	}

private:
	Blocks & blocks;
	llvm::AllocaInst * acc;
	llvm::Value * vm;
	llvm::Function * function;
	llvm::Module * module;
	neko_module * m;

	Stack stack;
};

class FunctionGenerator {
public:
	FunctionGenerator(llvm::Module * module_, neko_module * m_)
		: module(module_)
		, m(m_)
		, h(module->getContext())
	{}

	void makeFunctionDeclaration(neko::Function const & neko_function) {
		std::vector<const llvm::Type *> params;
		params.reserve(neko_function.getArgumentsCount() + 1);
		params.push_back(h.convert<neko_vm *>()); //vm * is always a first parameter
		for (int i = 0; i < neko_function.getArgumentsCount(); i++) {
			params.push_back(h.int_t());
		}

		llvm::FunctionType * FT = llvm::FunctionType::get(h.int_t(),
														  params,
														  false);
		llvm::Function::Create(FT,
							   llvm::Function::ExternalLinkage,
							   neko_function.getName(),
							   module);
	}

	void makeFunction(neko::Function const & neko_function) {
		llvm::Function * function = module->getFunction(neko_function.getName());
		//std::cout << "function " << neko_function.getName() << std::endl;
		llvm::BasicBlock::Create(module->getContext(), "entry", function);

		llvm::IRBuilder<> builder(&function->getEntryBlock());
		llvm::AllocaInst * acc = builder.CreateAlloca(h.int_t(), 0, "acc");

		llvm::BasicBlock * returnBlock = llvm::BasicBlock::Create(module->getContext(), "return", function);
		{
			llvm::IRBuilder<> ret_builder(returnBlock);
			ret_builder.CreateRet(ret_builder.CreateLoad(acc));
		}

		Blocks blocks(neko_function, returnBlock, function);
		CodeGeneration cd(blocks, acc, function, module, m);

		while(Block * block = blocks.getNextToCompile()) {
			assert(block->hasStack());
			cd.makeBasicBlock(block);
		}

		//add jump from entry block to first block in function
		builder.CreateBr(blocks.first()->getLLVMBlock());
	}
private:
	llvm::Module * module;
	neko_module * m;
	Helper h;
};

class PrimitiveRegistrator {
	typedef std::vector<const llvm::Type *> type_list;
public:
	PrimitiveRegistrator(llvm::Module * module_)
		: module(module_)
		, h(module->getContext())
	{}

	template<typename R>
	void registerPrimitive(std::string const & name, R (*primitive)()) {
		registerPrimitive(name, h.convert<R>(), type_list(), false);
	}

	#define TT(x) typename T##x
	#define T(x) T##x
	#define PRIM(x) template<typename R, REPEAT_LIST_MACRO_##x(TT)>			\
					void registerPrimitive(std::string const & name, R (*primitive)(REPEAT_LIST_MACRO_##x(T))) { \
						registerPrimitive(name, h.convert<R>(), makeTypeList<REPEAT_LIST_MACRO_##x(T)>(), false); \
					}

	REPEAT_MACRO_1_TO_10(PRIM)

	#undef PRIM
	#undef T
	#undef TT

	#define TT(x) typename T##x
	#define T(x) T##x
	#define PRIM(x) template<typename R, REPEAT_LIST_MACRO_##x(TT)>			\
		void registerPrimitive(std::string const & name, R (*primitive)(REPEAT_LIST_MACRO_##x(T), ...)) { \
						registerPrimitive(name, h.convert<R>(), makeTypeList<REPEAT_LIST_MACRO_##x(T)>(), true); \
					}

	REPEAT_MACRO_1_TO_10(PRIM)

	#undef PRIM
	#undef T
	#undef TT

private:
	void registerPrimitive(std::string const & name, llvm::Type const * resultType, type_list const & param_types, bool varArgs) {
		llvm::FunctionType * FT = llvm::FunctionType::get(
			resultType,
			param_types,
			varArgs);

		llvm::Function::Create(FT,
							   llvm::Function::ExternalLinkage,
							   name,
							   module);
	}

	#define TT(x) typename T##x
	#define T(x) h.convert<T##x>()
	#define PRIM(x) template<REPEAT_LIST_MACRO_##x(TT)>			\
					type_list makeTypeList() { \
						llvm::Type const * types[] = {REPEAT_LIST_MACRO_##x(T)}; \
						return type_list(types, types + sizeof(types) / sizeof(types[0])); \
					}

	REPEAT_MACRO_1_TO_10(PRIM)

	#undef PRIM
	#undef T
	#undef TT

	llvm::Module * module;
	Helper h;
};


void addPrimitives(llvm::Module * module) {
	PrimitiveRegistrator registrator(module);

	#define PRIMITIVE(name) registrator.registerPrimitive(#name, p_##name);
	#include "primitives_list.h"
	#undef PRIMITIVE
}

void makeThrowFunction(llvm::Module * module) {
	llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()),
													  false);
	llvm::Function * F = llvm::Function::Create(FT,
												llvm::Function::ExternalLinkage,
												"throw",
												module);

	llvm::BasicBlock * bb = llvm::BasicBlock::Create(module->getContext(), "entry", F);

	llvm::IRBuilder<> builder(bb);
	builder.CreateUnwind();
}
}

llvm::Module * makeLLVMModule(neko::Module const & n_m, neko_module * m) {
	llvm::Module * module = new ::llvm::Module("neko module", llvm::getGlobalContext());

	addPrimitives(module);
	makeThrowFunction(module);

	FunctionGenerator fg(module, m);

	for (neko::Module::const_iterator it = n_m.begin();
		 it != n_m.end();
		 ++it)
		{
			fg.makeFunctionDeclaration(*it);
		}

	for (neko::Module::const_iterator it = n_m.begin();
		 it != n_m.end();
		 ++it)
		{
			fg.makeFunction(*it);
		}


	return module;
}
