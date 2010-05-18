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

/* #define PARAMETER_TABLE */
/* #include "opcodes.h" */

extern void neko_module_jit( neko_module *m );

extern void llvm_cpp_jit( neko_vm * vm, neko_module const * m );

/* extern void * llvm_create_new_module(); */
/* extern void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count); */
/* extern void* llvm_close_and_get_code(void *); */

/* void get_function_addresses(neko_module *m) { */
/* 	int k = 0; */

/* 	while (true) { */
/* 		while( k < m->nglobals && !val_is_function(m->globals[k]) ) */
/* 			k++; */
/* 		if( k >= m->nglobals ) { */
/* 			break; */
/* 		} */
/* 		func_count++; */
/* 		k++; */
/* 	} */
/* } */

void neko_llvm_module_jit( neko_vm * vm, neko_module *m ) {
	llvm_cpp_jit(vm, m);
	//finally, run normal jit for now
	//	neko_module_jit( m );
}

#endif
