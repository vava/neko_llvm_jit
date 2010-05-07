#pragma once

#include "neko_code_container.h"

#include <vector>
#include <string>
#include <iterator>

class NekoCodeChunk {
public:
	NekoCodeChunk(neko_code_container const * code_container, unsigned int from_address, unsigned int to_address);

	unsigned int getFromAddress() const { return from_address; }
	unsigned int getToAddress() const { return to_address; }

	NekoCodeChunk getSubChunk(unsigned int to, unsigned int from) const;
	std::vector<NekoCodeChunk> splitByAddresses(std::vector<unsigned int> const & addresses) const;

	typedef neko_code_container::const_iterator const_iterator;
	typedef neko_code_container::value_type value_type;
	typedef neko_code_container::size_type size_type;

	const_iterator begin() const { return code_container->lower_bound(from_address); }
	const_iterator end() const { return code_container->lower_bound(to_address); }
	size_type size() const { return std::distance(begin(), end()); }

	void neko_dump(std::string const & indent = "") const;

private:
	neko_code_container const * code_container;

	unsigned int from_address;
	unsigned int to_address;
};

