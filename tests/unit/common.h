#include <memory>
#include <vector>
#include <list>
#include <string>

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
	value makeString(std::string const & str);

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
	NekoModuleWrapper(value name, int (&code_)[codesize], value (&globals_)[nglobals])
		: code(code_, code_ + codesize)
		, globals(globals_, globals_ + nglobals)
		, module(make_module(name))
	{}

	template<int codesize>
	NekoModuleWrapper(value name, int (&code_)[codesize], value (&)[0])
		: code(code_, code_ + codesize)
		, globals()
		, module(make_module(name))
	{}

	NekoModuleWrapper(value name, int (&)[0], value (&)[0])
		: code()
		, globals()
		, module(make_module(name))
	{}

	neko_module * get() {
		return module.get();
	}
private:
	void patch_jumps(std::vector<int> & code, int * address_base) const;
	void patch_globals(std::vector<value> & globals, int * address_base, neko_module * nm) const;
	neko_module * make_module(value name);

	std::vector<int> code;
	std::vector<value> globals;
	std::auto_ptr<neko_module> module;
};
