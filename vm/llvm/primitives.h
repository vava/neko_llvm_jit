#include "common.h"

int_val p_add(neko_vm * vm, int_val a, int_val b);
int_val p_sub(int_val a, int_val b);
int_val p_mult(int_val a, int_val b);
int_val p_div(int_val a, int_val b);
int_val p_mod(int_val a, int_val b);
int_val p_call(neko_vm * vm, neko_module * m, int_val pc, value this_arg, int_val f, int_val n, ...);
int_val p_get_arr_index(int_val acc, int_val index);
int_val p_set_arr_index(int_val arr, int_val index, int_val new_value);
int_val p_hash(int_val acc);
int_val p_acc_field(neko_vm * vm, neko_module * m, int_val pc, int_val obj, int_val idx);
void p_set_field(neko_vm * vm, neko_module * m, int_val pc, int_val obj, int_val idx, int_val new_value);
int_val p_make_env(int_val acc, value arr);
int_val p_acc_env(neko_vm * vm, neko_module * m, int_val pc, int_val idx);
void p_set_env(neko_vm * vm, int_val idx, int_val acc);
int_val p_apply(neko_vm * vm, neko_module * m, int_val pc, value this_arg, int_val f, int_val n, ...);
int_val p_type_of(int_val acc);
void p_setup_trap(neko_vm * vm, neko_module * m, int_val trap_addr);
void p_end_trap(neko_vm * vm);

