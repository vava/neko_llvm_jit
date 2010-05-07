extern "C" {
	#include "opcodes.h"
}
typedef struct _neko_module neko_module;

neko_module * makeNekoModule(OPCODE * opcodes, int size);
