#pragma once

#include "neko_code_chunk.h"
#include "neko_basic_block.h"

typedef struct _vfunction vfunction;

namespace neko {
	class Function {
	public:
		Function(NekoCodeChunk const & code_chunk, std::string const & name, vfunction const * f);
		Function(NekoCodeChunk const & code_chunk, std::string const & name);

		typedef std::vector<BasicBlock> blocks_container;
		typedef blocks_container::iterator iterator;
		typedef blocks_container::const_iterator const_iterator;
		typedef blocks_container::reverse_iterator reverse_iterator;
		typedef blocks_container::const_reverse_iterator const_reverse_iterator;
		typedef blocks_container::value_type value_type;
		typedef blocks_container::size_type size_type;

		iterator begin() { return blocks.begin(); }
		const_iterator begin() const { return blocks.begin(); }
		iterator end() { return blocks.end(); }
		const_iterator end() const { return blocks.end(); }

		reverse_iterator rbegin() { return blocks.rbegin(); }
		const_reverse_iterator rbegin() const { return blocks.rbegin(); }
		reverse_iterator rend() { return blocks.rend(); }
		const_reverse_iterator rend() const { return blocks.rend(); }

		size_type size() const { return blocks.size(); }

		void neko_dump(std::string const & indent = "") const;

		std::string const & getName() const { return name;}
		int getArgumentsCount() const { return nargs; }
	private:
		blocks_container blocks;
		std::string name;
		int nargs;
	};
}
