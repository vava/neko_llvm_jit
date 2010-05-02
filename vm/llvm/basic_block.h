class BasicBlock {
public:
	BasicBlock(NekoCodeChunk const & chunk_) : chunk(chunk_) {}

	typedef NekoCodeChunk::const_iterator const_iterator;

	const_iterator begin() const { return chunk.begin(); }
	const_iterator end() const { return chunk.end(); }

	void neko_dump(std::string const & indent = "") const;
private:
	NekoCodeChunk const & chunk;
};
