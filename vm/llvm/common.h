#pragma once

#include <stdint.h>

#define p_val_compare val_compare
#define p_val_throw val_throw
#define p_val_print val_print
#define p_alloc_string alloc_string
#define p_alloc_array alloc_array
#define p_alloc_object alloc_object

typedef struct _neko_module neko_module;
typedef uintptr_t ptr_val;
typedef intptr_t int_val;
typedef struct _value *value;
typedef struct _neko_vm neko_vm;
