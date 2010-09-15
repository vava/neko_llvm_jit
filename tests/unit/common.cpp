#define PARAMETER_TABLE
#include "common.h"
#include <string.h>

extern "C" {
	#include "neko_mod.h"
}

neko_module * makeNekoModule(int_val * opcodes, int_val size) {
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

value NekoValueHolder::makeFunction(void * addr, int_val nargs, neko_module * nm) {
	vfunction * f = makeValue<vfunction>();

	f->t = VAL_FUNCTION;
	f->nargs = nargs;
	f->addr = addr;
	f->env = NULL;
	f->module = nm;

	return (value)f;
}

value NekoValueHolder::makeString(std::string const & str) {
	vstring * s = (vstring *)::operator new(sizeof(vstring) + str.size());
	values.push_back((value)s);

	s->t = (val_type)(VAL_STRING | (str.size() << 3));
	memcpy(&s->c, str.c_str(), str.size() + 1);

	return (value)s;
}

void NekoModuleWrapper::patch_jumps(std::vector<int_val> & code, int_val * address_base) const {
	for (std::vector<int_val>::iterator pc = code.begin();
		 pc != code.end();
		 ++pc)
		{
			switch (*pc) {
				case Jump:
				case JumpIf:
				case JumpIfNot:
					*(pc + 1) = (int_val)(address_base + *(pc + 1));
			}

			pc += parameter_table[*pc];
		}
}

void NekoModuleWrapper::patch_globals(std::vector<value> & globals, int_val * address_base, neko_module * nm) const {
	for (std::vector<value>::iterator pc = globals.begin();
		 pc != globals.end();
		 ++pc)
		{
			if (val_is_function(*pc)) {
				vfunction * f = (vfunction *)*pc;
				f->module = nm;
				f->addr = address_base + (int_val)f->addr;
			}
		}
}

neko_module * NekoModuleWrapper::make_module(value name) {
	neko_module * module = new neko_module;
	module->name = name;

	patch_jumps(code, &code[0]);
	patch_globals(globals, &code[0], module);

	//set up module
	module->codesize = code.size();
	module->code = (int_val *)&code[0];
	module->nglobals = globals.size();
	module->globals = &globals[0];

	return module;
}

extern "C"
int_val * neko_get_ttable() {
	return NULL;
}
