#include "neko_module.h"

#include "llvm/LLVMContext.h"

#include <iostream>
#include <assert.h>
#include <algorithm>
#include <sstream>
#include <limits>

//has to be the last include statement
// because of heavy #define usage
extern "C" {
	#include "../neko_mod.h"
	#include "../opcodes.h"
}

namespace {
	std::vector<vfunction *> collect_functions(neko_module const * m) {
		std::vector<vfunction *> functions;

		for (ptr_val k = 0; k < m->nglobals; k++) {
			if (val_is_function(m->globals[k])) {
				functions.push_back((vfunction*)m->globals[k]);
			}
		}

		return functions;
	}

	ptr_val get_function_address(vfunction const * f) {
		return (ptr_val) f->addr;
	}

	std::vector<ptr_val> get_function_addresses(neko_module const * m, std::vector<vfunction *> const & functions) {
		std::vector<ptr_val> function_addresses;
		function_addresses.reserve(function_addresses.size() + 1);

		std::transform(functions.begin(), functions.end(),
					   std::back_inserter(function_addresses),
					   std::ptr_fun(get_function_address));

		//check special case for main function
		if (m->codesize > 0 && *m->code == Jump && !function_addresses.empty()) {
			function_addresses.push_back(*(m->code + 1));
		} else {
			//no jumps in front means main functions have started right away
			function_addresses.push_back((ptr_val)m->code);
		}

		return function_addresses;
	}

	neko::Function make_function(NekoCodeChunk const & chunk, vfunction const * f) {
		std::stringstream name;
		name << chunk.getFromAddress();
		return neko::Function(chunk, name.str(), f);
	}

	neko::Module::functions_container get_functions(neko_module const * m, NekoCodeChunk const & chunk) {
		std::vector<vfunction *> const & functions = collect_functions(m);
		std::vector<ptr_val> const & function_addresses = get_function_addresses(m, functions);
		std::vector<NekoCodeChunk> const & chunks = chunk.splitByAddresses(function_addresses);

		std::vector<NekoCodeChunk>::const_iterator begin = chunks.begin();
		if (chunks.size() > 1) {
			//chunks size > 1 means we have more than one function
			//  this also means that the first opcode in the program was a Jump to main function
			//  There's no point making a standalone function for that Jump, so we can just remove it.
			assert(*m->code == Jump);
			assert((int_val *)begin->getToAddress() - (int_val *)begin->getFromAddress() == 2
					 && *(int_val *)begin->getFromAddress() == Jump);
			++begin;
		}

		neko::Module::functions_container result;
		result.reserve(functions.size() + 1);

		std::transform(begin, chunks.end() - 1,
					   functions.begin(),
					   std::back_inserter(result),
					   std::ptr_fun(make_function));

		//add main function
		result.push_back(neko::Function(chunks.back(), std::string("main")));

		return result;
	}
}

neko::Module::Module(neko_module const * m) : code_container(m)
											, functions(get_functions(m, code_container.getNekoCodeChunk()))
{}

void neko::Module::neko_dump(std::string const & indent) const {
	for (const_iterator it = begin();
		 it != end();
		 ++it) {
		it->neko_dump(indent);
	}
}
