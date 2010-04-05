#include "jit.h"
#include <stdio.h>

//C interface
extern "C" {
	void * llvm_create_new_module() {
		return new Module();
	}

	void llvm_add_op(void * m, enum OPCODE opcode, int param, int params_count) {
		static_cast<Module*>(m)->add_new_opcode(opcode, param, params_count);
	}

	void* llvm_close_and_get_code(void *m) {
		Module * module = static_cast<Module*>(m);
		void * code = module->get_code();
		delete module;
		return code;
	}
}

Module::Module() {
}

Module::~Module() {
}

void print_neko_instruction(enum OPCODE op, int p, int params_count);

void Module::add_new_opcode(OPCODE opcode, int param, int params_count) {
	print_neko_instruction(opcode, param, params_count);
}

void * Module::get_code() {
	return 0;
}

// #include "llvm/Support/IRBuilder.h"
// #include "llvm/Module.h"

void print_neko_instruction(enum OPCODE op, int p, int params_count) {
	switch( op ) {
		case AccNull: printf("AccNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccTrue: printf("AccTrue"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccFalse: printf("AccFalse"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccThis: printf("AccThis"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccInt: printf("AccInt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack: printf("AccStack"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccGlobal: printf("AccGlobal"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccEnv: printf("AccEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccField: printf("AccField"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccArray: printf("AccArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex: printf("AccIndex"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccBuiltin: printf("AccBuiltin"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetStack: printf("SetStack"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetGlobal: printf("SetGlobal"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetEnv: printf("SetEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetField: printf("SetField"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetArray: printf("SetArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetIndex: printf("SetIndex"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case SetThis: printf("SetThis"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Push: printf("Push"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Pop: printf("Pop"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Call: printf("Call"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case ObjCall: printf("ObjCall"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Jump: printf("Jump"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpIf: printf("JumpIf"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpIfNot: printf("JumpIfNot"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Trap: printf("Trap"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case EndTrap: printf("EndTrap"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Ret: printf("Ret"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case MakeEnv: printf("MakeEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case MakeArray: printf("MakeArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Bool: printf("Bool"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case IsNull: printf("IsNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case IsNotNull: printf("IsNotNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Add: printf("Add"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Sub: printf("Sub"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Mult: printf("Mult"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Div: printf("Div"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Mod: printf("Mod"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Shl: printf("Shl"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Shr: printf("Shr"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case UShr: printf("UShr"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Or: printf("Or"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case And: printf("And"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Xor: printf("Xor"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Eq: printf("Eq"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Neq: printf("Neq"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Gt: printf("Gt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Gte: printf("Gte"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Lt: printf("Lt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Lte: printf("Lte"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Not: printf("Not"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case TypeOf: printf("TypeOf"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Compare: printf("Compare"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Hash: printf("Hash"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case New: printf("New"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case JumpTable: printf("JumpTable"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Apply: printf("Apply"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack0: printf("AccStack0"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccStack1: printf("AccStack1"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex0: printf("AccIndex0"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case AccIndex1: printf("AccIndex1"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case PhysCompare: printf("PhysCompare"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case TailCall: printf("TailCall"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		case Last: printf("Last"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
		default:
			printf("Something unexpected\n");
	}
}
