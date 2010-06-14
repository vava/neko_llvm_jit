#pragma once

#include "common.h"

#include <map>

class NekoCodeChunk;

typedef std::map<ptr_val, std::pair<ptr_val, int_val> > neko_code_container;

class NekoCodeContainer {
public:
	NekoCodeContainer(neko_module const * m);

	NekoCodeChunk getNekoCodeChunk() const;

private:
	neko_code_container opcodes;
	neko_module const * m;
};
