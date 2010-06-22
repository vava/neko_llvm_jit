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

	void llvm_jit_boot(neko_vm * vm, int_val * code, value, neko_module *) {
		((void (*)(neko_vm *))code)(vm);
	}

	void llvm_cpp_jit(neko_vm * vm, neko_module * m) {
		jit_boot_seq = (char *)&llvm_jit_boot;

		neko::Module code_base(m);
		if (vm->dump_neko) {
			code_base.neko_dump();
		}
		llvm::Module * module = makeLLVMModule(code_base, m);

		llvm::GuaranteedTailCallOpt = true;
		llvm::JITEmitDebugInfo = true;
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
			//OurFPM.add(llvm::createPromoteMemoryToRegisterPass());
			// Reassociate expressions.
			OurFPM.add(llvm::createReassociatePass());

			//selected passes from brainfuck jit compiler
			//  http://www.remcobloemen.nl/2010/02/brainfuck-using-llvm/
			//seems to be working good for us but we need more research on optimizations
			OurFPM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
			// OurFPM.add(llvm::createLICMPass());                 // Hoist loop invariants
			OurFPM.add(llvm::createIndVarSimplifyPass());       // Canonicalize indvars
			OurFPM.add(llvm::createLoopDeletionPass());         // Delete dead loops

			// Simplify code
			for(int repeat=0; repeat < 3; repeat++)	{
				// OurFPM.add(llvm::createGVNPass());                  // Remove redundancies
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
