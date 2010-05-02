#include "basic_block.h"

void BasicBlock::neko_dump(std::string const & indent) const {
	std::cout << indent << "{" << std::endl;
	chunk->neko_dump(indent + "\t");
	std::cout << indent << "}" << std::endl;
}
