#include "primitives.h"

#include <stdarg.h>
#include <math.h>
#include <stdio.h>

#include "repeat.h"

extern "C" {
	#include "neko.h"
	#include "neko_vm.h"
	#include "neko_mod.h"
	#include "vm.h"
	#include "objtable.h"

	extern value NEKO_TYPEOF[];

	value neko_append_int( neko_vm *vm, value str, int x, bool way );
	value neko_append_strings( value s1, value s2 );
	value neko_alloc_module_function( void *m, int_val pos, int nargs );

	int_val llvm_call(neko_vm * vm, void * f, value * args, int nargs);
}

extern field id_add, id_radd, id_sub, id_rsub, id_mult, id_rmult, id_div, id_rdiv, id_mod, id_rmod;
extern field id_get, id_set;

typedef int_val (*c_prim0)();
typedef int_val (*c_prim1)(int_val);
typedef int_val (*c_prim2)(int_val,int_val);
typedef int_val (*c_prim3)(int_val,int_val,int_val);
typedef int_val (*c_prim4)(int_val,int_val,int_val,int_val);
typedef int_val (*c_prim5)(int_val,int_val,int_val,int_val,int_val);
typedef int_val (*c_primN)(value*,int);
typedef int_val (*jit_prim)( neko_vm *, void *, value , neko_module *m );

typedef int_val (*c_llvmjit0)(neko_vm *);
#define LLVM_JIT_TYPEDEF(n) typedef int_val (*c_llvmjit##n)(neko_vm *, REPEAT_##n(int_val));
REPEAT_MACRO_1_TO_30(LLVM_JIT_TYPEDEF)
#undef LLVM_JIT_TYPEDEF

extern char *jit_boot_seq;

static int_val jit_run( neko_vm *vm, vfunction *acc ) {
	neko_module *m = (neko_module*)acc->module;
	return ((jit_prim)jit_boot_seq)(vm,acc->addr,(value)acc,m);
}

int_val p_add(neko_vm * vm_, int_val a, int_val b) {
	neko_vm * vm = (neko_vm *) vm_;

	if( (b & 1) && (a & 1) ) {
		return (int_val)alloc_int(val_int(a) + val_int(b));
	} else if( b & 1 ) {
		if( val_tag(a) == VAL_FLOAT ) {
			return (int_val)alloc_float(val_float(a) + val_int(b));
		} else if( (val_tag(a)&7) == VAL_STRING  ) {
			return (int_val)neko_append_int(vm,(value)a,val_int(b),true);
		} else if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_add);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("+"));
		}
	} else if( a & 1 ) {
		if( val_tag(b) == VAL_FLOAT )
			return (int_val)alloc_float(val_int(a) + val_float(b));
		else if( (val_tag(b)&7) == VAL_STRING )
			return (int_val)neko_append_int(vm,(value)b,val_int(a),false);
		else if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_radd);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("+"));
		}
	} else if( val_tag(b) == VAL_FLOAT && val_tag(a) == VAL_FLOAT )
		return (int_val)alloc_float(val_float(a) + val_float(b));
	else if( (val_tag(b)&7) == VAL_STRING && (val_tag(a)&7) == VAL_STRING )
		return (int_val)neko_append_strings((value)a,(value)b);
	else {
		if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_add);
			if( _f != val_null ) {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_radd);
			if( _f != val_null ) {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		if( (val_tag(b)&7) == VAL_STRING || (val_tag(a)&7) == VAL_STRING ) {
			buffer b = alloc_buffer(NULL);
			val_buffer(b,(value)a);
			val_buffer(b,(value)b);
			return (int_val)buffer_to_string(b);
		} else {
			val_throw(alloc_string("+"));
		}
	}
	return 0;
}

int_val p_sub(int_val a, int_val b) {
	if( (b & 1) && (a & 1) ) {
		return (int_val)alloc_int(val_int(a) - val_int(b));
	} else if( b & 1 ) {
		if( val_tag(a) == VAL_FLOAT ) {
			return (int_val)alloc_float(val_float(a) - val_int(b));
		} else if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_sub);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("-"));
		}
	} else if( a & 1 ) {
		if( val_tag(b) == VAL_FLOAT ) {
			return (int_val)alloc_float(val_int(a) - val_float(b));
		} else if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rsub);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("-"));
		}
	} else if( val_tag(b) == VAL_FLOAT && val_tag(a) == VAL_FLOAT ) {
		return (int_val)alloc_float(val_float(a) - val_float(b));
	} else {
		if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_sub);
			if( _f != val_null ) {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rsub);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		val_throw(alloc_string("-"));
	}
	return 0;
}

int_val p_mult(int_val a, int_val b) {
	if( (b & 1) && (a & 1) ) {
		return (int_val)alloc_int(val_int(a) * val_int(b));
	} else if( b & 1 ) {
		if( val_tag(a) == VAL_FLOAT ) {
			return (int_val)alloc_float(val_float(a) * val_int(b));
		} else if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_mult);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("*"));
		}
	} else if( a & 1 ) {
		if( val_tag(b) == VAL_FLOAT ) {
			return (int_val)alloc_float(val_int(a) * val_float(b));
		} else if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rmult);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("*"));
		}
	} else if( val_tag(b) == VAL_FLOAT && val_tag(a) == VAL_FLOAT ) {
		return (int_val)alloc_float(val_float(a) * val_float(b));
	} else {
		if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_mult);
			if( _f != val_null ) {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rmult);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		val_throw(alloc_string("*"));
	}
	return 0;
}

int_val p_div(int_val a, int_val b) {
	if( val_is_number(b) && val_is_number(a) ) {
		return (int_val)alloc_float( ((tfloat)val_number(a)) / val_number(b) );
	} else {
		if( val_is_object(a) ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_div);
			if( _f != val_null ) {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		if( val_is_object(b) ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rdiv);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		val_throw(alloc_string("/"));
	}
	return 0;
}

int_val p_mod(int_val a, int_val b) {
	if( b == 1 && val_is_int(a) ) {
		val_throw(alloc_string("%"));
	} else if( (b & 1) && (a & 1) ) {
		return (int_val)alloc_int(val_int(a) % val_int(b));
	} else if( b & 1 ) {
		if( val_tag(a) == VAL_FLOAT ) {
			return (int_val)alloc_float(fmod(val_float(a), val_int(b)));
		} else if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_mod);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("%"));
		}
	} else if( a & 1 ) {
		if( val_tag(b) == VAL_FLOAT ) {
			return (int_val)alloc_float(fmod(val_int(a), val_float(b)));
		} else if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rmod);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		} else {
			val_throw(alloc_string("%"));
		}
	} else if( val_tag(b) == VAL_FLOAT && val_tag(a) == VAL_FLOAT ) {
		return (int_val)alloc_float(fmod(val_float(a), val_float(b)));
	} else {
		if( val_tag(a) == VAL_OBJECT ) {
			value _o = (value)a;
			value _arg = (value)b;
			value _f = val_field(_o,id_mod);
			if( _f != val_null ) {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		if( val_tag(b) == VAL_OBJECT ) {
			value _o = (value)b;
			value _arg = (value)a;
			value _f = val_field(_o,id_rmod);
			if( _f == val_null ) {
				val_throw(alloc_string("Unsupported operation"));
			} else {
				return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
			}
		}

		val_throw(alloc_string("%"));
	}
	return 0;
}

namespace {
int_val v_call(neko_vm * vm, value this_arg, int_val f, int_val n, va_list argp) {
	vfunction* func = (vfunction*)f;

	if( f & 1 ) {
		val_throw(alloc_string("Invalid call"));
	} else if( val_tag(f) == VAL_FUNCTION && n == func->nargs ) {
		neko_module * m = (neko_module*)func->module;
		int_val* pc = (int_val*)func->addr;

		value env_backup = vm->env;
		value vthis_backup = vm->vthis;
		vm->vthis = this_arg;
		vm->env = func->env;

		int_val result = (int_val)neko_interp(vm, m, f, pc);

		vm->env = env_backup;
		vm->vthis = vthis_backup;

		return result;
	} else if( val_tag(f) == VAL_PRIMITIVE) {
		if( n == func->nargs ) {
			value env_backup = vm->env;
			value vthis_backup = vm->vthis;
			vm->vthis = this_arg;
			vm->env = func->env;

			int_val result = 0;
			switch( n ) {
				case 0:
					result = ((c_prim0)func->addr)();
					break;
				case 1:
					result = ((c_prim1)func->addr)(va_arg(argp, int_val));
					break;
				case 2:
					{
						int_val args[] = {va_arg(argp, int_val), va_arg(argp, int_val)};
						result = ((c_prim2)func->addr)(args[0], args[1]);
					}
					break;
				case 3:
					{
						int_val args[] = {va_arg(argp, int_val), va_arg(argp, int_val), va_arg(argp, int_val)};
						result = ((c_prim3)func->addr)(args[0], args[1], args[2]);
					}
					break;
				case 4:
					{
						int_val args[] = {va_arg(argp, int_val), va_arg(argp, int_val), va_arg(argp, int_val), va_arg(argp, int_val)};
						result = ((c_prim4)func->addr)(args[0], args[1], args[2], args[3]);
					}
					break;
				case 5:
					{
						int_val args[] = {va_arg(argp, int_val), va_arg(argp, int_val), va_arg(argp, int_val), va_arg(argp, int_val), va_arg(argp, int_val)};
						result = ((c_prim5)func->addr)(args[0], args[1], args[2], args[3], args[4]);
					}
					break;
				}

			vm->env = env_backup;
			vm->vthis = vthis_backup;

			return result;
		} else if( func->nargs == VAR_ARGS ) {
			int_val args[CALL_MAX_ARGS];
			for (int i = 0; i < n; i++) {
				args[i] = va_arg(argp, int_val);
			}

			value env_backup = vm->env;
			value vthis_backup = vm->vthis;
			vm->vthis = this_arg;
			vm->env = func->env;

			int_val result = ((c_primN)func->addr)((value*)(void*)args, n);

			vm->env = env_backup;
			vm->vthis = vthis_backup;

			return result;
		} else {
			val_throw(alloc_string("Invalid call"));
		}
	} else if( val_tag(f) == VAL_LLVMJITFUN) {
			if( n == func->nargs ) {
			value env_backup = vm->env;
			value vthis_backup = vm->vthis;
			vm->vthis = this_arg;
			vm->env = func->env;

			int_val result = 0;
			switch( n ) {
				case 0:
					{
						result = llvm_call(vm, (void *)func->addr, 0, 0);
					}
					break;
				#define M(x) args[x - 1]
				#define CASE(x) case x: { \
									int_val args[] = {REPEAT_##x(va_arg(argp, int_val))}; \
									result = llvm_call(vm, (void *)func->addr, (value *)args, n); }	\
									break;
				REPEAT_MACRO_1_TO_30(CASE)
				#undef CASE
				#undef M
			}

			vm->env = env_backup;
			vm->vthis = vthis_backup;

			return result;
		} else {
			val_throw(alloc_string("Invalid call"));
		}
	} else if( val_tag(f) == VAL_JITFUN ) {
		if( n == func->nargs ) {
			value env_backup = vm->env;
			value vthis_backup = vm->vthis;
			vm->vthis = this_arg;
			vm->env = func->env;

			int_val result = jit_run(vm,func);

			vm->env = env_backup;
			vm->vthis = vthis_backup;

			return result;
		} else {
			val_throw(alloc_string("Invalid call"));
		}
	} else {
		val_throw(alloc_string("Invalid call"));
	}

	return 0;
}
}

int_val p_call(neko_vm * vm, value this_arg, int_val f, int_val n, ...) {
	va_list argp;
	va_start(argp, n);
	int_val result = v_call(vm, this_arg, f, n, argp);
	va_end(argp);

	return result;
}

int_val p_debug_print(int_val v) {
	printf("%d", v);
	return v;
}

int_val p_get_arr_index(int_val acc, int_val index) {
	if( val_is_int(index) && val_is_array(acc) ) {
		index = val_int(index);
		if( index < 0 || index >= (int_val)val_array_size(acc) ) {
			return (int_val)val_null;
		} else {
			return (int_val)val_array_ptr(acc)[index];
		}
	} else if( val_is_object(acc) ) {
		value _o = (value)acc;
		value _arg = (value)alloc_int(index);
		value _f = val_field(_o,id_get);

		if( _f == val_null ) {
			val_throw(alloc_string("Unsupported operation"));
		} else {
			return (int_val)val_callEx(_o,_f,&_arg,1,NULL);
		}
	} else {
		val_throw(alloc_string("Invalid array access"));
	}
	return 0;
}

int_val p_set_arr_index(int_val arr, int_val index, int_val new_value) {
	if( val_is_array(arr) && val_is_int(index) ) {
		int k = val_int(index);
		if( k >= 0 && k < (int)val_array_size(arr) ) {
			val_array_ptr(arr)[k] = (value)new_value;
		}

		return new_value;
	} else if( val_is_object(arr) ) {
		value args[] = { (value)index, (value)new_value };
		value f = val_field((value)arr,id_set);
		if( f == val_null ) {
			val_throw(alloc_string("Unsupported operation"));
		} else {
			val_callEx((value)arr,f,args,2,NULL);
			return (int_val)args[1];
		}
	} else {
		val_throw(alloc_string("Invalid array access"));
	}

	return 0;
}

int_val p_hash(int_val acc) {
	if( val_is_string(acc) ) {
		return (int_val)alloc_int( val_id(val_string(acc)) );
	} else {
		val_throw(alloc_string("$hash"));
	}

	return 0;
}

int_val p_acc_field(neko_vm * vm, int_val obj, int_val idx) {
	if( val_is_object(obj) ) {
		value *f;
		value old = (value)obj, tacc = (value)obj;
		do {
			f = otable_find(&((vobject*)obj)->table,(field)idx);
			if( f )
				break;
			obj = (int_val)((vobject*)tacc)->proto;
			tacc = (value)obj;
		} while( obj );
		if( f ) {
			return (int_val)*f;
		} else if( vm->resolver ) {
			return (int_val)val_call2(vm->resolver,old,alloc_int(idx));
		} else {
			return (int_val)val_null;
		}
	} else {
		value v = val_field_name((field)idx);
		buffer b;
		if ( val_is_null(v) ) {
			val_throw(alloc_string("Invalid field access"));
		}
		b = alloc_buffer("Invalid field access : ");
		val_buffer(b,v);
		//PushInfos();
		val_throw(buffer_to_string(b));
	}

	return 0;
}

void p_set_field(int_val obj, int_val idx, int_val new_value) {
	if( val_is_object(obj) ) {
		otable_replace(&((vobject*)obj)->table,(field)idx,(value)new_value);
	} else {
		value v = val_field_name((field)idx);
		buffer b;
		if ( val_is_null(v) ) {
			val_throw(alloc_string("Invalid field access"));
		}
		b = alloc_buffer("Invalid field access : ");
		val_buffer(b,v);
		//PushInfos();
		val_throw(buffer_to_string(b));
	}
}

int_val p_make_env(int_val acc, value arr) {
	if( val_is_int(acc) || !val_is_function(acc) ) {
		val_throw(alloc_string("Invalid environment"));
	}
	vfunction * f = (vfunction*)acc;
	vfunction * f_copy = (vfunction *) neko_alloc_module_function(f->module, (int_val)f->addr, f->nargs);
	f_copy->t = f->t;
	f_copy->env = arr;

	return (int_val)f_copy;
}

int_val p_acc_env(neko_vm * vm, int_val idx) {
	if( idx >= (int_val)val_array_size(vm->env) ) {
		val_throw(alloc_string("Reading Outside Env"));
	}
	return (int_val)val_array_ptr(vm->env)[idx];
}

void p_set_env(neko_vm * vm, int_val idx, int_val acc) {
	if( idx >= (int_val)val_array_size(vm->env) ) {
		val_throw(alloc_string("Writing Outside Env"));
	}
	val_array_ptr(vm->env)[idx] = (value)acc;
}

int_val p_apply(neko_vm * vm, value this_arg, int_val f, int_val n, ...) {
	if( !val_is_function(f) ) {
		val_throw(alloc_string("$apply"));
	} else {
		int fargs = val_fun_nargs(f);
		if( fargs == n || fargs == VAR_ARGS ) {
			va_list argp;
			va_start(argp, n);

			int_val result = v_call(vm, this_arg, f, n, argp);

			va_end(argp);

			return result;
		} else if( n > fargs ) {
			val_throw(alloc_string("$apply"));
		} else {
			value env = alloc_array(fargs + 1);

			int i = 0;
			val_array_ptr(env)[i++] = (value)f;

			va_list argp;
			va_start(argp, n);
			for (; i <= n; i++) {
				val_array_ptr(env)[i] = (value)va_arg(argp, int_val);
			}
			va_end(argp);

			for (; i <= fargs; i++) {
				val_array_ptr(env)[i] = val_null;
			}

			return (int_val)neko_alloc_apply((int)(fargs - n),env);
		}
	}
	return 0;
}

int_val p_type_of(int_val acc) {
	return (int_val)(val_is_int(acc) ? alloc_int(1) : NEKO_TYPEOF[val_tag(acc)&7]);
}

extern "C" {
int_val llvm_call(neko_vm * vm, void * f, value * args, int nargs) {
	switch( nargs ) {
		case 0:
			{
				return ((c_llvmjit0)f)(vm);
			}
			break;
		#define M(x) (int_val)args[x - 1]
		#define CASE(x) case x: { \
			return ((c_llvmjit##x)f)(vm, REPEAT_LIST_MACRO_##x(M)); } \
			break;

		REPEAT_MACRO_1_TO_30(CASE)

		#undef CASE
		#undef M
	}

	return 0;
}
}
