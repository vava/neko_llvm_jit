//#include "../opcodes.h"


//C interface
extern "C" {
	typedef struct _neko_module neko_module;
	typedef struct _neko_vm neko_vm;

	void llvm_cpp_jit(neko_vm * vm, neko_module * m);
	// void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count);
	// void* llvm_close_and_get_code(void *);
}

//#include "common.h"
//#include "stack.h"

// #include "llvm/LLVMContext.h"
// #include "llvm/Module.h"
// #include "llvm/ExecutionEngine/ExecutionEngine.h"
// #include <memory>

// class Module {
// public:
// 	Module();
// 	~Module();
// 	void add_new_opcode(OPCODE opcode, int param, int params_count);
// 	void * get_code();
// private:
// 	llvm::LLVMContext & ctx;
// 	Helper h;
// 	llvm::Module * llvmModule;
// 	Builder builder;
// 	llvm::Value * acc;
// 	llvm::Function * main;
// 	const llvm::PointerType * vfunction_struct;
// 	const llvm::PointerType * prim1;
// 	Stack stack;
// 	//owns the module which owns all other expressions.
// 	//  the only thing that has to be deleted
// 	std::auto_ptr<llvm::ExecutionEngine> executionEngine;
// };
