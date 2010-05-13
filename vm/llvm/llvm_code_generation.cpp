#include "llvm_code_generation.h"

llvm::Module * makeLLVMModule(neko::Module const & neko_module) {
	llvm::Module * module = new ::llvm::Module(neko_module.getName(), llvm::getGlobalContext());

	for (neko::Module::const_iterator it = neko_module.begin();
		 it != neko_module.end();
		 ++it)
		{
			makeFunctionDeclaration(it->second, module);
		}

	for (neko::Module::const_iterator it = neko_module.begin();
		 it != neko_module.end();
		 ++it)
		{
			module->getOrInsertFunction(it->second.getName(), makeFunction(it->second, module));
		}

	return module;
}

llvm::FunctionType * makeFunctionDeclaration(neko::Function const & neko_function, llvm::Module * module) {
    llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()), std::vector<const llvm::Type *>(), false);
	llvm::Function * F = llvm::Function::Create(FT,
												llvm::Function::ExternalLinkage,
												neko_function.getName(),
												module);
	return F;
}

typedef id2block_type std::map<unsigned int, llvm::BasicBlock *>;

llvm::Function * makeFunction(neko::Function const & neko_function, llvm::Module * module) {
	llvm::Function * function = module->getFunction(neko_function.getName());
	llvm::BasicBlock * entryBB = llvm::BasicBlock::Create(module->getContext(), "entry", function);

	id2block_type id2block;

	for (neko::Function::const_iterator it = neko_function.begin();
		 it != neko_function.end();
		 ++it)
		{
			std::stringstream bb_name;
			bb_name << it->first;
			id2block.insert(std::make_pair(it->first, llvm::BasicBlock::Create(module->getContext(), bb_name.str(), function)));
		}

	Stack stack(entryBB);

	for (neko::Function::const_iterator it = neko_function.begin();
		 it != neko_function.end();
		 ++it)
		{
			//remember stack length
			makeBasicBlock(it->second, id2block.find(it->first)->second, id2block, module, stack);
			//check stack length
		}
}

void makeBasicBlock(neko::BasicBlock const & bb, llvm::BasicBlock * bb, id2block_type const & id2block, llvm::Module * module, Stack & stack) {
	llvm::IRBuilder<> builder(bb);
}
