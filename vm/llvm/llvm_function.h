namespace neko {
	namespace llvm {
		class Function {
		public:
			Function(neko::Function const & function);

			llvm::Function const * getFunction() const;
		private:
			llvm::Function * F;
		};
	}
}
