namespace neko {
	namespace llvm {
		class FunctionDeclaration {
		public:
			FunctionDeclaration(neko::Function const & function);

			llvm::Function const * getFunctionDeclaration() const;
		private:
			llvm::Function const * F;
		};
	}
}
