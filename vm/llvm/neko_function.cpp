#include "neko_function.h"

#include <algorithm>
#include <iostream>

extern "C" {
	#include "../opcodes.h"
}

namespace {
	std::vector<ptr_val> get_block_addresses(NekoCodeChunk const & code_chunk) {
		std::vector<ptr_val> block_addresses;

		for (NekoCodeChunk::const_iterator it = code_chunk.begin(); it != code_chunk.end(); ++it) {
			switch (it->second.first) {
				case Jump:
				case JumpIf:
				case JumpIfNot:
					block_addresses.push_back(it->second.second);
					break;
				default:
					//do nothing
					;
			}
		}

		return block_addresses;
	}

	neko::BasicBlock make_block(NekoCodeChunk const & chunk) {
		return neko::BasicBlock(chunk);
	}

	neko::Function::blocks_container get_blocks(NekoCodeChunk const & chunk) {
		std::vector<ptr_val> block_addresses = get_block_addresses(chunk);
		std::vector<NekoCodeChunk> chunks = chunk.splitByAddresses(block_addresses);

		neko::Function::blocks_container result;
		result.reserve(chunks.size());

		std::transform(chunks.begin(), chunks.end(),
					   std::back_inserter(result),
					   std::ptr_fun(make_block));

		return result;
	}
}

neko::Function::Function(NekoCodeChunk const & code_chunk, std::string const & name_)
	: blocks(get_blocks(code_chunk))
	, name(name_)
{}

void neko::Function::neko_dump(std::string const & indent) const {
	std::cout << indent << "def " << name << " {" << std::endl;
	std::cout << indent << "\t//Number of blocks: " << blocks.size() << std::endl;
	for (const_iterator it = begin();
		 it != end();
		 ++it) {
		it->neko_dump(indent + '\t');
	}
	std::cout << indent << "}" << std::endl;
}
