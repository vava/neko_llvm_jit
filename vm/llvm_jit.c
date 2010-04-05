#include "vm.h"
#include "neko_mod.h"
#include "objtable.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifdef NEKO_POSIX
#	include <sys/types.h>
#	include <sys/mman.h>
#	define USE_MMAP
#endif

#define tmp_alloc(size) malloc(size)
#define tmp_free(ptr)	free(ptr)

#if defined(NEKO_X86) && !defined(NEKO_MAC)
#define JIT_ENABLE
#endif

#ifdef NEKO_MAC
#define STACK_ALIGN
#endif

#if defined(NEKO_WINDOWS) && defined(_DEBUG_XX)
#define	STACK_ALIGN
#define STACK_ALIGN_DEBUG
#endif

//#define JIT_DEBUG

#ifdef JIT_ENABLE

#define PARAMETER_TABLE
#include "opcodes.h"

extern void neko_module_jit( neko_module *m );

extern void * llvm_create_new_module();
extern void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count);
extern void* llvm_close_and_get_code(void *);

void neko_llvm_module_jit( neko_module *m ) {
	printf("Hello\n");
	unsigned int i = 0;
	int param = 0;
	int params_count = 0;
	void * module = llvm_create_new_module();
	void * code = 0;

	while( i <= m->codesize ) {
		enum OPCODE op = m->code[i];
		i++;
		param = (int)m->code[i];
		params_count = parameter_table[op];

		llvm_add_op(module, op, param, params_count);

		i += params_count;
	}

	code = llvm_close_and_get_code(module);

	//finally, run normal jit for now
	neko_module_jit( m );
}

#endif
