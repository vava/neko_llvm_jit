#pragma once

#include "llvm/Constants.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"

#include "common.h"

extern "C" {
#include "../neko.h"
}

class Helper {
public:
	Helper(llvm::LLVMContext & ctx_): ctx(ctx_) {}

	llvm::ConstantInt * int_0() const {
		return int_n(0);
	}

	llvm::ConstantInt * int_1() const {
		return int_n(1);
	}

	llvm::ConstantInt * int_n(int_val n) const {
		return llvm::ConstantInt::get(int_t(), n);
	}

	// llvm::Value * getArrayIndex(Builder & builder, llvm::Value * array, llvm::Value * idx, std::string const & array_name = "array") const {
	// 	llvm::Value * indexies[2] = {idx, int_0()};
	// 	return builder.CreateGEP(array, indexies, indexies + 2, array_name + "[idx]");
	// }

	//return 1 if value represents neko int and 0 otherwise
	// llvm::Value * is_int(Builder & builder, llvm::Value * v) const {
	// 	return builder.CreateAnd(v, int_1());
	// }

	llvm::IntegerType const * int_t() const {
		return templ_int_t<int_val>();
	}

	template<typename T>
	llvm::IntegerType const * templ_int_t() const {
		switch(sizeof(T)) {
			case 1: return llvm::Type::getInt8Ty(ctx);
			case 2: return llvm::Type::getInt16Ty(ctx);
			case 4: return llvm::Type::getInt32Ty(ctx);
			default: return llvm::Type::getInt64Ty(ctx);
		}
	}

	llvm::Type const * void_t() const {
		return llvm::Type::getVoidTy(ctx);
	}

	template<typename T>
	llvm::Constant * constant(T t) const {
		return llvm::ConstantInt::get(convert<T>(), t);
	}

	template<typename T>
	llvm::Constant * constant_0() const {
		return llvm::ConstantInt::get(convert<T>(), 0);
	}

	template<typename T>
	llvm::Constant * constant_1() const {
		return llvm::ConstantInt::get(convert<T>(), 1);
	}

	template<typename T>
	llvm::Constant * constant(T * t) const {
		return llvm::ConstantExpr::getIntToPtr(int_n((int_val)t), convert<T *>());
	}

	template<typename T>
	llvm::Type const * convert() const {
		return Convert<T>::from(*this);
	}

	template<typename T>
	llvm::Type const * convert_array(int_val size) const {
		return llvm::ArrayType::get(convert<T>(), size);
	}

private:
	llvm::LLVMContext & ctx;

	template<typename T>
	struct Convert {
		static llvm::Type const * from(Helper const & h);
	};

	template<typename T>
	friend class Convert;
};

template<>
struct Helper::Convert<void> {
	static llvm::Type const * from(Helper const & h) {
		return h.void_t();
	}
};

#if INTPTR_MAX != INT32_MAX
template<>
struct Helper::Convert<int> {
	static llvm::Type const * from(Helper const & h) {
		return h.templ_int_t<int>();
	}
};
#endif

template<>
struct Helper::Convert<val_type> {
	static llvm::Type const * from(Helper const & h) {
		return h.templ_int_t<val_type>();
	}
};

template<>
struct Helper::Convert<unsigned int> {
	static llvm::Type const * from(Helper const & h) {
		return h.templ_int_t<int>();
	}
};

template<>
struct Helper::Convert<char> {
	static llvm::Type const * from(Helper const & h) {
		return h.templ_int_t<char>();
	}
};

template<>
struct Helper::Convert<int_val> {
	static llvm::Type const * from(Helper const & h) {
		return h.int_t();
	}
};

template<>
struct Helper::Convert<varray> {
	static llvm::Type const * from(Helper const & h) {
		// typedef struct {
		// 	val_type t;
		// 	value ptr;
		// } varray;

		std::vector<const llvm::Type*> fields;
		fields.push_back(h.convert<val_type>()); //t
		fields.push_back(h.convert<value>()); //ptr

		return llvm::StructType::get(h.ctx, fields);
	}
};

template<>
struct Helper::Convert<neko_vm *> {
	static llvm::Type const * from(Helper const & h) {
		return h.convert<int_val>()->getPointerTo();
	}
};

template<>
struct Helper::Convert<value> {
	static llvm::Type const * from(Helper const & h) {
		return h.convert<int_val>()->getPointerTo();
	}
};

template<typename T>
struct Helper::Convert<const T> {
	static llvm::Type const * from(Helper const & h) {
		return h.convert<T>();
	}
};

template<typename T>
struct Helper::Convert<T *> {
	static llvm::Type const * from(Helper const & h) {
		return h.convert<T>()->getPointerTo();
	}
};
