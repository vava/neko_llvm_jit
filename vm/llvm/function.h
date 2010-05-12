#include "neko_code_chunk.h"
#include "basic_block.h"

namespace neko {
	class Function {
	public:
		Function(NekoCodeChunk const & code_chunk, std::string const & name);

		typedef std::map<unsigned int, BasicBlock> blocks_container;
		typedef blocks_container::iterator iterator;
		typedef blocks_container::const_iterator const_iterator;
		typedef blocks_container::value_type value_type;
		typedef blocks_container::size_type size_type;

		iterator begin() { return blocks.begin(); }
		const_iterator begin() const { return blocks.begin(); }
		iterator end() { return blocks.end(); }
		const_iterator end() const { return blocks.end(); }
		size_type size() const { return blocks.size(); }

		void neko_dump(std::string const & indent = "") const;

		std::string const & getName() const { return name;}
	private:
		blocks_container blocks;
		std::string name;
	};
}
