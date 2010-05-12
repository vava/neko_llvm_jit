namespace neko {
	namespace llvm {
		class Module {
		public:
			Module(neko::Module const & neko_module);

			llvm::Module * getModule() const;
		private:
			llvm::Module * module;
		}
	}
}
