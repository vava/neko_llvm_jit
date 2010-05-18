#include "llvm_code_generation.h"

#include "primitives.h"
#include "stack.h"


#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"

#include <sstream>

extern "C" {
	#include "../opcodes.h"
}

namespace {
typedef std::map<unsigned int, llvm::BasicBlock *> id2block_type;

class CodeGeneration {
public:
	CodeGeneration(id2block_type const & id2block_,
				   llvm::Function * function_,
				   llvm::Module * module_,
				   neko_vm * vm_)
		: id2block(id2block_)
		, function(function_)
		, module(module_)
		, vm(vm_)
		, stack(&function->getEntryBlock())
		, h(function->getContext())
	{}

	void makeBasicBlock(neko::BasicBlock const & neko_bb) {
		llvm::BasicBlock * bb = id2block.find(neko_bb.getId())->second;
		llvm::IRBuilder<> builder(bb);

		for (neko::BasicBlock::const_iterator it = neko_bb.begin();
			 it != neko_bb.end();
			 ++it)
			{
				makeOpCode(builder,
						   (OPCODE)it->second.first, it->second.second);
			}
		builder.CreateRetVoid();
	}

	llvm::CallInst * callPrimitive(llvm::IRBuilder<> & builder, std::string const & primitive) const {
		std::vector<llvm::Value *> args;
		return callPrimitive(builder, primitive, args);
	}
	llvm::CallInst * callPrimitive(llvm::IRBuilder<> & builder, std::string const & primitive, llvm::Value * p1) const {
		std::vector<llvm::Value *> args;
		args.push_back(p1);
		return callPrimitive(builder, primitive, args);
	}
	llvm::CallInst * callPrimitive(llvm::IRBuilder<> & builder, std::string const & primitive, llvm::Value * p1, llvm::Value * p2) const {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);
		return callPrimitive(builder, primitive, args);
	}
	llvm::CallInst * callPrimitive(llvm::IRBuilder<> & builder, std::string const & primitive, llvm::Value * p1, llvm::Value * p2, llvm::Value * p3 ) const {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);args.push_back(p3);
		return callPrimitive(builder, primitive, args);
	}

	llvm::CallInst * callPrimitive(llvm::IRBuilder<> & builder, std::string const & primitive, std::vector<llvm::Value *> const & arguments) const {
		llvm::Function * P = module->getFunction(primitive);
		return builder.CreateCall(P, arguments.begin(), arguments.end());
	}

	void makeOpCode(llvm::IRBuilder<> & builder, OPCODE opcode, int param) {
		switch(opcode) {
			case AccInt:
			case AccBuiltin:
				acc = h.int_n(param);
				break;
			case AccStack0:
				acc = stack.load(builder, 0);
				break;
			case AccStack1:
				acc = stack.load(builder, 1);
				break;
			case Add:
				acc = callPrimitive(builder, "add", h.constant(vm), stack.load(builder, 0), acc);
				stack.pop(1);
				break;
			case Call:
				{
					std::vector<llvm::Value *> params;
					params.push_back(h.constant(vm));
					params.push_back(acc); params.push_back(h.int_n(param));
					for (int i = param - 1; i >=0; --i) {
						params.push_back(stack.load(builder, i));
					}
					acc = callPrimitive(builder, "call", params);
					stack.pop(param);
				}
				break;
			case Push:
				stack.push(builder, acc);
				break;
			case Pop:
				stack.pop(param);
				break;
			case Last:
				builder.CreateRetVoid();
				break;
		}
	}

private:
	id2block_type const & id2block;
	llvm::Function * function;
	llvm::Module * module;
	neko_vm * vm;

	llvm::Value * acc;
	Stack stack;
	Helper h;
};

class FunctionGenerator {
public:
	FunctionGenerator(llvm::Module * module_,
					  neko_vm * vm_)
		: module(module_)
		, h(module->getContext())
		, vm(vm_)
	{}

	void makeFunctionDeclaration(neko::Function const & neko_function) {
		llvm::FunctionType * FT = llvm::FunctionType::get(h.void_t(), std::vector<const llvm::Type *>(), false);
		llvm::Function::Create(FT,
							   llvm::Function::ExternalLinkage,
							   neko_function.getName(),
							   module);
	}

	void makeFunction(neko::Function const & neko_function) {
		llvm::Function * function = module->getFunction(neko_function.getName());
		llvm::BasicBlock::Create(module->getContext(), "entry", function);

		id2block_type id2block;

		for (neko::Function::const_iterator it = neko_function.begin();
			 it != neko_function.end();
			 ++it)
			{
				std::stringstream bb_name;
				bb_name << it->getId();
				id2block.insert(std::make_pair(it->getId(), llvm::BasicBlock::Create(module->getContext(), bb_name.str(), function)));
			}

		CodeGeneration cd(id2block, function, module, vm);

		for (neko::Function::const_iterator it = neko_function.begin();
			 it != neko_function.end();
			 ++it)
			{
				//remember stack length
				cd.makeBasicBlock(*it);
				//check stack length
			}

		llvm::IRBuilder<> builder(&function->getEntryBlock());
		builder.CreateBr(++function->begin());
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

	void registerPrimitive(std::string const & name, int (*primitive)()) {
		registerPrimitive(name, type_list(), false);
	}

	template<typename T>
	void registerPrimitive(std::string const & name, int (*primitive)(T)) {
		registerPrimitive(name, makeTypeList<T>(), false);
	}

	template<typename T1, typename T2>
	void registerPrimitive(std::string const & name, int (*primitive)(T1, T2)) {
		registerPrimitive(name, makeTypeList<T1, T2>(), false);
	}

	template<typename T1, typename T2, typename T3>
	void registerPrimitive(std::string const & name, int (*primitive)(T1, T2, T3)) {
		registerPrimitive(name, makeTypeList<T1, T2, T3>(), false);
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void registerPrimitive(std::string const & name, int (*primitive)(T1, T2, T3, T4)) {
		registerPrimitive(name, makeTypeList<T1, T2, T3, T4>(), false);
	}

	template<typename T>
	void registerPrimitive(std::string const & name, int (*primitive)(T, ...)) {
		registerPrimitive(name, makeTypeList<T>(), true);
	}

	template<typename T1, typename T2>
	void registerPrimitive(std::string const & name, int (*primitive)(T1, T2, ...)) {
		registerPrimitive(name, makeTypeList<T1, T2>(), true);
	}

	template<typename T1, typename T2, typename T3>
	void registerPrimitive(std::string const & name, int (*primitive)(T1, T2, T3, ...)) {
		registerPrimitive(name, makeTypeList<T1, T2, T3>(), true);
	}

private:
	void registerPrimitive(std::string const & name, type_list const & param_types, bool varArgs) {
		llvm::FunctionType * FT = llvm::FunctionType::get(
			h.int_t(),
			param_types,
			varArgs);

		llvm::Function::Create(FT,
							   llvm::Function::ExternalLinkage,
							   name,
							   module);
	}

	template<typename T>
	type_list makeTypeList() {
		type_list tl;
		tl.push_back(h.convert<T>());
		return tl;
	}

	template<typename T1, typename T2>
	type_list makeTypeList() {
		type_list tl;
		tl.push_back(h.convert<T1>());
		tl.push_back(h.convert<T2>());
		return tl;
	}

	template<typename T1, typename T2, typename T3>
	type_list makeTypeList() {
		type_list tl;
		tl.push_back(h.convert<T1>());
		tl.push_back(h.convert<T2>());
		tl.push_back(h.convert<T3>());
		return tl;
	}

	template<typename T1, typename T2, typename T3, typename T4>
	type_list makeTypeList() {
		type_list tl;
		tl.push_back(h.convert<T1>());
		tl.push_back(h.convert<T2>());
		tl.push_back(h.convert<T3>());
		tl.push_back(h.convert<T4>());
		return tl;
	}

	llvm::Module * module;
	Helper h;
};


void addPrimitives(llvm::Module * module) {
	PrimitiveRegistrator registrator(module);

    #define PRIMITIVE(name) registrator.registerPrimitive(#name, name);
	#include "primitives_list.h"
	#undef PRIMITIVE
}
}

llvm::Module * makeLLVMModule(neko::Module const & neko_module,
							  neko_vm * vm) {
	llvm::Module * module = new ::llvm::Module("neko module", llvm::getGlobalContext());

	addPrimitives(module);

	FunctionGenerator fg(module, vm);

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
