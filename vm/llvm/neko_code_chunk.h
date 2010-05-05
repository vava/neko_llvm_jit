#pragma once

#include "neko_code_container.h"

#include <vector>
#include <string>

class NekoCodeChunk {
public:
	NekoCodeChunk(neko_code_container const * code_container, int from_address, int to_address);

	int getFromAddress() const { return from_address; }
	int getToAddress() const { return to_address; }
	
	NekoCodeChunk getSubChunk(int to, int from) const;
	std::vector<NekoCodeChunk> splitByAddresses(std::vector<int> const & addresses) const;

	typedef neko_code_container::const_iterator const_iterator;

	const_iterator begin() const { return code_container->lower_bound(from_address); }
	const_iterator end() const { return code_container->lower_bound(to_address); }

	void neko_dump(std::string const & indent = "") const;

private:
	neko_code_container const * code_container;

	int from_address;
	int to_address;
};

