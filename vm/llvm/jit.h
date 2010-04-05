#include "../opcodes.h"

//C interface
extern "C" {
	void * llvm_create_new_module();
	void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count);
	void* llvm_close_and_get_code(void *);
}

#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"

class Module {
public:
	Module();
	~Module();
	void add_new_opcode(OPCODE opcode, int param, int params_count);
	void * get_code();
private:
	llvm::Module * llvmModule;
	llvm::IRBuilder<> builder;
};
