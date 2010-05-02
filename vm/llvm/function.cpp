#include "function.h"

#include <algorithm>
#include <iostream>

namespace {
	std::vector<int> get_block_addresses(NekoCodeChunk const & code_chunk) {
		std::vector<int> block_addresses;

		for (NekoCodeChunk::const_iterator it = code_chunk.begin(); it != code_chunk.end(); ++it) {
			switch (it->second.first) {
				case Jump:
				case JumpIf:
				case JumpIfNot:
					block_addresses.push_back(it->first);
					break;
				default:
					//do nothing
					;
			}
		}

		std::sort(block_addresses.begin(), block_addresses.end());

		return block_addresses;
	}

	Function::blocks_container get_blocks(NekoCodeChunk const & chunk) {
		std::vector<int> block_addresses = get_block_addresses(chunk);
		std::vector<NekoCodeChunk> chunks = chunk.splitByAddresses(block_addresses);

		Function::blocks_container result;

		std::transform(block_addresses.begin(), block_addresses.end(),
					   chunks.begin(),
					   std::inserter(result, result.begin()),
					   std::make_pair<const int, BasicBlock>);

		return result;
	}
}

Function::Function(NekoCodeChunk const & code_chunk) : blocks(get_blocks(code_chunk))
{}

void Function::neko_dump(std::string const & indent) const {
	std::cout << indent << "{" << std::endl;
	for (const_iterator it = begin();
		 it != end();
		 ++it) {
		std::cout << it->first << " : ";
		it->second.neko_dump(indent + "\t");
	}
	std::cout << indent << "}" << std::endl;
}
