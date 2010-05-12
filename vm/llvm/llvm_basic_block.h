namespace neko {
	namespace llvm {
		class BasicBlock {
		public:
			BasicBlock(neko::BasicBlock const & bb);
		private:
			llvm::BasicBlock * bb;
		};
	}
}
