#include <memory>
#include <vector>
#include <list>
#include <string>

extern "C" {
	#include "opcodes.h"
}

#include "llvm/common.h"

typedef struct _value *value;

neko_module * makeNekoModule(int_val * opcodes, int_val size);

class NekoValueHolder {
public:
	NekoValueHolder() {}
	~NekoValueHolder();
	value makeFunction(void * addr, int_val nargs, neko_module * nm);
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
	template<int_val codesize, int_val nglobals>
	NekoModuleWrapper(value name, int_val (&code_)[codesize], value (&globals_)[nglobals])
		: code(code_, code_ + codesize)
		, globals(globals_, globals_ + nglobals)
		, module(make_module(name))
	{}

	template<int_val codesize>
	NekoModuleWrapper(value name, int_val (&code_)[codesize], value (&)[0])
		: code(code_, code_ + codesize)
		, globals()
		, module(make_module(name))
	{}

	NekoModuleWrapper(value name, int_val (&)[0], value (&)[0])
		: code()
		, globals()
		, module(make_module(name))
	{}

	neko_module * get() {
		return module.get();
	}
private:
	void patch_jumps(std::vector<int_val> & code, int_val * address_base) const;
	void patch_globals(std::vector<value> & globals, int_val * address_base, neko_module * nm) const;
	neko_module * make_module(value name);

	std::vector<int_val> code;
	std::vector<value> globals;
	std::auto_ptr<neko_module> module;
};
