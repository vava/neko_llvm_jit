#include "neko_code_chunk.h"
#include "basic_block.h"

class Function {
public:
	Function(NekoCodeChunk const & code_chunk);

	typedef std::map<unsigned int, BasicBlock> blocks_container;
	typedef blocks_container::iterator iterator;
	typedef blocks_container::const_iterator const_iterator;

	iterator begin() { return blocks.begin(); }
	const_iterator begin() const { return blocks.begin(); }
	iterator end() { return blocks.end(); }
	const_iterator end() const { return blocks.end(); }

 	void neko_dump(std::string const & indent = "") const;
private:
	blocks_container blocks;
};
