#include "module_code_base.h"
#include <iostream>

#include <assert.h>
#include <algorithm>

//has to be the last include statement
// because of heavy #define usage
extern "C" {
	#include "../neko_mod.h"
	#include "../opcodes.h"
}

namespace {
	std::vector<int> get_function_addresses(neko_module const * m) {
		std::vector<int> function_addresses;

		for (unsigned int k = 0; k < m->nglobals; k++) {
			if (val_is_function(m->globals[k])) {
				function_addresses.push_back((int)((vfunction*)m->globals[k])->addr);
			}
		}

		//check special case for main function
		if (*m->code == Jump) {
			function_addresses.push_back((int)(m->code + 1));
		} else {
			//no jumps in front means main functions have started right away
			assert(function_addresses.empty()); //maybe we should add additional check though
			function_addresses.push_back((int)m->code);
		}

		std::sort(function_addresses.begin(), function_addresses.end());

		return function_addresses;
	}

	// Function make_function(NekoCodeChunk const & chunk) {
	// 	return Function(chunk);
	// }

	ModuleCodeBase::functions_container get_functions(neko_module const * m, NekoCodeChunk const & chunk) {
		std::vector<int> function_addresses = get_function_addresses(m);
		std::vector<NekoCodeChunk> chunks = chunk.splitByAddresses(function_addresses);

		ModuleCodeBase::functions_container result;

		std::transform(function_addresses.begin(), function_addresses.end(),
					   chunks.begin(),
					   std::inserter(result, result.begin()),
					   std::make_pair<const int, Function>);

		return result;
	}
}

ModuleCodeBase::ModuleCodeBase(neko_module const * m) : code_container(m), functions(get_functions(m, code_container.getNekoCodeChunk()))
{}

void ModuleCodeBase::neko_dump(std::string const & indent) const {
	for (const_iterator it = begin();
		 it != end();
		 ++it) {
		std::cout << "def " << it->first << "() ";
		it->second.neko_dump(indent + "\t");
	}
}
