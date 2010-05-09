#define PARAMETER_TABLE
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

NekoValueHolder::~NekoValueHolder() {
	for (std::vector<value>::iterator it = values.begin();
		 it != values.end();
		 ++it)
		{
			delete *it;
		}
}

value NekoValueHolder::makeFunction(void * addr, int nargs, neko_module * nm) {
	vfunction * f = makeValue<vfunction>();

	f->t = VAL_FUNCTION;
	f->nargs = nargs;
	f->addr = addr;
	f->env = NULL;
	f->module = nm;

	return (value)f;
}

void NekoModuleWrapper::patch_jumps(std::vector<int> & code, unsigned int address_base) const {
	for (std::vector<int>::iterator pc = code.begin();
		 pc != code.end();
		 ++pc)
		{
			switch (*pc) {
				case Jump:
				case JumpIf:
				case JumpIfNot:
					*(pc + 1) += address_base;
			}

			pc += parameter_table[*pc];
		}
}

void NekoModuleWrapper::patch_globals(std::vector<value> & globals, unsigned int address_base, neko_module * nm) const {
	for (std::vector<value>::iterator pc = globals.begin();
		 pc != globals.end();
		 ++pc)
		{
			if (val_is_function(*pc)) {
				vfunction * f = (vfunction *)*pc;
				f->module = nm;
				f->addr = (void *)(((unsigned int *)f->addr) + address_base);
			}
		}
}

neko_module * NekoModuleWrapper::make_module() {
	neko_module * module = new neko_module;

	//deal with code
	patch_jumps(code, (unsigned int)&code[0]);

	//deal with globals
	patch_globals(globals, (unsigned int)&code[0], module);

	//set up module
	module->codesize = code.size();
	module->code = (int_val *)&code[0];
	module->nglobals = globals.size();
	module->globals = &globals[0];

	return module;
}
