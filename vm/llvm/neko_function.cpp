#include "neko_function.h"

#include <algorithm>
#include <iostream>

extern "C" {
	#include "../opcodes.h"
	#include "../neko.h"
}

namespace {
	std::vector<ptr_val> get_block_addresses(NekoCodeChunk const & code_chunk) {
		std::vector<ptr_val> block_addresses;

		for (NekoCodeChunk::const_iterator it = code_chunk.begin(); it != code_chunk.end(); ++it) {
			switch (it->second.first) {
				case Trap:
				case Jump:
					block_addresses.push_back(it->second.second);
					break;
				case JumpIf:
				case JumpIfNot:
					{
						block_addresses.push_back(it->second.second);
						//in llvm jump always have 'then' and 'else' parts
						NekoCodeChunk::const_iterator next = it; ++next;
						if (next != code_chunk.end()) {
							block_addresses.push_back(next->first);
						}
					}
					break;
				case JumpTable:
					{
						//JumpTable usually have structure as so:
						//   JumpTable(4); /*4 means what to add to address in order to jump to default*/
						//   /*case 0:*/Jump(address);
						//   /*case 1:*/Jump(address);
						//   /*default:*/

						//skip all the jumps
						NekoCodeChunk::const_iterator def_block = it;
						std::advance(def_block, it->second.second / 2 + 1);

						//add address of default block
						block_addresses.push_back(def_block->first);
					}
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
		std::vector<ptr_val> const & block_addresses = get_block_addresses(chunk);
		std::vector<NekoCodeChunk> const & chunks = chunk.splitByAddresses(block_addresses);

		neko::Function::blocks_container result;
		result.reserve(chunks.size());

		std::transform(chunks.begin(), chunks.end(),
					   std::back_inserter(result),
					   std::ptr_fun(make_block));

		return result;
	}
}

neko::Function::Function(NekoCodeChunk const & code_chunk, std::string const & name_, vfunction const * f)
	: blocks(get_blocks(code_chunk))
	, name(name_)
	, nargs(f->nargs)
{}

neko::Function::Function(NekoCodeChunk const & code_chunk, std::string const & name_)
	: blocks(get_blocks(code_chunk))
	, name(name_)
	, nargs(0)
{}

void neko::Function::neko_dump(std::string const & indent) const {
	std::cout << indent << "def " << name << "(";
	for (int i = 0; i < nargs; i++) {
		if (i != 0) {
			std::cout << ", ";
		}
		std::cout << "int";
	}
	std::cout << ") {" << std::endl;
	std::cout << indent << "\t//Number of blocks: " << blocks.size() << std::endl;
	for (const_iterator it = begin();
		 it != end();
		 ++it) {
		it->neko_dump(indent + '\t');
	}
	std::cout << indent << "}" << std::endl;
}
