#pragma once

//#include "../opcodes.h"

#include <map>

class NekoCodeChunk;
typedef struct _neko_module neko_module;

typedef std::map<unsigned int, std::pair<unsigned int, int> > neko_code_container;

class NekoCodeContainer {
public:
	NekoCodeContainer(neko_module const * m);

	NekoCodeChunk getNekoCodeChunk() const;
private:
	neko_code_container opcodes;
};
