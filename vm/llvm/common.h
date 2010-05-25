#pragma once

#include <stdint.h>

#define p_val_compare val_compare
#define p_val_throw val_throw
#define p_alloc_string alloc_string

typedef struct _neko_module neko_module;
typedef uintptr_t ptr_val;
typedef intptr_t int_val;
typedef struct _value *value;
typedef struct _neko_vm neko_vm;
