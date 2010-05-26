#include "common.h"

int_val __attribute__((fastcall)) p_add(neko_vm * vm, int_val a, int_val b);
int_val p_sub(int_val a, int_val b);
int_val p_mult(int_val a, int_val b);
int_val p_div(int_val a, int_val b);
int_val p_mod(int_val a, int_val b);
int_val p_call(neko_vm * vm, int_val f, int_val n, ...);
int_val p_debug_print(int_val v);
