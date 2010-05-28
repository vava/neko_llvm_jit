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

extern "C" {
	#include "../opcodes.h"
}

namespace {

class CodeGeneration {
public:
	CodeGeneration(id2block_type const & id2block_,
				   llvm::AllocaInst * acc_,
				   llvm::Function * function_,
				   llvm::Module * module_)
		: id2block(id2block_)
		, acc(acc_)
		, function(function_)
		, module(module_)
		, stack(&function->getEntryBlock())
	{
		llvm::IRBuilder<> builder(&function->getEntryBlock());
		llvm::Function::arg_iterator it = function->arg_begin();
		vm = &*it++; //vm is always first parameter
		for (; it != function->arg_end(); ++it) {
			stack.push(builder, &*it);
		}
	}

	void makeBasicBlock(neko::BasicBlock const & neko_bb, llvm::BasicBlock * curr_bb, llvm::BasicBlock * next_bb) {
		LLVMInstrHelper instr_generator(curr_bb, next_bb,
										acc, vm, stack,
										function, module,
										id2block);

		for (neko::BasicBlock::const_iterator it = neko_bb.begin();
			 it != neko_bb.end();
			 ++it)
			{
				instr_generator.makeOpCode(it->second.first, it->second.second);
			}
	}

private:
	id2block_type const & id2block;
	llvm::AllocaInst * acc;
	llvm::Value * vm;
	llvm::Function * function;
	llvm::Module * module;

	Stack stack;
};

class FunctionGenerator {
public:
	FunctionGenerator(llvm::Module * module_)
		: module(module_)
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
		llvm::BasicBlock::Create(module->getContext(), "entry", function);

		llvm::IRBuilder<> builder(&function->getEntryBlock());
		llvm::AllocaInst * acc = builder.CreateAlloca(h.int_t(), 0, "acc");

		llvm::BasicBlock * returnBlock = llvm::BasicBlock::Create(module->getContext(), "return", function);
		{
			llvm::IRBuilder<> ret_builder(returnBlock);
			ret_builder.CreateRet(ret_builder.CreateLoad(acc));
		}

		id2block_type id2block;

		for (neko::Function::const_iterator it = neko_function.begin();
			 it != neko_function.end();
			 ++it)
			{
				std::stringstream bb_name;
				bb_name << it->getId();
				llvm::BasicBlock * bb = llvm::BasicBlock::Create(module->getContext(), bb_name.str(), function);
		 		id2block.insert(std::make_pair(it->getId(), std::make_pair(&(*it), bb)));
			}

		CodeGeneration cd(id2block, acc, function, module);

		for (id2block_type::const_iterator it = id2block.begin(); it != id2block.end(); ++it) {
			id2block_type::const_iterator next = it; ++next;
			llvm::BasicBlock * next_bb = (next == id2block.end()) ? returnBlock : next->second.second;
			llvm::BasicBlock * curr_bb = it->second.second;
			neko::BasicBlock const * curr_nekobb = it->second.first;
			cd.makeBasicBlock(*curr_nekobb, curr_bb, next_bb);
		}

		//add jump from entry block to first block in function
		builder.CreateBr(id2block.begin()->second.second);
	}
private:
	llvm::Module * module;
	Helper h;
	neko_vm * vm;
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

	REPEAT_MACRO_1_TO_5(PRIM)

	#undef PRIM
	#undef T
	#undef TT

	#define TT(x) typename T##x
	#define T(x) T##x
	#define PRIM(x) template<typename R, REPEAT_LIST_MACRO_##x(TT)>			\
		void registerPrimitive(std::string const & name, R (*primitive)(REPEAT_LIST_MACRO_##x(T), ...)) { \
						registerPrimitive(name, h.convert<R>(), makeTypeList<REPEAT_LIST_MACRO_##x(T)>(), true); \
					}

	REPEAT_MACRO_1_TO_5(PRIM)

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

	REPEAT_MACRO_1_TO_5(PRIM)

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
}

llvm::Module * makeLLVMModule(neko::Module const & neko_module) {
	llvm::Module * module = new ::llvm::Module("neko module", llvm::getGlobalContext());

	addPrimitives(module);

	FunctionGenerator fg(module);

	for (neko::Module::const_iterator it = neko_module.begin();
		 it != neko_module.end();
		 ++it)
		{
			fg.makeFunctionDeclaration(*it);
		}

	for (neko::Module::const_iterator it = neko_module.begin();
		 it != neko_module.end();
		 ++it)
		{
			fg.makeFunction(*it);
		}

	return module;
}
