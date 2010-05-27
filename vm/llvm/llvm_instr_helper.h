
#include "neko_basic_block.h"
#include "stack.h"

#include "llvm/Support/IRBuilder.h"
#include "llvm/BasicBlock.h"
#include "llvm/Module.h"

#include <map>

typedef std::map<ptr_val, std::pair<neko::BasicBlock const *, llvm::BasicBlock *> > id2block_type;

class LLVMInstrHelper {
public:
	LLVMInstrHelper(llvm::BasicBlock * curr_bb,
					llvm::BasicBlock * next_bb_,
					llvm::AllocaInst * acc_,
					Stack & stack_,
					llvm::Function * function_,
					llvm::Module * module_,
					neko_vm * vm_,
					id2block_type const & id2block_)
		: builder(curr_bb)
		, next_bb(next_bb_)
		, acc(acc_)
		, stack(stack_.lockStack(builder))
		, function(function_)
		, module(module_)
		, vm(vm_)
		, id2block(id2block_)
		, h(module->getContext())
	{}

	~LLVMInstrHelper() {
		//make sure block ends with terminate expression
		if (builder.GetInsertBlock()->getTerminator() == 0) {
			builder.CreateBr(next_bb);
		}
	}

	void makeOpCode(int_val opcode, int_val param);
private:
	llvm::CallInst * callPrimitive(std::string const & primitive) {
		std::vector<llvm::Value *> args;
		return callPrimitive(primitive, args);
	}

	llvm::CallInst * callPrimitive(std::string const & primitive, llvm::Value * p1) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);
		return callPrimitive(primitive, args);
	}

	llvm::CallInst * callPrimitive(std::string const & primitive, llvm::Value * p1, llvm::Value * p2) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);
		return callPrimitive(primitive, args);
	}

	llvm::CallInst * callPrimitive(std::string const & primitive, llvm::Value * p1, llvm::Value * p2, llvm::Value * p3 ) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);args.push_back(p3);
		return callPrimitive(primitive, args);
	}

	llvm::CallInst * callPrimitive(std::string const & primitive, std::vector<llvm::Value *> const & arguments);

	void set_acc(llvm::Value * acc_val) {
		builder.CreateStore(acc_val, acc);
	}

	llvm::Value * get_acc() {
		return builder.CreateLoad(acc);
	}

	llvm::Value * get_null() const;
	llvm::Value * get_false() const;
	llvm::Value * get_true() const;

	void makeAccBoolBranching(llvm::Value * condition, llvm::Value * true_, llvm::Value * false_);
	void makeCompare(llvm::Value* (llvm::IRBuilder<>::*f_cmp)(llvm::Value *, llvm::Value *, const llvm::Twine &));

	llvm::Value * makeAllocCInt(int_val value) const {
		return h.int_n((int_val)((((int)(value)) << 1) | 1));
	}

	llvm::Value * makeAllocInt(llvm::Value * value) {
		return builder.CreateSExt(
			builder.CreateOr(
				builder.CreateShl(
					builder.CreateTrunc(value, h.convert<int>()), 1),
				h.constant_1<int>()
			),
			h.convert<int_val>());
	}

	llvm::Value * makeNekoArray(std::vector<llvm::Value *> const & array);

	void makeIntOp(llvm::Value* (llvm::IRBuilder<>::*f)(llvm::Value *, llvm::Value *, const llvm::Twine &),
				   std::string const & op);

	llvm::BasicBlock * getBasicBlock(int param) {
		return id2block.find(param)->second.second;
	}

	llvm::IRBuilder<> builder;
	llvm::BasicBlock * next_bb;
	llvm::AllocaInst * acc;
	LockedStack stack;
	llvm::Function * function;
	llvm::Module * module;
	neko_vm * vm;
	id2block_type const & id2block;
	Helper h;
};
