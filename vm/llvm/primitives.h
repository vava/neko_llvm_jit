#include "common.h"

int_val p_add(neko_vm * vm, int_val a, int_val b);
int_val p_sub(int_val a, int_val b);
int_val p_mult(int_val a, int_val b);
int_val p_div(int_val a, int_val b);
int_val p_mod(int_val a, int_val b);
int_val p_call(neko_vm * vm, int_val f, int_val n, ...);
int_val p_debug_print(int_val v);
int_val p_get_arr_index(int_val acc, int_val index);
int_val p_set_arr_index(int_val arr, int_val index, int_val new_value);
int_val p_hash(int_val acc);
int_val p_acc_field(neko_vm * vm, int_val obj, int_val idx);
void p_set_field(int_val obj, int_val idx, int_val new_value);

