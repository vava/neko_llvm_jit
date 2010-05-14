#include "neko_module.h"

#include "llvm/Module.h"

llvm::Module * makeLLVMModule(neko::Module const & module);
// llvm::FunctionType * makeFunctionType(neko::Function const & f);
// llvm::Function * makeFunction(neko::Function const & f, llvm::Module * module);
// llvm::BasicBlock * makeBasicBlock(neko::BasicBlock const & bb, llvm::Function * f, Stack & stack);

