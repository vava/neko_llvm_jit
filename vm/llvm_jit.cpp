#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifdef NEKO_POSIX
#	include <sys/types.h>
#	include <sys/mman.h>
#	define USE_MMAP
#endif

extern "C" {
#include "vm.h"
#include "neko_mod.h"
#include "objtable.h"

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

#define PARAMETER_TABLE
#include "opcodes.h"
#undef PARAMETER_TABLE
#undef OP
#undef OPBEGIN
#undef OPEND
#undef _NEKO_OPCODES_H

#ifdef JIT_ENABLE

extern void neko_module_jit( neko_module *m );
}

void print_neko_instruction(enum OPCODE op, int p);

extern "C" void neko_llvm_module_jit( neko_module *m ) {
	printf("Hello from llvm jit\n");
	unsigned int i = 0;
	int param = 0;

	while( i <= m->codesize ) {
		enum OPCODE op = static_cast<OPCODE>(m->code[i]);
		i++;
		param = (int)m->code[i];
		print_neko_instruction(op, param);
		i += parameter_table[op];
	}
	//finally, run normal jit for now
	neko_module_jit( m );
}

void print_neko_instruction(enum OPCODE op, int p) {
	switch( op ) {
		case AccNull: printf("AccNull"); if (parameter_table[AccNull] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccTrue: printf("AccTrue"); if (parameter_table[AccTrue] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccFalse: printf("AccFalse"); if (parameter_table[AccFalse] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccThis: printf("AccThis"); if (parameter_table[AccThis] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccInt: printf("AccInt"); if (parameter_table[AccInt] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack: printf("AccStack"); if (parameter_table[AccStack] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccGlobal: printf("AccGlobal"); if (parameter_table[AccGlobal] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccEnv: printf("AccEnv"); if (parameter_table[AccEnv] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccField: printf("AccField"); if (parameter_table[AccField] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccArray: printf("AccArray"); if (parameter_table[AccArray] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex: printf("AccIndex"); if (parameter_table[AccIndex] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccBuiltin: printf("AccBuiltin"); if (parameter_table[AccBuiltin] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetStack: printf("SetStack"); if (parameter_table[SetStack] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetGlobal: printf("SetGlobal"); if (parameter_table[SetGlobal] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetEnv: printf("SetEnv"); if (parameter_table[SetEnv] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetField: printf("SetField"); if (parameter_table[SetField] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetArray: printf("SetArray"); if (parameter_table[SetArray] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetIndex: printf("SetIndex"); if (parameter_table[SetIndex] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetThis: printf("SetThis"); if (parameter_table[SetThis] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Push: printf("Push"); if (parameter_table[Push] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Pop: printf("Pop"); if (parameter_table[Pop] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Call: printf("Call"); if (parameter_table[Call] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case ObjCall: printf("ObjCall"); if (parameter_table[ObjCall] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Jump: printf("Jump"); if (parameter_table[Jump] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpIf: printf("JumpIf"); if (parameter_table[JumpIf] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpIfNot: printf("JumpIfNot"); if (parameter_table[JumpIfNot] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Trap: printf("Trap"); if (parameter_table[Trap] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case EndTrap: printf("EndTrap"); if (parameter_table[EndTrap] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Ret: printf("Ret"); if (parameter_table[Ret] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case MakeEnv: printf("MakeEnv"); if (parameter_table[MakeEnv] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case MakeArray: printf("MakeArray"); if (parameter_table[MakeArray] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Bool: printf("Bool"); if (parameter_table[Bool] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case IsNull: printf("IsNull"); if (parameter_table[IsNull] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case IsNotNull: printf("IsNotNull"); if (parameter_table[IsNotNull] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Add: printf("Add"); if (parameter_table[Add] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Sub: printf("Sub"); if (parameter_table[Sub] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Mult: printf("Mult"); if (parameter_table[Mult] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Div: printf("Div"); if (parameter_table[Div] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Mod: printf("Mod"); if (parameter_table[Mod] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Shl: printf("Shl"); if (parameter_table[Shl] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Shr: printf("Shr"); if (parameter_table[Shr] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case UShr: printf("UShr"); if (parameter_table[UShr] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Or: printf("Or"); if (parameter_table[Or] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case And: printf("And"); if (parameter_table[And] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Xor: printf("Xor"); if (parameter_table[Xor] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Eq: printf("Eq"); if (parameter_table[Eq] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Neq: printf("Neq"); if (parameter_table[Neq] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Gt: printf("Gt"); if (parameter_table[Gt] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Gte: printf("Gte"); if (parameter_table[Gte] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Lt: printf("Lt"); if (parameter_table[Lt] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Lte: printf("Lte"); if (parameter_table[Lte] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Not: printf("Not"); if (parameter_table[Not] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case TypeOf: printf("TypeOf"); if (parameter_table[TypeOf] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Compare: printf("Compare"); if (parameter_table[Compare] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Hash: printf("Hash"); if (parameter_table[Hash] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case New: printf("New"); if (parameter_table[New] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpTable: printf("JumpTable"); if (parameter_table[JumpTable] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Apply: printf("Apply"); if (parameter_table[Apply] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack0: printf("AccStack0"); if (parameter_table[AccStack0] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack1: printf("AccStack1"); if (parameter_table[AccStack1] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex0: printf("AccIndex0"); if (parameter_table[AccIndex0] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex1: printf("AccIndex1"); if (parameter_table[AccIndex1] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case PhysCompare: printf("PhysCompare"); if (parameter_table[PhysCompare] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case TailCall: printf("TailCall"); if (parameter_table[TailCall] == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Last: printf("Last"); if (parameter_table[Last] == 1) {printf("(%d)", p);}; printf("\n"); break;
		default:
			printf("Something unexpected\n");
	}
}
#endif
