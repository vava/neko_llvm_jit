#include "llvm_code_generation.h"

#include "primitives.h"
#include "stack.h"


#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"

#include <sstream>
#include <stdio.h>


extern "C" {
	#include "../opcodes.h"
}

namespace {
typedef std::map<ptr_val, std::pair<neko::BasicBlock const *, llvm::BasicBlock *> > id2block_type;

class CodeGeneration {
public:
	CodeGeneration(id2block_type const & id2block_,
				   llvm::AllocaInst * acc_,
				   llvm::Function * function_,
				   llvm::Module * module_,
				   neko_vm * vm_)
		: id2block(id2block_)
		, acc(acc_)
		, function(function_)
		, module(module_)
		, vm(vm_)
		, stack(&function->getEntryBlock())
		, h(function->getContext())
	{}

	llvm::BasicBlock * getBasicBlock(int param) {
		return id2block.find(param)->second.second;
	}

	void makeBasicBlock(neko::BasicBlock const & neko_bb, llvm::BasicBlock * curr_bb, llvm::BasicBlock * next_bb) {
		llvm::IRBuilder<> builder(curr_bb);

		for (neko::BasicBlock::const_iterator it = neko_bb.begin();
			 it != neko_bb.end();
			 ++it)
			{
				makeOpCode(builder,
						   next_bb,
						   (OPCODE)it->second.first, it->second.second);
			}
		//make sure block ends with terminate expression
		if (builder.GetInsertBlock()->getTerminator() == 0) {
			builder.CreateBr(next_bb);
		}
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

	void set_acc(llvm::IRBuilder<> & builder, llvm::Value * acc_val) {
		builder.CreateStore(acc_val, acc);
	}

	llvm::Value * get_acc(llvm::IRBuilder<> & builder) const {
		return builder.CreateLoad(acc);
	}

	llvm::Value * get_null() const {
		return h.int_n((int_val)val_null);
	}

	llvm::Value * get_false() const {
		return h.int_n((int_val)val_false);
	}

	llvm::Value * get_true() const {
		return h.int_n((int_val)val_true);
	}

	void makeAccBoolBranching(llvm::IRBuilder<> & builder, llvm::Value * condition, bool straight_forward_order = true) {
		llvm::BasicBlock * bb_true = llvm::BasicBlock::Create(function->getContext(), "", function);
		llvm::BasicBlock * bb_false = llvm::BasicBlock::Create(function->getContext(), "", function);
		llvm::BasicBlock * bb_cont = llvm::BasicBlock::Create(function->getContext(), "", function);

		builder.CreateCondBr(condition,
							 (straight_forward_order) ? bb_true : bb_false,
							 (straight_forward_order) ? bb_false : bb_true);

		builder.SetInsertPoint(bb_true);
		set_acc(builder, get_true());
		builder.CreateBr(bb_cont);

		builder.SetInsertPoint(bb_false);
		set_acc(builder, get_false());
		builder.CreateBr(bb_cont);

		builder.SetInsertPoint(bb_cont);
	}

	void makeCompare(llvm::IRBuilder<> & builder, llvm::Value* (llvm::IRBuilder<>::*f_cmp)(llvm::Value *, llvm::Value *, const llvm::Twine &)) {
		set_acc(builder, callPrimitive(builder,
									   "val_compare",
									   builder.CreateIntToPtr(
										   stack.load(builder, 0),
										   h.convert<int_val *>()),
									   builder.CreateIntToPtr(
										   get_acc(builder),
										   h.convert<int_val *>())));
		stack.pop(1);

		makeAccBoolBranching(builder, builder.CreateAnd((builder.*f_cmp)(get_acc(builder), h.int_0(), ""),
														builder.CreateICmpNE(get_acc(builder), h.int_n(invalid_comparison))));
	}

	void makeOpCode(llvm::IRBuilder<> & builder, llvm::BasicBlock * next_bb, OPCODE opcode, int_val param) {
		switch(opcode) {
			case AccNull:
				set_acc(builder, get_null());
				break;
			case AccTrue:
				set_acc(builder, get_true());
				break;
			case AccFalse:
				set_acc(builder, get_false());
				break;
			case AccInt:
				set_acc(builder, h.int_n(param));
				break;
			case AccBuiltin:
				set_acc(builder, h.int_n(param));
				break;
			case AccStack0:
				set_acc(builder, stack.load(builder, 0));
				break;
			case AccStack1:
				set_acc(builder, stack.load(builder, 1));
				break;
			case AccStack:
				set_acc(builder, stack.load(builder, param));
				break;
			case AccGlobal:
				set_acc(builder, builder.CreateLoad(
							builder.CreateIntToPtr(
								h.int_n(param),
								h.convert<int_val *>()
							)
						));
				break;
			case SetStack:
				stack.store(builder, param, get_acc(builder));
				break;
			case SetGlobal:
				builder.CreateStore(
					get_acc(builder),
					builder.CreateIntToPtr(
						h.int_n(param),
						h.convert<int_val *>()
					)
				);
				break;
			case Add:
				set_acc(builder, callPrimitive(builder, "add", h.constant(vm), stack.load(builder, 0), get_acc(builder)));
				stack.pop(1);
				break;
			case Sub:
				set_acc(builder, callPrimitive(builder, "sub", stack.load(builder, 0), get_acc(builder)));
				stack.pop(1);
				break;
			case Mult:
				set_acc(builder, callPrimitive(builder, "mult", stack.load(builder, 0), get_acc(builder)));
				stack.pop(1);
				break;
			case Div:
				set_acc(builder, callPrimitive(builder, "div", stack.load(builder, 0), get_acc(builder)));
				stack.pop(1);
				break;
			case Mod:
				set_acc(builder, callPrimitive(builder, "mod", stack.load(builder, 0), get_acc(builder)));
				stack.pop(1);
				break;
			case Call:
				{
					std::vector<llvm::Value *> params;
					params.push_back(h.constant(vm));
					params.push_back(get_acc(builder)); params.push_back(h.int_n(param));
					for (int_val i = param - 1; i >=0; --i) {
						params.push_back(stack.load(builder, i));
					}
					set_acc(builder, callPrimitive(builder, "call", params));
					stack.pop(param);
				}
				break;
			case Lt:
				makeCompare(builder, &llvm::IRBuilder<>::CreateICmpSLT);
				break;
			case Eq:
				makeCompare(builder, &llvm::IRBuilder<>::CreateICmpEQ);
				break;
			case Lte:
				makeCompare(builder, &llvm::IRBuilder<>::CreateICmpSLE);
				break;
			case Gt:
				makeCompare(builder, &llvm::IRBuilder<>::CreateICmpSGT);
				break;
			case Gte:
				makeCompare(builder, &llvm::IRBuilder<>::CreateICmpSGE);
				break;
			case Neq:
				{
					set_acc(builder, callPrimitive(builder,
												   "val_compare",
												   builder.CreateIntToPtr(
													   stack.load(builder, 0),
													   h.convert<int_val *>()),
												   builder.CreateIntToPtr(
													   get_acc(builder),
													   h.convert<int_val *>())));
					stack.pop(1);

					makeAccBoolBranching(builder, builder.CreateICmpEQ(get_acc(builder), h.int_0()), false);
				}
				break;
			case Jump:
				builder.CreateBr(getBasicBlock(param));
				break;
			case JumpIf:
				builder.CreateCondBr(builder.CreateICmpEQ(get_acc(builder), get_true()), getBasicBlock(param), next_bb);
				break;
			case JumpIfNot:
				//callPrimitive(builder, "debug_print", get_acc(builder));
				builder.CreateCondBr(builder.CreateICmpNE(get_acc(builder), get_true()), getBasicBlock(param), next_bb);
				break;
			case Push:
				stack.push(builder, get_acc(builder));
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
	llvm::AllocaInst * acc;
	llvm::Function * function;
	llvm::Module * module;
	neko_vm * vm;

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
		llvm::FunctionType * FT = llvm::FunctionType::get(h.int_t(), std::vector<const llvm::Type *>(), false);
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

		CodeGeneration cd(id2block, acc, function, module, vm);

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

	void registerPrimitive(std::string const & name, int_val (*primitive)()) {
		registerPrimitive(name, type_list(), false);
	}

	template<typename T>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T)) {
		registerPrimitive(name, makeTypeList<T>(), false);
	}

	template<typename T1, typename T2>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T1, T2)) {
		registerPrimitive(name, makeTypeList<T1, T2>(), false);
	}

	template<typename T1, typename T2, typename T3>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T1, T2, T3)) {
		registerPrimitive(name, makeTypeList<T1, T2, T3>(), false);
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T1, T2, T3, T4)) {
		registerPrimitive(name, makeTypeList<T1, T2, T3, T4>(), false);
	}

	template<typename T>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T, ...)) {
		registerPrimitive(name, makeTypeList<T>(), true);
	}

	template<typename T1, typename T2>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T1, T2, ...)) {
		registerPrimitive(name, makeTypeList<T1, T2>(), true);
	}

	template<typename T1, typename T2, typename T3>
	void registerPrimitive(std::string const & name, int_val (*primitive)(T1, T2, T3, ...)) {
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

    #define PRIMITIVE(name) registrator.registerPrimitive(#name, p_##name);
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
