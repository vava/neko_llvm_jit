#include "basic_block.h"

#include <iostream>

void neko::BasicBlock::neko_dump(std::string const & indent) const {
	std::cout << "{" << std::endl;
	std::cout << indent << "\t//block is from " << chunk.getFromAddress() << " to " << chunk.getToAddress() << std::endl;
	chunk.neko_dump(indent + "\t");
	std::cout << indent << "}" << std::endl;
}
