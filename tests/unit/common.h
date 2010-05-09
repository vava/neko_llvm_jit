#include <memory>
#include <vector>

extern "C" {
	#include "opcodes.h"
}
typedef struct _neko_module neko_module;
typedef struct _value *value;

neko_module * makeNekoModule(OPCODE * opcodes, int size);

class NekoValueHolder {
public:
	NekoValueHolder() {}
	~NekoValueHolder();
	value makeFunction(void * addr, int nargs, neko_module * nm);

private:
	template<typename T>
	T * makeValue() {
		T * result = new T;
		values.push_back((value)result);
		return result;
	}

	std::vector<value> values;
};

class NekoModuleWrapper {
public:
	template<int codesize, int nglobals>
	NekoModuleWrapper(int (&code_)[codesize], value (&globals_)[nglobals])
		: code(code_, code_ + codesize)
		, globals(globals_, globals_ + nglobals)
		, module(make_module())
	{}

	neko_module * get() {
		return module.get();
	}
private:
	void patch_jumps(std::vector<int> & code, unsigned int address_base) const;
	void patch_globals(std::vector<value> & globals, unsigned int address_base, neko_module * nm) const;
	neko_module * make_module();

	std::vector<int> code;
	std::vector<value> globals;
	std::auto_ptr<neko_module> module;
};
