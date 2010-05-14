#include "llvm_code_generation.h"
#include "stack.h"

#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"

#include <sstream>

extern "C" {
	#include "../opcodes.h"
}

namespace {
void makeFunctionDeclaration(neko::Function const & neko_function, llvm::Module * module) {
    llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()), std::vector<const llvm::Type *>(), false);
	llvm::Function::Create(FT,
						   llvm::Function::ExternalLinkage,
						   neko_function.getName(),
						   module);
}

typedef std::map<unsigned int, llvm::BasicBlock *> id2block_type;

class CodeGeneration {
public:
	CodeGeneration(id2block_type const & id2block_,
				   llvm::Function * function_,
				   llvm::Module * module_)
		: id2block(id2block_)
		, function(function_)
		, module(module_)
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
				{
					llvm::Value * left = stack.load(builder, 0);
					llvm::Value * right = acc;

					llvm::BasicBlock * Then = llvm::BasicBlock::Create(function->getContext(), "then", function);
					llvm::BasicBlock * Else = llvm::BasicBlock::Create(function->getContext(), "else", function);
					llvm::BasicBlock * Merge = llvm::BasicBlock::Create(function->getContext(), "merge", function);

					builder.CreateCondBr(
						builder.CreateICmpEQ(
							builder.CreateAnd(
								h.is_int(builder, left),
								h.is_int(builder, right),
								"is_int(acc) && is_int(*sp)"),
							h.int_1()),
						Then,
						Else);
					//empty Else
					builder.SetInsertPoint(Else);
					builder.CreateBr(Merge);
					//Create Then
					builder.SetInsertPoint(Then);
					acc = builder.CreateSub(builder.CreateAdd(left, right), h.int_1());
					builder.CreateBr(Merge);
					builder.SetInsertPoint(Merge);

					llvm::PHINode * phi = builder.CreatePHI(h.int_t());
					phi->addIncoming(acc, Then);
					phi->addIncoming(h.int_0(), Else);

					acc = phi;

					stack.pop(1);
				}
				break;
			case Call:
				{
					// llvm::Value * vfunc_ptr = builder.CreateIntToPtr(acc, vfunction_struct, "(vfunction *)acc");
					// llvm::Value * val_type = builder.CreateLoad(builder.CreateConstGEP2_32(vfunc_ptr, 0, 0, "val_type"));
					// llvm::Value * addr = builder.CreateIntToPtr(builder.CreateLoad(builder.CreateConstGEP2_32(vfunc_ptr, 0, 2), "addr"), prim1);
					// llvm::Value * param1 = stack.load(0);
					// llvm::Value * returnValue;
					// llvm::Value * arr = builder.CreateAlloca(h.int_t(), h.int_n(param), "arr");
					// builder.CreateStore(param1, builder.CreateConstGEP1_32(arr, 0));
					// switch(param) {
					// 	case 1:
					// 		acc = builder.CreateCall2(addr, builder.CreatePtrToInt(arr, h.int_t()), h.int_n(param));
					// 		break;
					// }

					// stack.pop(1);
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

	llvm::Value * acc;
	Stack stack;
	Helper h;
};

void makeFunction(neko::Function const & neko_function, llvm::Module * module) {
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

	CodeGeneration cd(id2block, function, module);

	for (neko::Function::const_iterator it = neko_function.begin();
		 it != neko_function.end();
		 ++it)
		{
			//remember stack length
			cd.makeBasicBlock(*it);
			//check stack length
		}
}

}

llvm::Module * makeLLVMModule(neko::Module const & neko_module) {
	llvm::Module * module = new ::llvm::Module(neko_module.getName(), llvm::getGlobalContext());

	for (neko::Module::const_iterator it = neko_module.begin();
		 it != neko_module.end();
		 ++it)
		{
			makeFunctionDeclaration(*it, module);
		}

	for (neko::Module::const_iterator it = neko_module.begin();
		 it != neko_module.end();
		 ++it)
		{
			makeFunction(*it, module);
		}

	return module;
}
