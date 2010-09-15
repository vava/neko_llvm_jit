#include "neko_code_container.h"
#include "neko_code_chunk.h"

#include <limits>
#include <iostream>
#include <stdio.h>

extern "C" {
	#include "../neko_mod.h"
	#include "../vm.h"

	#define PARAMETER_TABLE
	#include "../opcodes.h"
	#undef PARAMETER_TABLE
    #undef OP
    #undef OPBEGIN
    #undef OPEND
}

namespace {
	class NekoThreadingOpcodesUnscrambler {
	public:
		NekoThreadingOpcodesUnscrambler() {
			int_val *jtbl = neko_get_ttable();
			(
			#undef _NEKO_OPCODES_H
			#define OP(x) unscramle_map.insert(std::make_pair(jtbl[x], x))
			#define OPBEGIN
			#define OPEND
			#include "../opcodes.h"
			#undef OP
			0);
		}

		OPCODE get_op(int_val scrambled) {
			std::map<int_val, OPCODE>::const_iterator it = unscramle_map.find(scrambled);
			if (it == unscramle_map.end()) {
				val_throw(alloc_string("Unscramled failed"));
				return Last;
			} else {
				return it->second;
			}
		}

	private:
		std::map<int_val, OPCODE> unscramle_map;
	};

	OPCODE unscramble(int_val scrambled) {
		static NekoThreadingOpcodesUnscrambler unscrambler;
		return unscrambler.get_op(scrambled);
	}
}

NekoCodeContainer::NekoCodeContainer(neko_module const * m_): m(m_) {
	OPCODE op;

	for (int_val * instruction_address = m->code;
		 instruction_address < m->code + m->codesize;
		 instruction_address += parameter_table[op] + 1 )
		{
			op = (OPCODE)*instruction_address;
			if (op > Last) {
				//we're looking at scrambled by NEKO_DIRECT_THREADING opcode,
				//  unscramble it
				op = unscramble(op);
			}
			int_val param = *(instruction_address + parameter_table[op]);

			opcodes.insert(std::make_pair((ptr_val)instruction_address, std::make_pair(op, param)));
		}
}

NekoCodeChunk NekoCodeContainer::getNekoCodeChunk() const {
	return NekoCodeChunk(m, &opcodes, opcodes.begin()->first, std::numeric_limits<ptr_val>::max());
}
