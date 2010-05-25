#include "primitives.h"

#include <stdarg.h>
#include <math.h>
#include <stdio.h>

extern "C" {
	#include "neko.h"
	#include "neko_vm.h"
	#include "neko_mod.h"
	#include "vm.h"

	value neko_append_int( neko_vm *vm, value str, int x, bool way );
	value neko_append_strings( value s1, value s2 );
}

extern field id_add, id_radd, id_sub, id_rsub, id_mult, id_rmult, id_div, id_rdiv, id_mod, id_rmod;

typedef int_val (*c_prim0)();
typedef int_val (*c_prim1)(int_val);
typedef int_val (*c_prim2)(int_val,int_val);
typedef int_val (*c_prim3)(int_val,int_val,int_val);
typedef int_val (*c_prim4)(int_val,int_val,int_val,int_val);
typedef int_val (*c_prim5)(int_val,int_val,int_val,int_val,int_val);
typedef int_val (*c_primN)(value*,int);
typedef int_val (*jit_prim)( neko_vm *, void *, value , neko_module *m );

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

int_val p_call(neko_vm * vm_, int_val f, int_val n, ...) {
	vfunction* func = (vfunction*)f;
	neko_vm * vm = (neko_vm *) vm_;

	if( f & 1 ) {
		val_throw(alloc_string("Invalid call"));
	} else if( val_tag(f) == VAL_FUNCTION && n == func->nargs ) {
		neko_module * m = (neko_module*)func->module;
		int_val* pc = (int_val*)func->addr;

		value env_backup = vm->env;
		value vthis_backup = vm->vthis;

		vm->env = func->env;
		int_val result = (int_val)neko_interp(vm, m, f, pc);

		vm->env = env_backup;
		vm->vthis = vthis_backup;

		return result;
	} else if( val_tag(f) == VAL_PRIMITIVE ) {
		if( n == func->nargs ) {
			va_list argp;
			va_start(argp, n);

			int_val result = 0;
			switch( n ) {
				case 0:
					result = ((c_prim0)func->addr)();
					break;
				case 1:
					result = ((c_prim1)func->addr)(va_arg(argp, int_val));
					break;
				case 2:
					result = ((c_prim2)func->addr)(va_arg(argp, int_val),va_arg(argp, int_val));
					break;
				case 3:
					result = ((c_prim3)func->addr)(va_arg(argp, int_val),va_arg(argp, int_val), va_arg(argp, int_val));
					break;
				case 4:
					result = ((c_prim4)func->addr)(va_arg(argp, int_val),va_arg(argp, int_val), va_arg(argp, int_val),va_arg(argp, int_val));
					break;
				case 5:
					result = ((c_prim5)func->addr)(va_arg(argp, int_val),va_arg(argp, int_val), va_arg(argp, int_val),va_arg(argp, int_val),va_arg(argp, int_val));
					break;
				}
			va_end(argp);
			return result;
		} else if( func->nargs == VAR_ARGS ) {
			va_list argp;
			va_start(argp, n);

			int_val args[CALL_MAX_ARGS];
			for (int i = 0; i < n; i++) {
				args[i] = va_arg(argp, int_val);
			}
			va_end(argp);

			value env_backup = vm->env;
			value vthis_backup = vm->vthis;

			int_val result = ((c_primN)func->addr)((value*)(void*)args, n);

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

int_val p_debug_print(int_val v) {
	printf("%d", v);
	return v;
}
