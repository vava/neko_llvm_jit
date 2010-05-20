#include "common.h"

int_val p_add(void * vm, int_val a, int_val b);
int_val p_sub(int_val a, int_val b);
int_val p_mult(int_val a, int_val b);
int_val p_div(int_val a, int_val b);
int_val p_call(void * vm, int_val f, int_val n, ...);
