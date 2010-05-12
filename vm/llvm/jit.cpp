#include "jit.h"

// #include "llvm/Analysis/Verifier.h"
// #include "llvm/ExecutionEngine/JIT.h"
// #include "llvm/Target/TargetSelect.h"
// #include "llvm/Target/TargetData.h"
// #include "llvm/PassManager.h"
// //#include "llvm/ModuleProvider.h"
// #include "llvm/LinkAllPasses.h"

#include "module_code_base.h"

#include <iostream>
#include <stdio.h>

//C interface
extern "C" {
	void * llvm_cpp_jit(neko_module const * m) {
		printf("Creating module\n");

		neko::ModuleCodeBase code_base(m);

		printf("Dumping\n");
		code_base.neko_dump();
	}
}


// 	//TODO:
// 	//  Module::function_iterator fit = module.fbegin();
// 	//  Function::block_iterator bit = function.bbegin();
// 	//  Block::opcode_iterator oit = block.obegin();
// 	//    and opcode convertions (+ printing for debugging and testing purposes)

// namespace {
// 	llvm::ExecutionEngine * makeExecutionEngine(llvm::Module * module) {
// 		llvm::InitializeNativeTarget();

// 		std::string error_string;
// 		llvm::ExecutionEngine * ee = llvm::EngineBuilder(module)
// 										.setEngineKind(llvm::EngineKind::JIT)
// 										.setErrorStr(&error_string)
// 										.create();
// 		if (!ee) {
// 			std::cerr << "Could not create ExecutionEngine: " << error_string << std::endl;
// 		}

// 		return ee;
// 	}
// }

// Module::Module(): ctx(llvm::getGlobalContext()),
// 				  h(ctx),
// 				  llvmModule(new llvm::Module("test module", ctx)),
// 				  builder(ctx),
// 				  stack(ctx),
// 				  executionEngine(makeExecutionEngine(llvmModule))
// {
// 	//create "main" function
// 	llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), std::vector<const llvm::Type *>(), false);
// 	main = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", llvmModule);
// 	llvm::BasicBlock *BB = llvm::BasicBlock::Create(ctx, "entry", main);
// 	builder.SetInsertPoint(BB);
// 	stack.init(&builder);

// 	std::vector<const llvm::Type*> vfunction_struct_types;
// 	vfunction_struct_types.push_back(h.int_t());// val_type t
// 	vfunction_struct_types.push_back(h.int_t());// int nargs
// 	vfunction_struct_types.push_back(h.int_t());// void * addr
// 	vfunction_struct_types.push_back(h.int_t());// int32 * env
// 	vfunction_struct_types.push_back(h.int_t());// void * module

// 	vfunction_struct = llvm::StructType::get(ctx, vfunction_struct_types, true)->getPointerTo();

// 	std::vector<const llvm::Type*> prim1_types;
// 	prim1_types.push_back(h.int_t());
// 	prim1_types.push_back(h.int_t());
// 	prim1 = llvm::FunctionType::get(h.int_t(), prim1_types, false)->getPointerTo();
// }

// Module::~Module() {
// }

// void Module::add_new_opcode(OPCODE opcode, int param, int params_count) {
// 	// switch( opcode ) {
// 	// 	case AccInt:
// 	// 	case AccBuiltin:
// 	// 		acc = h.int_n(param);
// 	// 		break;
// 	// 	case AccStack0:
// 	// 		acc = stack.load(0);
// 	// 		break;
// 	// 	case AccStack1:
// 	// 		acc = stack.load(1);
// 	// 		break;
// 	// 	case Add:
// 	// 		{
// 	// 			llvm::Value * left = stack.load(0);
// 	// 			llvm::Value * right = acc;

// 	// 			llvm::BasicBlock * Then = llvm::BasicBlock::Create(ctx, "then", main);
// 	// 			llvm::BasicBlock * Else = llvm::BasicBlock::Create(ctx, "else", main);
// 	// 			llvm::BasicBlock * Merge = llvm::BasicBlock::Create(ctx, "merge", main);

// 	// 			builder.CreateCondBr(
// 	// 				builder.CreateICmpEQ(
// 	// 					builder.CreateAnd(
// 	// 						h.is_int(builder, left),
// 	// 						h.is_int(builder, right),
// 	// 						"is_int(acc) && is_int(*sp)"),
// 	// 					h.int_1()),
// 	// 				Then,
// 	// 				Else);
// 	// 			//empty Else
// 	// 			builder.SetInsertPoint(Else);
// 	// 			builder.CreateBr(Merge);
// 	// 			//Create Then
// 	// 			builder.SetInsertPoint(Then);
// 	// 			acc = builder.CreateSub(builder.CreateAdd(left, right), h.int_1());
// 	// 			builder.CreateBr(Merge);
// 	// 			builder.SetInsertPoint(Merge);

// 	// 			llvm::PHINode * phi = builder.CreatePHI(h.int_t());
// 	// 			phi->addIncoming(acc, Then);
// 	// 			phi->addIncoming(h.int_0(), Else);

// 	// 			acc = phi;

// 	// 			stack.pop(1);
// 	// 		}
// 	// 		break;
// 	// 	case Call:
// 	// 		{
// 	// 			llvm::Value * vfunc_ptr = builder.CreateIntToPtr(acc, vfunction_struct, "(vfunction *)acc");
// 	// 			llvm::Value * val_type = builder.CreateLoad(builder.CreateConstGEP2_32(vfunc_ptr, 0, 0, "val_type"));
// 	// 			llvm::Value * addr = builder.CreateIntToPtr(builder.CreateLoad(builder.CreateConstGEP2_32(vfunc_ptr, 0, 2), "addr"), prim1);
// 	// 			llvm::Value * param1 = stack.load(0);
// 	// 			llvm::Value * returnValue;
// 	// 			llvm::Value * arr = builder.CreateAlloca(h.int_t(), h.int_n(param), "arr");
// 	// 			builder.CreateStore(param1, builder.CreateConstGEP1_32(arr, 0));
// 	// 			switch(param) {
// 	// 				case 1:
// 	// 					acc = builder.CreateCall2(addr, builder.CreatePtrToInt(arr, h.int_t()), h.int_n(param));
// 	// 					break;
// 	// 			}

// 	// 			stack.pop(1);
// 	// 		}
// 	// 		break;
// 	// 	case Push:
// 	// 		stack.push(acc);
// 	// 		break;
// 	// 	case Pop:
// 	// 		stack.pop(param);
// 	// 		break;
// 	// 	case Last:
// 	// 		builder.CreateRetVoid();
// 	// 		break;
// 	// }
// }

// void * Module::get_code() {
// 	llvm::Function * main = llvmModule->getFunction("main");
// 	main->dump();
// 	llvm::verifyFunction(*main);

// 	//llvm::ExistingModuleProvider mp(llvmModule);

// 	llvm::PassManager OurFPM;//(&mp);
// 	// Set up the optimizer pipeline.  Start with registering info about how the
// 	// target lays out data structures.
// 	OurFPM.add(new llvm::TargetData(*executionEngine->getTargetData()));
// 	// Promote allocas to registers.
// 	OurFPM.add(llvm::createPromoteMemoryToRegisterPass());
// 	// Do simple "peephole" optimizations and bit-twiddling optzns.
// 	OurFPM.add(llvm::createInstructionCombiningPass());
// 	// Reassociate expressions.
// 	OurFPM.add(llvm::createReassociatePass());
// 	// Eliminate Common SubExpressions.
// 	OurFPM.add(llvm::createGVNPass());
// 	// Simplify the control flow graph (deleting unreachable blocks, etc).
// 	OurFPM.add(llvm::createCFGSimplificationPass());

// 	//OurFPM.doInitialization();
// 	OurFPM.run(*llvmModule);

// 	main->dump();

// 	//run main
// 	if (executionEngine.get()) {
// 		void *FPtr = executionEngine->getPointerToFunction(main);
// 		void (*FP)() = (void (*)())(intptr_t)FPtr;
// 		FP();
// 	}
// 	return 0;
// }
