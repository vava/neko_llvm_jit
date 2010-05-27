#include "jit.h"

#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetData.h"
#include "llvm/PassManager.h"
// //#include "llvm/ModuleProvider.h"
#include "llvm/LinkAllPasses.h"

#include "primitives.h"
#include "llvm_code_generation.h"

#include "neko_module.h"

#include <iostream>
#include <stdio.h>
#include <sstream>

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

		llvm::GuaranteedTailCallOpt = true;
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
		#define FAST_PRIMITIVE PRIMITIVE
		#define PRIMITIVE(name) ee->addGlobalMapping(ee->FindFunctionNamed(#name), (void *)p_##name);
		#include "primitives_list.h"
		#undef PRIMITIVE
		#undef FAST_PRIMITIVE

		if (vm->llvm_optimizations) {
			llvm::PassManager OurFPM;
			// Set up the optimizer pipeline.  Start with registering info about how the
			// target lays out data structures.
			OurFPM.add(new llvm::TargetData(*ee->getTargetData()));

			// Promote allocas to registers.
			OurFPM.add(llvm::createPromoteMemoryToRegisterPass());
			// Reassociate expressions.
			OurFPM.add(llvm::createReassociatePass());

			//selected passes from brainfuck jit compiler
			//  http://www.remcobloemen.nl/2010/02/brainfuck-using-llvm/
			//seems to be working good for us but we need more research on optimizations
			OurFPM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
			OurFPM.add(llvm::createLICMPass());                 // Hoist loop invariants
			OurFPM.add(llvm::createIndVarSimplifyPass());       // Canonicalize indvars
			OurFPM.add(llvm::createLoopDeletionPass());         // Delete dead loops

			// Simplify code
			for(int repeat=0; repeat < 3; repeat++)	{
				OurFPM.add(llvm::createGVNPass());                  // Remove redundancies
				OurFPM.add(llvm::createSCCPPass());                 // Constant prop with SCCP
				OurFPM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
				OurFPM.add(llvm::createInstructionCombiningPass());
				OurFPM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
				OurFPM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
				OurFPM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
			}

			OurFPM.run(*module);
		}

		if (vm->dump_llvm) {
			module->dump();
		}

		//converting globals
		for (ptr_val k = 0; k < m->nglobals; k++) {
			if (val_is_function(m->globals[k])) {
				vfunction * f = (vfunction*)m->globals[k];
				std::stringstream ss; ss << (int_val)f->addr;
				std::string name = ss.str();

				llvm::Function * F = module->getFunction(name);
				if (F) {
					llvm::verifyFunction(*F);

					void *FPtr = ee->getPointerToFunction(F);
					f->addr = FPtr;
					f->t = VAL_LLVMJITFUN;
				}
			}
		}

		llvm::Function * main = module->getFunction("main");
		llvm::verifyFunction(*main);

		void *FPtr = ee->getPointerToFunction(main);

		m->jit = FPtr;
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
