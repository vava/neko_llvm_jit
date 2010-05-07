#include "common.h"

extern "C" {
	#include "neko_mod.h"
}

neko_module * makeNekoModule(OPCODE * opcodes, int size) {
	neko_module * nm = new neko_module;
	nm->codesize = size;
	nm->code = (int_val *)opcodes;

	return nm;
}
