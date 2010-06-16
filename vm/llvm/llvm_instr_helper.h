
#include "neko_basic_block.h"
#include "stack.h"
#include "blocks.h"

#include "llvm/Support/IRBuilder.h"
#include "llvm/BasicBlock.h"
#include "llvm/Module.h"

#include <map>

typedef std::map<ptr_val, std::pair<neko::BasicBlock const *, llvm::BasicBlock *> > id2block_type;
typedef std::map<llvm::BasicBlock *, Stack> Stacks;

class LLVMInstrHelper {
public:
	LLVMInstrHelper(Block * block_,
					llvm::AllocaInst * acc_,
					llvm::Value * vm_,
					llvm::Function * function_,
					llvm::Module * module_,
					Blocks & blocks_,
					neko_module * m_)
		: block(block_)
		, builder(block->getLLVMBlock())
		, acc(acc_)
		, vm(vm_)
		, stack(block->getStack()->lockStack(builder))
		, function(function_)
		, module(module_)
		, blocks(blocks_)
		, m(m_)
		, h(module->getContext())
	{}

	~LLVMInstrHelper() {
		//make sure block ends with terminate expression
		if (builder.GetInsertBlock()->getTerminator() == 0) {
			builder.CreateBr(block->getNext()->getLLVMBlock());
			checkAndCopyStack(stack, block->getNext());
		}
	}

	void makeJumpTable(std::vector<ptr_val> const & cases, Block * def);
	void makeOpCode(int_val opcode, int_val param, ptr_val pc);
private:
    void checkAndCopyStack(LockedStack const & curr_stack, Block * bb) {
		Stack & stack = curr_stack.unlock();

		bb->copyOrCheckStack(stack);
		blocks.addToCompilationQueue(bb);
	}

	llvm::Value * callPrimitive(std::string const & primitive) {
		std::vector<llvm::Value *> args;
		return callPrimitive(primitive, args);
	}

	llvm::Value * callPrimitive(std::string const & primitive, llvm::Value * p1) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);
		return callPrimitive(primitive, args);
	}

	llvm::Value * callPrimitive(std::string const & primitive, llvm::Value * p1, llvm::Value * p2) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);
		return callPrimitive(primitive, args);
	}

	llvm::Value * callPrimitive(std::string const & primitive, llvm::Value * p1, llvm::Value * p2, llvm::Value * p3 ) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);args.push_back(p3);
		return callPrimitive(primitive, args);
	}

	llvm::Value * callPrimitive(std::string const & primitive, llvm::Value * p1, llvm::Value * p2, llvm::Value * p3, llvm::Value * p4 ) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);args.push_back(p3);args.push_back(p4);
		return callPrimitive(primitive, args);
	}

	llvm::Value * callPrimitive(std::string const & primitive, llvm::Value * p1, llvm::Value * p2, llvm::Value * p3, llvm::Value * p4, llvm::Value * p5 ) {
		std::vector<llvm::Value *> args;
		args.push_back(p1);args.push_back(p2);args.push_back(p3);args.push_back(p4);args.push_back(p5);
		return callPrimitive(primitive, args);
	}

	llvm::Value * callPrimitive(std::string const & primitive, std::vector<llvm::Value *> const & arguments);

	void set_acc(llvm::Value * acc_val) {
		builder.CreateStore(acc_val, acc);
	}

	llvm::Value * get_acc() {
		return builder.CreateLoad(acc);
	}

	llvm::Value * get_null() const;
	llvm::Value * get_false() const;
	llvm::Value * get_true() const;
	llvm::Value * get_this();
	void set_this(llvm::Value * new_this);

	void makeAccBoolBranching(llvm::Value * condition, llvm::Value * true_, llvm::Value * false_);
	void makeCompare(llvm::Value* (llvm::IRBuilder<>::*f_cmp)(llvm::Value *, llvm::Value *, const llvm::Twine &));

	llvm::ConstantInt * makeAllocCInt(int_val value) const {
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

	void makeMemCpyCall(llvm::IRBuilder<> & builder, llvm::Value * dest, llvm::Value * source, llvm::Value * size) const;

	Block * block;
	llvm::IRBuilder<> builder;
	llvm::AllocaInst * acc;
	llvm::Value * vm;
	LockedStack stack;
	llvm::Function * function;
	llvm::Module * module;
	Blocks & blocks;
	neko_module * m;
	Helper h;
};
