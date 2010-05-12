#include "neko_code_chunk.h"

namespace neko {
	class BasicBlock {
	public:
		BasicBlock(NekoCodeChunk const & chunk_) : chunk(chunk_) {}

		typedef NekoCodeChunk::const_iterator const_iterator;
		typedef neko_code_container::value_type value_type;
		typedef neko_code_container::size_type size_type;

		const_iterator begin() const { return chunk.begin(); }
		const_iterator end() const { return chunk.end(); }
		size_type size() const { return std::distance(begin(), end()); }

		void neko_dump(std::string const & indent = "") const;
	private:
		NekoCodeChunk chunk;
	};
}
