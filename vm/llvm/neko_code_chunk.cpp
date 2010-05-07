#include "neko_code_chunk.h"

#include "assert.h"
#include <limits>
#include "stdio.h"
#include <iostream>
#include <algorithm>

extern "C" {
	#define PARAMETER_TABLE
	#include "../opcodes.h"
}

NekoCodeChunk::NekoCodeChunk(neko_code_container const * code_container_, unsigned int from_address_, unsigned int to_address_)
	: code_container(code_container_)
	, from_address(from_address_)
	, to_address(to_address_)
{
	assert(code_container != NULL);
}

NekoCodeChunk NekoCodeChunk::getSubChunk(unsigned int from, unsigned int to) const {
	std::cout << to << " " << from << std::endl;
	std::cout << to_address << " " << from_address << std::endl;
	assert(from >= from_address && to <= to_address);
	return NekoCodeChunk(code_container, from, to);
}

std::vector<NekoCodeChunk> NekoCodeChunk::splitByAddresses(std::vector<unsigned int> const & addresses) const {
	std::vector<unsigned int> local_addresses(addresses);
	local_addresses.push_back(from_address);
	local_addresses.push_back(to_address);

	std::sort(local_addresses.begin(), local_addresses.end());
	local_addresses.erase(std::unique(local_addresses.begin(), local_addresses.end()), local_addresses.end());

	std::vector<NekoCodeChunk> chunks;
	std::vector<unsigned int>::const_iterator end = local_addresses.end() - 1;
	for (std::vector<unsigned int>::const_iterator it = local_addresses.begin();
		 it != end;
		 ++it)
		{
			unsigned int from = *it;
			unsigned int to = *(it + 1);

			chunks.push_back(getSubChunk(from, to));
		}

	return chunks;
}

namespace {
	void print_neko_instruction(enum OPCODE op, int p, unsigned int params_count) {
		switch( op ) {
			case AccNull: printf("AccNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccTrue: printf("AccTrue"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccFalse: printf("AccFalse"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccThis: printf("AccThis"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccInt: printf("AccInt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccStack: printf("AccStack"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccGlobal: printf("AccGlobal"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccEnv: printf("AccEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccField: printf("AccField"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccArray: printf("AccArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccIndex: printf("AccIndex"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccBuiltin: printf("AccBuiltin"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetStack: printf("SetStack"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetGlobal: printf("SetGlobal"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetEnv: printf("SetEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetField: printf("SetField"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetArray: printf("SetArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetIndex: printf("SetIndex"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case SetThis: printf("SetThis"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Push: printf("Push"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Pop: printf("Pop"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Call: printf("Call"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case ObjCall: printf("ObjCall"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Jump: printf("Jump"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case JumpIf: printf("JumpIf"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case JumpIfNot: printf("JumpIfNot"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Trap: printf("Trap"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case EndTrap: printf("EndTrap"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Ret: printf("Ret"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case MakeEnv: printf("MakeEnv"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case MakeArray: printf("MakeArray"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Bool: printf("Bool"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case IsNull: printf("IsNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case IsNotNull: printf("IsNotNull"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Add: printf("Add"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Sub: printf("Sub"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Mult: printf("Mult"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Div: printf("Div"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Mod: printf("Mod"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Shl: printf("Shl"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Shr: printf("Shr"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case UShr: printf("UShr"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Or: printf("Or"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case And: printf("And"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Xor: printf("Xor"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Eq: printf("Eq"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Neq: printf("Neq"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Gt: printf("Gt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Gte: printf("Gte"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Lt: printf("Lt"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Lte: printf("Lte"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Not: printf("Not"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case TypeOf: printf("TypeOf"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Compare: printf("Compare"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Hash: printf("Hash"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case New: printf("New"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case JumpTable: printf("JumpTable"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Apply: printf("Apply"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccStack0: printf("AccStack0"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccStack1: printf("AccStack1"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccIndex0: printf("AccIndex0"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case AccIndex1: printf("AccIndex1"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case PhysCompare: printf("PhysCompare"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case TailCall: printf("TailCall"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			case Last: printf("Last"); if (params_count == 1) {printf("(%d)", p);}; printf("\n"); break;
			default:
				printf("Something unexpected\n");
		}
	}
}


#define PARAMETER_TABLE
#include "opcodes.h"

void NekoCodeChunk::neko_dump(std::string const & indent) const {
	for (const_iterator it = begin();
		 it != end();
		 ++it)
		{
			std::cout << indent;
			print_neko_instruction((OPCODE) it->second.first, it->second.second, parameter_table[it->second.first]);
		}
}
