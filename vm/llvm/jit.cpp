#include "jit.h"

#include "llvm/Analysis/Verifier.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetData.h"
#include "llvm/PassManager.h"
// //#include "llvm/ModuleProvider.h"
//#include "llvm/LinkAllPasses.h"
#include "llvm/Support/StandardPasses.h"
#include "llvm/Support/CommandLine.h"

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

	namespace {
		typedef void (* llvm_throw_type)();
		llvm_throw_type llvm_throw;
	}

	char *llvm_handle_trap = NULL;

	void llvm_trap_handler(neko_vm * vm) {
		llvm_throw();
	}

	void llvm_jit_boot(neko_vm * vm, int_val * code, value, neko_module *) {
		*(char**)vm->start = llvm_handle_trap;
		((void (*)(neko_vm *))code)(vm);
	}

	namespace {
		//this is fixed in next llvm version
		//  but for now it is impossible to call this function with PassManager
		static inline void createStandardFunctionPasses(llvm::PassManager *PM,
														unsigned OptimizationLevel) {
			if (OptimizationLevel > 0) {
				PM->add(llvm::createCFGSimplificationPass());
				if (OptimizationLevel == 1)
					PM->add(llvm::createPromoteMemoryToRegisterPass());
				else
					PM->add(llvm::createScalarReplAggregatesPass());
				PM->add(llvm::createInstructionCombiningPass());
			}
		}
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

		// char * options[] = {"-enable-correct-eh-support"};
		// llvm::cl::ParseCommandLineOptions(sizeof(options)/sizeof(options[0]), options);

		std::string error_string;
		llvm::ExecutionEngine * ee = llvm::EngineBuilder(module)
										.setOptLevel((vm->llvm_optimizations)
													   ?llvm::CodeGenOpt::Aggressive
													   :llvm::CodeGenOpt::None)
										.setEngineKind(llvm::EngineKind::JIT)
										.setErrorStr(&error_string)
										.create();

		//enable lazy compilation
		ee->DisableLazyCompilation(false);
		if (!ee) {
			std::cerr << "Could not create ExecutionEngine: " << error_string << std::endl;
		}

		//register primitives
		#define PRIMITIVE(name) ee->addGlobalMapping(ee->FindFunctionNamed(#name), (void *)p_##name);
		#include "primitives_list.h"
		#undef PRIMITIVE

		llvm::PassManager OurFPM;
		// Set up the optimizer pipeline.  Start with registering info about how the
		// target lays out data structures.
		OurFPM.add(new llvm::TargetData(*ee->getTargetData()));
		createStandardFunctionPasses(&OurFPM, (vm->llvm_optimizations)?3:0);
		llvm::createStandardModulePasses(&OurFPM, (vm->llvm_optimizations)?3:0,
										 false, true,
										 vm->llvm_optimizations, vm->llvm_optimizations,
										 true, 0);

		OurFPM.add(llvm::createLowerInvokePass(0, true));

		OurFPM.run(*module);

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

		llvm::Function * unwind = module->getFunction("throw");
		llvm::verifyFunction(*unwind);
		llvm_throw = (llvm_throw_type)ee->getPointerToFunction(unwind);
		llvm_handle_trap = (char *)llvm_trap_handler;

		m->jit = FPtr;
	}
}
