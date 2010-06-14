#include "neko_code_chunk.h"

#include "assert.h"
#include <limits>
#include "stdio.h"
#include <iostream>
#include <algorithm>

extern "C" {
	#define PARAMETER_TABLE
	#include "../opcodes.h"

	#include "neko_mod.h"
}

NekoCodeChunk::NekoCodeChunk(neko_module const * m_, neko_code_container const * code_container_, ptr_val from_address_, ptr_val to_address_)
	: m(m_)
	, code_container(code_container_)
	, from_address(from_address_)
	, to_address(to_address_)
{
	assert(code_container != NULL);
}

NekoCodeChunk NekoCodeChunk::getSubChunk(ptr_val from, ptr_val to) const {
	assert(from >= from_address && to <= to_address);
	return NekoCodeChunk(m, code_container, from, to);
}

std::vector<NekoCodeChunk> NekoCodeChunk::splitByAddresses(std::vector<ptr_val> const & addresses) const {
	std::vector<ptr_val> local_addresses(addresses);
	local_addresses.push_back(from_address);
	local_addresses.push_back(to_address);

	std::sort(local_addresses.begin(), local_addresses.end());
	local_addresses.erase(std::unique(local_addresses.begin(), local_addresses.end()), local_addresses.end());

	std::vector<NekoCodeChunk> chunks;
	std::vector<ptr_val>::const_iterator end = local_addresses.end() - 1;
	for (std::vector<ptr_val>::const_iterator it = local_addresses.begin();
		 it != end;
		 ++it)
		{
			ptr_val from = *it;
			ptr_val to = *(it + 1);

			chunks.push_back(getSubChunk(from, to));
		}

	return chunks;
}

namespace {
	void print_neko_instruction(enum OPCODE op, int_val p, ptr_val params_count) {
		switch( op ) {
			case AccNull: printf("AccNull"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccTrue: printf("AccTrue"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccFalse: printf("AccFalse"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccThis: printf("AccThis"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccInt: printf("AccInt"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccStack: printf("AccStack"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccGlobal: printf("AccGlobal"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccEnv: printf("AccEnv"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccField: printf("AccField"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccArray: printf("AccArray"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccIndex: printf("AccIndex"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccBuiltin: printf("AccBuiltin"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetStack: printf("SetStack"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetGlobal: printf("SetGlobal"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetEnv: printf("SetEnv"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetField: printf("SetField"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetArray: printf("SetArray"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetIndex: printf("SetIndex"); if (params_count == 1) {printf("(%d)", p);}; break;
			case SetThis: printf("SetThis"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Push: printf("Push"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Pop: printf("Pop"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Call: printf("Call"); if (params_count == 1) {printf("(%d)", p);}; break;
			case ObjCall: printf("ObjCall"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Jump: printf("Jump"); if (params_count == 1) {printf("(%d)", p);}; break;
			case JumpIf: printf("JumpIf"); if (params_count == 1) {printf("(%d)", p);}; break;
			case JumpIfNot: printf("JumpIfNot"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Trap: printf("Trap"); if (params_count == 1) {printf("(%d)", p);}; break;
			case EndTrap: printf("EndTrap"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Ret: printf("Ret"); if (params_count == 1) {printf("(%d)", p);}; break;
			case MakeEnv: printf("MakeEnv"); if (params_count == 1) {printf("(%d)", p);}; break;
			case MakeArray: printf("MakeArray"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Bool: printf("Bool"); if (params_count == 1) {printf("(%d)", p);}; break;
			case IsNull: printf("IsNull"); if (params_count == 1) {printf("(%d)", p);}; break;
			case IsNotNull: printf("IsNotNull"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Add: printf("Add"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Sub: printf("Sub"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Mult: printf("Mult"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Div: printf("Div"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Mod: printf("Mod"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Shl: printf("Shl"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Shr: printf("Shr"); if (params_count == 1) {printf("(%d)", p);}; break;
			case UShr: printf("UShr"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Or: printf("Or"); if (params_count == 1) {printf("(%d)", p);}; break;
			case And: printf("And"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Xor: printf("Xor"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Eq: printf("Eq"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Neq: printf("Neq"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Gt: printf("Gt"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Gte: printf("Gte"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Lt: printf("Lt"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Lte: printf("Lte"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Not: printf("Not"); if (params_count == 1) {printf("(%d)", p);}; break;
			case TypeOf: printf("TypeOf"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Compare: printf("Compare"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Hash: printf("Hash"); if (params_count == 1) {printf("(%d)", p);}; break;
			case New: printf("New"); if (params_count == 1) {printf("(%d)", p);}; break;
			case JumpTable: printf("JumpTable"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Apply: printf("Apply"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccStack0: printf("AccStack0"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccStack1: printf("AccStack1"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccIndex0: printf("AccIndex0"); if (params_count == 1) {printf("(%d)", p);}; break;
			case AccIndex1: printf("AccIndex1"); if (params_count == 1) {printf("(%d)", p);}; break;
			case PhysCompare: printf("PhysCompare"); if (params_count == 1) {printf("(%d)", p);}; break;
			case TailCall: printf("TailCall"); if (params_count == 1) {printf("(%d)", p);}; break;
			case Last: printf("Last"); if (params_count == 1) {printf("(%d)", p);}; break;
			default:
				printf("Something unexpected");
		}
	}

	int bitcount( unsigned int k ) {
		int b = 0;
		while( k ) {
			b++;
			k &= (k - 1);
		}
		return b;
	}
}


#define PARAMETER_TABLE
#include "opcodes.h"

void NekoCodeChunk::neko_dump(std::string const & indent) const {
	for (const_iterator it = begin();
		 it != end();
		 ++it)
		{
			std::cout << indent << it->first << ": ";
			print_neko_instruction((OPCODE) it->second.first, it->second.second, parameter_table[it->second.first]);

			std::cout << "; // ";
			{
				int ppc = (int)((int_val *)it->first - m->code);
				int idx = m->dbgidxs[ppc>>5].base + bitcount(m->dbgidxs[ppc>>5].bits >> (31 - (ppc & 31)));
				value s = val_array_ptr(m->dbgtbl)[idx];
				if( val_is_string(s) )
					printf("%s",val_string(s));
				else if( val_is_array(s) && val_array_size(s) == 2 && val_is_string(val_array_ptr(s)[0]) && val_is_int(val_array_ptr(s)[1]) )
					printf("file %s line %d",val_string(val_array_ptr(s)[0]),val_int(val_array_ptr(s)[1]));
				else
					printf("???");
			}
			std::cout << std::endl;
		}
}
