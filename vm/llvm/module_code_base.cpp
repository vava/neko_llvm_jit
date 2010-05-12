#include "module_code_base.h"

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
	std::vector<unsigned int> get_function_addresses(neko_module const * m) {
		std::vector<unsigned int> function_addresses;

		for (unsigned int k = 0; k < m->nglobals; k++) {
			if (val_is_function(m->globals[k])) {
				function_addresses.push_back((unsigned int)((vfunction*)m->globals[k])->addr);
			}
		}

		//check special case for main function
		if (m->codesize > 0 && *m->code == Jump && !function_addresses.empty()) {
			function_addresses.push_back(*(m->code + 1));
		} else {
			//no jumps in front means main functions have started right away
			function_addresses.push_back((unsigned int)m->code);
		}

		return function_addresses;
	}

	std::pair<const unsigned int, neko::Function> make_function(NekoCodeChunk const & chunk) {
		bool isMain = chunk.getToAddress() == std::numeric_limits<unsigned int>::max();
		std::stringstream name;
		if (isMain) {
			name << "main";
		} else {
			name << chunk.getFromAddress();
		}
		return std::make_pair(chunk.getFromAddress(), neko::Function(chunk, name.str()));
	}

	neko::ModuleCodeBase::functions_container get_functions(neko_module const * m, NekoCodeChunk const & chunk) {
		std::vector<unsigned int> const function_addresses = get_function_addresses(m);
		std::vector<NekoCodeChunk> const chunks = chunk.splitByAddresses(function_addresses);

		std::vector<NekoCodeChunk>::const_iterator begin = chunks.begin();
		if (chunks.size() > 1) {
			//chunks size > 1 means we have more than one function
			//  this also means that the first opcode in the program was a Jump to main function
			//  There's no point making a standalone function for that Jump, so we can just remove it.
			assert(*m->code == Jump);
			assert((OPCODE *)begin->getToAddress() - (OPCODE *)begin->getFromAddress() == 2
					 && *(OPCODE *)begin->getFromAddress() == Jump);
			++begin;
		}

		neko::ModuleCodeBase::functions_container result;

		std::transform(begin, chunks.end(),
					   std::inserter(result, result.begin()),
					   std::ptr_fun(make_function));

		return result;
	}
}

neko::ModuleCodeBase::ModuleCodeBase(neko_module const * m) : code_container(m)
													  , functions(get_functions(m, code_container.getNekoCodeChunk()))
													  , name(val_string(m->name))
{}

void neko::ModuleCodeBase::neko_dump(std::string const & indent) const {
	for (const_iterator it = begin();
		 it != end();
		 ++it) {
		it->second.neko_dump(indent);
	}
}
