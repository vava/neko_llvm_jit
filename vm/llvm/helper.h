
class Helper {
public:
	Helper(llvm::LLVMContext & ctx_): ctx(ctx_) {}

	llvm::ConstantInt * int_0() const {
		return int_n(0);
	}

	llvm::ConstantInt * int_1() const {
		return int_n(1);
	}

	llvm::ConstantInt * int_n(int n) const {
		return llvm::ConstantInt::get(int_t(), n);
	}

	llvm::IntegerType const * int_t() const {
		return llvm::Type::getInt32Ty(ctx);
	}

	llvm::Value * getArrayIndex(Builder & builder, llvm::Value * array, llvm::Value * idx, std::string const & array_name = "array") const {
		llvm::Value * indexies[2] = {idx, int_0()};
		return builder.CreateGEP(array, indexies, indexies + 2, array_name + "[idx]");
	}

	//return 1 if value represents neko int and 0 otherwise
	llvm::Value * is_int(Builder & builder, llvm::Value * v) const {
		return builder.CreateAnd(v, int_1());
	}
private:
	llvm::LLVMContext & ctx;
};
