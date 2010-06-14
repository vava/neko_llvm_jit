#include "neko_code_container.h"
#include "neko_code_chunk.h"

#include <limits>
#include <iostream>

extern "C" {
	#include "../neko_mod.h"

	#define PARAMETER_TABLE
	#include "../opcodes.h"
}

NekoCodeContainer::NekoCodeContainer(neko_module const * m_): m(m_) {
	for (int_val * instruction_address = m->code;
		 instruction_address < m->code + m->codesize;
		 instruction_address += parameter_table[(OPCODE)*instruction_address] + 1 )
		{
			OPCODE op = (OPCODE)*instruction_address;
			int_val param = *(instruction_address + parameter_table[op]);

			opcodes.insert(std::make_pair((ptr_val)instruction_address, std::make_pair(op, param)));
		}
}

NekoCodeChunk NekoCodeContainer::getNekoCodeChunk() const {
	return NekoCodeChunk(m, &opcodes, opcodes.begin()->first, std::numeric_limits<ptr_val>::max());
}
