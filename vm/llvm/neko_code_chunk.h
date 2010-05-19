#pragma once

#include "neko_code_container.h"

#include <vector>
#include <string>
#include <iterator>

class NekoCodeChunk {
public:
	NekoCodeChunk(neko_code_container const * code_container, ptr_val from_address, ptr_val to_address);

	ptr_val getFromAddress() const { return from_address; }
	ptr_val getToAddress() const { return to_address; }

	NekoCodeChunk getSubChunk(ptr_val to, ptr_val from) const;
	std::vector<NekoCodeChunk> splitByAddresses(std::vector<ptr_val> const & addresses) const;

	typedef neko_code_container::const_iterator const_iterator;
	typedef neko_code_container::value_type value_type;
	typedef neko_code_container::size_type size_type;

	const_iterator begin() const { return code_container->lower_bound(from_address); }
	const_iterator end() const { return code_container->lower_bound(to_address); }
	size_type size() const { return std::distance(begin(), end()); }

	void neko_dump(std::string const & indent = "") const;

private:
	neko_code_container const * code_container;

	ptr_val from_address;
	ptr_val to_address;
};

