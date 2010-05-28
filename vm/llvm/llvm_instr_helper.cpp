#include "llvm_instr_helper.h"

extern "C" {
	#include "../opcodes.h"
	#include "../neko.h"
	#include "../vm.h"
}

llvm::Value * LLVMInstrHelper::get_null() const {
	return h.int_n((int_val)val_null);
}

llvm::Value * LLVMInstrHelper::get_false() const {
	return h.int_n((int_val)val_false);
}

llvm::Value * LLVMInstrHelper::get_true() const {
	return h.int_n((int_val)val_true);
}

llvm::Value * LLVMInstrHelper::get_this() {
	return
		builder.CreateLoad(
			builder.CreateConstGEP2_32(
				vm,
				0, 3, "vm->vthis"));
}

llvm::CallInst * LLVMInstrHelper::callPrimitive(std::string const & primitive, std::vector<llvm::Value *> const & arguments) {
	llvm::Function * P = module->getFunction(primitive);
	llvm::CallInst * callInst = builder.CreateCall(P, arguments.begin(), arguments.end());
	callInst->setCallingConv(P->getCallingConv());
	return callInst;
}

void LLVMInstrHelper::makeAccBoolBranching(llvm::Value * condition, llvm::Value * true_, llvm::Value * false_) {
	llvm::BasicBlock * bb_true = llvm::BasicBlock::Create(function->getContext(), "", function);
	llvm::BasicBlock * bb_false = llvm::BasicBlock::Create(function->getContext(), "", function);
	llvm::BasicBlock * bb_cont = llvm::BasicBlock::Create(function->getContext(), "", function);

	builder.CreateCondBr(condition,
						 bb_true,
						 bb_false);

	builder.SetInsertPoint(bb_true);
	set_acc(true_);
	builder.CreateBr(bb_cont);

	builder.SetInsertPoint(bb_false);
	set_acc(false_);
	builder.CreateBr(bb_cont);

	builder.SetInsertPoint(bb_cont);
}

void LLVMInstrHelper::makeCompare(llvm::Value* (llvm::IRBuilder<>::*f_cmp)(llvm::Value *, llvm::Value *, const llvm::Twine &)) {
	set_acc(builder.CreateSExt(callPrimitive("val_compare",
											 builder.CreateIntToPtr(
												 stack.load(0),
												 h.convert<int_val *>()),
											 builder.CreateIntToPtr(
												 get_acc(),
												 h.convert<int_val *>())),
							   h.convert<int_val>()));
	stack.pop(1);

	makeAccBoolBranching(builder.CreateAnd((builder.*f_cmp)(get_acc(), h.int_0(), ""),
										   builder.CreateICmpNE(get_acc(), h.int_n(invalid_comparison))),
						 get_true(), get_false());
}

void LLVMInstrHelper::makeIntOp(llvm::Value* (llvm::IRBuilder<>::*f)(llvm::Value *, llvm::Value *, const llvm::Twine &),
			   std::string const & op) {
	llvm::BasicBlock * bb_true = llvm::BasicBlock::Create(function->getContext(), "", function);
	llvm::BasicBlock * bb_false = llvm::BasicBlock::Create(function->getContext(), "", function);
	llvm::BasicBlock * bb_cont = llvm::BasicBlock::Create(function->getContext(), "", function);

	builder.CreateCondBr(
		builder.CreateICmpNE(
			builder.CreateAnd(
				builder.CreateAnd(get_acc(), h.int_n(1)),
				builder.CreateAnd(stack.load(0), h.int_n(1))
			),
			h.int_0()),
		bb_true,
		bb_false);

	builder.SetInsertPoint(bb_true);
	set_acc(makeAllocInt((builder.*f)(
							 builder.CreateAShr(
								 builder.CreateTrunc(stack.load(0), h.convert<int>()),
								 h.constant_1<int>()
							 ),
							 builder.CreateAShr(
								 builder.CreateTrunc(get_acc(), h.convert<int>()),
								 h.constant_1<int>()
							 ),
							 ""
						 )
			)
	);
	builder.CreateBr(bb_cont);

	builder.SetInsertPoint(bb_false);
	callPrimitive("val_throw", callPrimitive("alloc_string", builder.CreateGlobalStringPtr(op.c_str())));
	builder.CreateBr(bb_cont);

	builder.SetInsertPoint(bb_cont);

	stack.pop(1);
}

llvm::Value * LLVMInstrHelper::makeNekoArray(std::vector<llvm::Value *> const & array) {
	llvm::Value * arr = callPrimitive("alloc_array", h.constant<unsigned int>(array.size()));

	llvm::Value * ptr =
		builder.CreatePointerCast(
			builder.CreateConstGEP2_32(
				builder.CreatePointerCast(arr, h.convert<varray *>()),
				0, 1, "ptr"),
			h.convert_array<int_val>(array.size())->getPointerTo());

	llvm::Value * llvm_arr = builder.CreateLoad(ptr);

	for (unsigned int i = 0; i < array.size(); i++) {
		llvm_arr = builder.CreateInsertValue(llvm_arr, array[i], i);
	}

	builder.CreateStore(llvm_arr, ptr);

	return builder.CreatePtrToInt(arr, h.int_t());
}

void LLVMInstrHelper::makeOpCode(int_val opcode, int_val param) {
	switch((OPCODE)opcode) {
		case AccNull:
			set_acc(get_null());
			break;
		case AccTrue:
			set_acc(get_true());
			break;
		case AccFalse:
			set_acc(get_false());
			break;
		case AccInt:
			set_acc(h.int_n(param));
			break;
		case AccBuiltin:
			set_acc(h.int_n(param));
			break;
		case AccStack0:
			set_acc(stack.load(0));
			break;
		case AccStack1:
			set_acc(stack.load(1));
			break;
		case AccStack:
			set_acc(stack.load(param));
			break;
		case AccGlobal:
			set_acc(builder.CreateLoad(
						builder.CreateIntToPtr(
							h.int_n(param),
							h.convert<int_val *>()
						)
					));
			break;
		case SetStack:
			stack.store(param, get_acc());
			break;
		case SetGlobal:
			builder.CreateStore(
				get_acc(),
				builder.CreateIntToPtr(
					h.int_n(param),
					h.convert<int_val *>()
				)
			);
			break;
		case Add:
			set_acc(callPrimitive("add", vm, stack.load(0), get_acc()));
			stack.pop(1);
			break;
		case Sub:
			set_acc(callPrimitive("sub", stack.load(0), get_acc()));
			stack.pop(1);
			break;
		case Mult:
			set_acc(callPrimitive("mult", stack.load(0), get_acc()));
			stack.pop(1);
			break;
		case Div:
			set_acc(callPrimitive("div", stack.load(0), get_acc()));
			stack.pop(1);
			break;
		case Mod:
			set_acc(callPrimitive("mod", stack.load(0), get_acc()));
			stack.pop(1);
			break;
		case Shl:
			makeIntOp(&llvm::IRBuilder<>::CreateShl, "<<");
			break;
		case Shr:
			makeIntOp(&llvm::IRBuilder<>::CreateAShr, ">>");
			break;
		case UShr:
			makeIntOp(&llvm::IRBuilder<>::CreateLShr, ">>>");
			break;
		case Or:
			makeIntOp(&llvm::IRBuilder<>::CreateOr, "|");
			break;
		case And:
			makeIntOp(&llvm::IRBuilder<>::CreateAnd, "&");
			break;
		case Xor:
			makeIntOp(&llvm::IRBuilder<>::CreateXor, "^");
			break;
		case Call:
			{
				std::vector<llvm::Value *> params;
				params.reserve(param + 3);

				params.push_back(vm);params.push_back(get_this());
				params.push_back(get_acc()); params.push_back(h.int_n(param));
				for (int_val i = param - 1; i >=0; --i) {
					params.push_back(stack.load(i));
				}
				set_acc(callPrimitive("call", params));
				stack.pop(param);
			}
			break;
		case TailCall:
			{
				//PopInfos(true);
				std::vector<llvm::Value *> params;
				params.reserve(param + 3);

				int nargs = (int)((param) & 7);
				params.push_back(vm);params.push_back(get_this());
				params.push_back(get_acc()); params.push_back(h.int_n(nargs));
				for (int_val i = nargs - 1; i >=0; --i) {
					params.push_back(stack.load(i));
				}
				set_acc(callPrimitive("call", params));
				stack.pop(nargs);
			}
			break;
		case Ret:
			{
				//PopInfos(true);
				stack.pop(param);
			}
			break;
		case Lt:
			makeCompare(&llvm::IRBuilder<>::CreateICmpSLT);
			break;
		case Eq:
			makeCompare(&llvm::IRBuilder<>::CreateICmpEQ);
			break;
		case Lte:
			makeCompare(&llvm::IRBuilder<>::CreateICmpSLE);
			break;
		case Gt:
			makeCompare(&llvm::IRBuilder<>::CreateICmpSGT);
			break;
		case Gte:
			makeCompare(&llvm::IRBuilder<>::CreateICmpSGE);
			break;
		case Neq:
			{
				set_acc(builder.CreateSExt(
							callPrimitive("val_compare",
										  builder.CreateIntToPtr(
											  stack.load(0),
											  h.convert<int_val *>()),
										  builder.CreateIntToPtr(
											  get_acc(),
											  h.convert<int_val *>())),
							h.convert<int_val>()));
				stack.pop(1);

				makeAccBoolBranching(builder.CreateICmpEQ(get_acc(), h.int_0()), get_false(), get_true());
			}
			break;
		case Bool:
			makeAccBoolBranching(builder.CreateOr(
									 builder.CreateOr(
										 builder.CreateICmpEQ(get_acc(), get_false()),
										 builder.CreateICmpEQ(get_acc(), get_null())),
									 builder.CreateICmpEQ(get_acc(), h.int_1())),
								 get_false(), get_true());
			break;
		case Not:
			makeAccBoolBranching(builder.CreateOr(
									 builder.CreateOr(
										 builder.CreateICmpEQ(get_acc(), get_false()),
										 builder.CreateICmpEQ(get_acc(), get_null())),
									 builder.CreateICmpEQ(get_acc(), h.int_1())),
								 get_true(), get_false());
			break;
		case IsNull:
			makeAccBoolBranching(builder.CreateICmpEQ(get_acc(), get_null()),
								 get_true(), get_false());
			break;
		case IsNotNull:
			makeAccBoolBranching(builder.CreateICmpEQ(get_acc(), get_null()),
								 get_false(), get_true());
			break;
		case Compare:
			{
				set_acc(builder.CreateSExt(
							callPrimitive("val_compare",
										  builder.CreateIntToPtr(
											  stack.load(0),
											  h.convert<int_val *>()),
										  builder.CreateIntToPtr(
											  get_acc(),
											  h.convert<int_val *>())),
							h.convert<int_val>()));
				stack.pop(1);

				makeAccBoolBranching(builder.CreateICmpEQ(get_acc(), h.int_n(invalid_comparison)),
									 get_null(), makeAllocInt(get_acc()));
			}
			break;
		case PhysCompare:
			{
				llvm::BasicBlock * bb_true = llvm::BasicBlock::Create(function->getContext(), "", function);
				llvm::BasicBlock * bb_false = llvm::BasicBlock::Create(function->getContext(), "", function);
				llvm::BasicBlock * bb_cont = llvm::BasicBlock::Create(function->getContext(), "", function);

				builder.CreateCondBr(builder.CreateICmpSGT(stack.load(0), get_acc()),
									 bb_true,
									 bb_false);

				builder.SetInsertPoint(bb_true);
				set_acc(makeAllocCInt(1));
				builder.CreateBr(bb_cont);

				builder.SetInsertPoint(bb_false);
				makeAccBoolBranching(builder.CreateICmpSLT(stack.load(0), get_acc()),
									 makeAllocCInt(-1), makeAllocCInt(0));
				builder.CreateBr(bb_cont);

				builder.SetInsertPoint(bb_cont);
				stack.pop(1);
			}
			break;
		case Jump:
			builder.CreateBr(getBasicBlock(param));
			break;
		case JumpIf:
			builder.CreateCondBr(builder.CreateICmpEQ(get_acc(), get_true()), getBasicBlock(param), next_bb);
			break;
		case JumpIfNot:
			//callPrimitive(builder, "debug_print", get_acc(builder));
			builder.CreateCondBr(builder.CreateICmpNE(get_acc(), get_true()), getBasicBlock(param), next_bb);
			break;
		case Push:
			stack.push(get_acc());
			break;
		case Pop:
			stack.pop(param);
			break;
		case MakeArray:
			{
				std::vector<llvm::Value *> values;
				values.reserve(param + 1);

				values.push_back(get_acc());
				for (int_val i = param - 1; i >= 0; --i) {
					values.push_back(stack.load(i));
				}

				set_acc(makeNekoArray(values));

				stack.pop(param);
			}
			break;
		case AccIndex0:
			set_acc(callPrimitive("get_arr_index", get_acc(), makeAllocCInt(0)));
			break;
		case AccIndex1:
			set_acc(callPrimitive("get_arr_index", get_acc(), makeAllocCInt(1)));
			break;
		case AccIndex:
			set_acc(callPrimitive("get_arr_index", get_acc(), makeAllocCInt(param)));
			break;
		case AccArray:
			set_acc(callPrimitive("get_arr_index", stack.load(0), get_acc()));
			stack.pop(1);
			break;
		case SetArray:
			set_acc(callPrimitive("set_arr_index", stack.load(0), stack.load(1), get_acc()));
			stack.pop(2);
			break;
		case SetIndex:
			set_acc(callPrimitive("set_arr_index", stack.load(0), makeAllocCInt(param), get_acc()));
			stack.pop(1);
			break;
		case New:
			set_acc(builder.CreatePtrToInt(
						callPrimitive("alloc_object",
									  builder.CreateIntToPtr(
										  get_acc(),
										  h.convert<value>())),
						h.int_t()));
			break;
		case Hash:
			set_acc(callPrimitive("hash", get_acc()));
			break;
		case AccField:
			set_acc(callPrimitive("acc_field", vm, get_acc(), h.int_n(param)));
			break;
		case SetField:
			callPrimitive("set_field", stack.load(0), h.int_n(param), get_acc());
			stack.pop(1);
			break;
		case AccThis:
			set_acc(builder.CreatePtrToInt(get_this(), h.int_t()));
			break;
		case ObjCall:
			{
				std::vector<llvm::Value *> params;
				params.reserve(param + 3);

				params.push_back(vm);params.push_back(builder.CreateIntToPtr(stack.load(0), h.convert<value>()));
				params.push_back(get_acc()); params.push_back(h.int_n(param));
				for (int_val i = param; i > 0; --i) {
					params.push_back(stack.load(i));
				}
				set_acc(callPrimitive("call", params));
				stack.pop(param);
			}
			break;
		case Last:
			builder.CreateRetVoid();
			break;
	}
}
