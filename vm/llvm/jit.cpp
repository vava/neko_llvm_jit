#include "jit.h"

#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/PassManager.h"
// //#include "llvm/ModuleProvider.h"
#include "llvm/LinkAllPasses.h"

#include "primitives.h"
#include "llvm_code_generation.h"

#include "neko_module.h"

#include <iostream>
#include <stdio.h>

//C interface
extern "C" {
	#include "neko.h"
	#include "neko_vm.h"
	#include "vm.h"
	#include "neko_mod.h"
	extern char *jit_boot_seq;

	void llvm_jit_boot(neko_vm * vm, int_val * code, value acc, neko_module * m) {
		((void (*)())code)();
	}

	void llvm_cpp_jit(neko_vm * vm, neko_module * m) {
		jit_boot_seq = (char *)&llvm_jit_boot;

		neko::Module code_base(m);
		if (vm->dump_neko) {
			code_base.neko_dump();
		}
		llvm::Module * module = makeLLVMModule(code_base, vm);

		llvm::InitializeNativeTarget();

		std::string error_string;
		llvm::ExecutionEngine * ee = llvm::EngineBuilder(module)
										.setEngineKind(llvm::EngineKind::JIT)
										.setErrorStr(&error_string)
										.create();
		if (!ee) {
			std::cerr << "Could not create ExecutionEngine: " << error_string << std::endl;
		}

		//register primitives
		#define PRIMITIVE(name) ee->addGlobalMapping(ee->FindFunctionNamed(#name), (void *)p_##name);
		#include "primitives_list.h"
		#undef PRIMITIVE

		if (vm->llvm_optimizations) {
			llvm::PassManager OurFPM;
			// Set up the optimizer pipeline.  Start with registering info about how the
			// target lays out data structures.
			OurFPM.add(new llvm::TargetData(*ee->getTargetData()));
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
			OurFPM.run(*module);
		}

		if (vm->dump_llvm) {
			module->dump();
		}

		llvm::Function * main = module->getFunction("main");
		llvm::verifyFunction(*main);

		void *FPtr = ee->getPointerToFunction(main);

		m->jit = FPtr;

		//TODO: patch globals
	}
}


// below is code that we might need later again
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
