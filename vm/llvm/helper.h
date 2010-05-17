
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

	llvm::Value * getArrayIndex(Builder & builder, llvm::Value * array, llvm::Value * idx, std::string const & array_name = "array") const {
		llvm::Value * indexies[2] = {idx, int_0()};
		return builder.CreateGEP(array, indexies, indexies + 2, array_name + "[idx]");
	}

	//return 1 if value represents neko int and 0 otherwise
	llvm::Value * is_int(Builder & builder, llvm::Value * v) const {
		return builder.CreateAnd(v, int_1());
	}

	llvm::IntegerType const * int_t() const {
		return llvm::Type::getInt32Ty(ctx);
	}

	llvm::Type const * void_t() const {
		return llvm::Type::getVoidTy(ctx);
	}

	template<typename T>
	llvm::Type const * convert() const {
		return Convert<T>::from(*this);
	}

private:
	llvm::LLVMContext & ctx;

	template<typename T>
	struct Convert {
		static llvm::Type const * from(Helper const & h);
	};
};

template<>
struct Helper::Convert<int> {
	static llvm::Type const * from(Helper const & h) {
		return h.int_t();
	}
};

template<>
struct Helper::Convert<void> {
	static llvm::Type const * from(Helper const & h) {
		return h.void_t();
	}
};

template<typename T>
struct Helper::Convert<T *> {
	static llvm::Type const * from(Helper const & h) {
		return h.convert<T>()->getPointerTo();
	}
};

