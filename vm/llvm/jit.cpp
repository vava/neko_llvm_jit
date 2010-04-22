#include "jit.h"

#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/PassManager.h"
#include "llvm/ModuleProvider.h"
#include "llvm/LinkAllPasses.h"

#include <iostream>
#include <stdio.h>

//C interface
extern "C" {
	void * llvm_create_new_module() {
		return new Module();
	}

	void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count) {
		static_cast<Module*>(m)->add_new_opcode(opcode, param, params_count);
	}

	void* llvm_close_and_get_code(void *m) {
		Module * module = static_cast<Module*>(m);
		void * code = module->get_code();
		delete module;
		return code;
	}
}

namespace {
	llvm::ExecutionEngine * makeExecutionEngine(llvm::Module * module) {
		llvm::InitializeNativeTarget();

		std::string error_string;
		llvm::ExecutionEngine * ee = llvm::EngineBuilder(module)
										.setEngineKind(llvm::EngineKind::JIT)
										.setErrorStr(&error_string)
										.create();
		if (!ee) {
			std::cerr << "Could not create ExecutionEngine: " << error_string << std::endl;
		}

		return ee;
	}
}

Module::Module(): ctx(llvm::getGlobalContext()),
				  h(ctx),
				  llvmModule(new llvm::Module("test module", ctx)),
				  builder(ctx),
				  stack(ctx),
				  executionEngine(makeExecutionEngine(llvmModule))
{
	//create "main" function
	llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), std::vector<const llvm::Type *>(), false);
	main = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", llvmModule);
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(ctx, "entry", main);
	builder.SetInsertPoint(BB);
	stack.init(&builder);

	std::vector<const llvm::Type*> vfunction_struct_types;
	vfunction_struct_types.push_back(h.int_t());// val_type t
	vfunction_struct_types.push_back(h.int_t());// int nargs
	vfunction_struct_types.push_back(h.int_t());// void * addr
	vfunction_struct_types.push_back(h.int_t());// int32 * env
	vfunction_struct_types.push_back(h.int_t());// void * module

	vfunction_struct = llvm::StructType::get(ctx, vfunction_struct_types, true)->getPointerTo();

	std::vector<const llvm::Type*> prim1_types;
	prim1_types.push_back(h.int_t());
	prim1_types.push_back(h.int_t());
	prim1 = llvm::FunctionType::get(h.int_t(), prim1_types, false)->getPointerTo();
}

Module::~Module() {
}

void print_neko_instruction(enum OPCODE op, int p, int params_count);

void Module::add_new_opcode(OPCODE opcode, int param, int params_count) {
	print_neko_instruction(opcode, param, params_count);
	switch( opcode ) {
		case AccInt:
		case AccBuiltin:
			acc = h.int_n(param);
			break;
		case AccStack0:
			acc = stack.load(0);
			break;
		case AccStack1:
			acc = stack.load(1);
			break;
		case Add:
			{
				llvm::Value * left = stack.load(0);
				llvm::Value * right = acc;

				llvm::BasicBlock * Then = llvm::BasicBlock::Create(ctx, "then", main);
				llvm::BasicBlock * Else = llvm::BasicBlock::Create(ctx, "else", main);
				llvm::BasicBlock * Merge = llvm::BasicBlock::Create(ctx, "merge", main);

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
				llvm::Value * vfunc_ptr = builder.CreateIntToPtr(acc, vfunction_struct, "(vfunction *)acc");
				llvm::Value * val_type = builder.CreateLoad(builder.CreateConstGEP2_32(vfunc_ptr, 0, 0, "val_type"));
				llvm::Value * addr = builder.CreateIntToPtr(builder.CreateLoad(builder.CreateConstGEP2_32(vfunc_ptr, 0, 2), "addr"), prim1);
				llvm::Value * param1 = stack.load(0);
				llvm::Value * returnValue;
				llvm::Value * arr = builder.CreateAlloca(h.int_t(), h.int_n(param), "arr");
				builder.CreateStore(param1, builder.CreateConstGEP1_32(arr, 0));
				switch(param) {
					case 1:
						acc = builder.CreateCall2(addr, builder.CreatePtrToInt(arr, h.int_t()), h.int_n(param));
						break;
				}

				stack.pop(1);
			}
			break;
		case Push:
			stack.push(acc);
			break;
		case Pop:
			stack.pop(param);
			break;
		case Last:
			builder.CreateRetVoid();
			break;
	}
}

void * Module::get_code() {
	llvm::Function * main = llvmModule->getFunction("main");
	main->dump();
	llvm::verifyFunction(*main);

	//llvm::ExistingModuleProvider mp(llvmModule);

	llvm::PassManager OurFPM;//(&mp);
	// Set up the optimizer pipeline.  Start with registering info about how the
	// target lays out data structures.
	OurFPM.add(new llvm::TargetData(*executionEngine->getTargetData()));
	// Promote allocas to registers.
	OurFPM.add(llvm::createPromoteMemoryToRegisterPass());
	// Do simple "peephole" optimizations and bit-twiddling optzns.
	OurFPM.add(llvm::createInstructionCombiningPass());
	// Reassociate expressions.
	OurFPM.add(llvm::createReassociatePass());
	// Eliminate Common SubExpressions.
	OurFPM.add(llvm::createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	OurFPM.add(llvm::createCFGSimplificationPass());

	//OurFPM.doInitialization();
	OurFPM.run(*llvmModule);

	main->dump();

	//run main
	if (executionEngine.get()) {
		void *FPtr = executionEngine->getPointerToFunction(main);
		void (*FP)() = (void (*)())(intptr_t)FPtr;
		FP();
	}
	return 0;
}

void print_neko_instruction(enum OPCODE op, int p, int params_count) {
	switch( op ) {
		case AccNull: printf("AccNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccTrue: printf("AccTrue"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccFalse: printf("AccFalse"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccThis: printf("AccThis"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccInt: printf("AccInt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack: printf("AccStack"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccGlobal: printf("AccGlobal"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccEnv: printf("AccEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccField: printf("AccField"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccArray: printf("AccArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex: printf("AccIndex"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccBuiltin: printf("AccBuiltin"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetStack: printf("SetStack"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetGlobal: printf("SetGlobal"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetEnv: printf("SetEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetField: printf("SetField"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetArray: printf("SetArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetIndex: printf("SetIndex"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetThis: printf("SetThis"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Push: printf("Push"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Pop: printf("Pop"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Call: printf("Call"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case ObjCall: printf("ObjCall"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Jump: printf("Jump"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpIf: printf("JumpIf"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpIfNot: printf("JumpIfNot"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Trap: printf("Trap"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case EndTrap: printf("EndTrap"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Ret: printf("Ret"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case MakeEnv: printf("MakeEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case MakeArray: printf("MakeArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Bool: printf("Bool"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case IsNull: printf("IsNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case IsNotNull: printf("IsNotNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Add: printf("Add"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Sub: printf("Sub"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Mult: printf("Mult"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Div: printf("Div"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Mod: printf("Mod"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Shl: printf("Shl"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Shr: printf("Shr"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case UShr: printf("UShr"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Or: printf("Or"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case And: printf("And"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Xor: printf("Xor"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Eq: printf("Eq"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Neq: printf("Neq"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Gt: printf("Gt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Gte: printf("Gte"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Lt: printf("Lt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Lte: printf("Lte"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Not: printf("Not"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case TypeOf: printf("TypeOf"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Compare: printf("Compare"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Hash: printf("Hash"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case New: printf("New"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpTable: printf("JumpTable"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Apply: printf("Apply"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack0: printf("AccStack0"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack1: printf("AccStack1"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex0: printf("AccIndex0"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex1: printf("AccIndex1"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case PhysCompare: printf("PhysCompare"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case TailCall: printf("TailCall"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Last: printf("Last"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		default:
			printf("Something unexpected\n");
	}
}
