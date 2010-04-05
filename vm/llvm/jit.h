#include "../opcodes.h"

//C interface
extern "C" {
	void * llvm_create_new_module();
	void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count);
	void* llvm_close_and_get_code(void *);
}

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <memory>

class Module {
public:
	Module();
	~Module();
	void add_new_opcode(OPCODE opcode, int param, int params_count);
	void * get_code();
private:
	llvm::LLVMContext & ctx;
	llvm::Module * llvmModule;
	llvm::IRBuilder<> builder;
	llvm::IntegerType const * const intType;
	llvm::Value * acc;
	//owns the module which owns all other expressions.
	//  the only thing that has to be deleted
	std::auto_ptr<llvm::ExecutionEngine> executionEngine;
};
