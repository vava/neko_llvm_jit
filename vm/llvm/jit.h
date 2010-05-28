//#include "../opcodes.h"


//C interface
extern "C" {
	typedef struct _neko_module neko_module;
	typedef struct _neko_vm neko_vm;

	void llvm_cpp_jit(neko_vm * vm, neko_module * m);
}
